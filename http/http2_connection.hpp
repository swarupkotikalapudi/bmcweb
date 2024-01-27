#pragma once
#include "bmcweb_config.h"

#include "async_resp.hpp"
#include "authentication.hpp"
#include "complete_response_fields.hpp"
#include "http_response.hpp"
#include "http_utility.hpp"
#include "logging.hpp"
#include "mutual_tls.hpp"
#include "nghttp2_adapters.hpp"
#include "ssl_key_handler.hpp"
#include "utility.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/beast/http/error.hpp>
#include <boost/beast/http/parser.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/serializer.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>
#include <boost/beast/websocket.hpp>

#include <atomic>
#include <chrono>
#include <vector>

namespace crow
{

struct Http2StreamData
{
    Request req{};
    Response res{};
    std::optional<bmcweb::FileBody::writer> writer;
};

template <typename Adaptor, typename Handler>
class HTTP2Connection :
    public std::enable_shared_from_this<HTTP2Connection<Adaptor, Handler>>
{
    using self_type = HTTP2Connection<Adaptor, Handler>;

  public:
    HTTP2Connection(Adaptor&& adaptorIn, Handler* handlerIn,
                    std::function<std::string()>& getCachedDateStrF) :
        adaptor(std::move(adaptorIn)),
        ngSession(initializeNghttp2Session()), handler(handlerIn),
        getCachedDateStr(getCachedDateStrF)
    {}

    void start()
    {
        // Create the control stream
        streams[0];

        if (sendServerConnectionHeader() != 0)
        {
            BMCWEB_LOG_ERROR("send_server_connection_header failed");
            return;
        }
        doRead();
    }

    int sendServerConnectionHeader()
    {
        BMCWEB_LOG_DEBUG("send_server_connection_header()");

        uint32_t maxStreams = 4;
        std::array<nghttp2_settings_entry, 2> iv = {
            {{NGHTTP2_SETTINGS_MAX_CONCURRENT_STREAMS, maxStreams},
             {NGHTTP2_SETTINGS_ENABLE_PUSH, 0}}};
        int rv = ngSession.submitSettings(iv);
        if (rv != 0)
        {
            BMCWEB_LOG_ERROR("Fatal error: {}", nghttp2_strerror(rv));
            return -1;
        }
        writeBuffer();
        return 0;
    }

    static ssize_t fileReadCallback(nghttp2_session* /* session */,
                                    int32_t streamId, uint8_t* buf,
                                    size_t length, uint32_t* dataFlags,
                                    nghttp2_data_source* /*source*/,
                                    void* userPtr)
    {
        self_type& self = userPtrToSelf(userPtr);

        auto streamIt = self.streams.find(streamId);
        if (streamIt == self.streams.end())
        {
            return NGHTTP2_ERR_TEMPORAL_CALLBACK_FAILURE;
        }
        Http2StreamData& stream = streamIt->second;
        BMCWEB_LOG_DEBUG("File read callback length: {}", length);

        boost::beast::error_code ec;
        boost::optional<std::pair<boost::asio::const_buffer, bool>> out =
            stream.writer->getWithMaxSize(ec, length);
        if (ec)
        {
            return NGHTTP2_ERR_TEMPORAL_CALLBACK_FAILURE;
        }

        if (!out)
        {
            *dataFlags |= NGHTTP2_DATA_FLAG_EOF;
            return 0;
        }

        BMCWEB_LOG_DEBUG("Send chunk of size: {}", out->first.size());
        if (length < out->first.size())
        {
            // Should never happen because of length limit on get() above
            return NGHTTP2_ERR_TEMPORAL_CALLBACK_FAILURE;
        }

        BMCWEB_LOG_DEBUG("Copying {} bytes to buf", out->first.size());
        memcpy(buf, out->first.data(), out->first.size());

        if (!out->second)
        {
            BMCWEB_LOG_DEBUG("Setting OEF and nocopy flag");
            *dataFlags |= NGHTTP2_DATA_FLAG_EOF;
            //*dataFlags |= NGHTTP2_DATA_FLAG_NO_COPY;
        }
        return static_cast<ssize_t>(out->first.size());
    }

    nghttp2_nv headerFromStringViews(std::string_view name,
                                     std::string_view value, uint8_t flags)
    {
        uint8_t* nameData = std::bit_cast<uint8_t*>(name.data());
        uint8_t* valueData = std::bit_cast<uint8_t*>(value.data());
        return {nameData, valueData, name.size(), value.size(), flags};
    }

    int sendResponse(Response& completedRes, int32_t streamId)
    {
        BMCWEB_LOG_DEBUG("send_response stream_id:{}", streamId);

        auto it = streams.find(streamId);
        if (it == streams.end())
        {
            close();
            return -1;
        }
        Response& thisRes = it->second.res;
        thisRes = std::move(completedRes);
        crow::Request& thisReq = it->second.req;
        std::vector<nghttp2_nv> hdr;

        completeResponseFields(thisReq, thisRes);
        thisRes.addHeader(boost::beast::http::field::date, getCachedDateStr());

        boost::beast::http::fields& fields = thisRes.fields();
        std::string code = std::to_string(thisRes.resultInt());
        hdr.emplace_back(
            headerFromStringViews(":status", code, NGHTTP2_NV_FLAG_NONE));
        for (const boost::beast::http::fields::value_type& header : fields)
        {
            hdr.emplace_back(headerFromStringViews(
                header.name_string(), header.value(),
                NGHTTP2_NV_FLAG_NO_COPY_VALUE | NGHTTP2_NV_FLAG_NO_COPY_NAME));
        }
        Http2StreamData& stream = it->second;
        crow::Response& res = stream.res;
        http::response<bmcweb::FileBody>& fbody = res.response;
        stream.writer.emplace(fbody.base(), fbody.body());

        nghttp2_data_provider dataPrd{
            .source = {.fd = 0},
            .read_callback = fileReadCallback,
        };

        int rv = ngSession.submitResponse(streamId, hdr, &dataPrd);
        if (rv != 0)
        {
            BMCWEB_LOG_ERROR("Fatal error: {}", nghttp2_strerror(rv));
            close();
            return -1;
        }
        writeBuffer();

        return 0;
    }

    nghttp2_session initializeNghttp2Session()
    {
        nghttp2_session_callbacks callbacks;
        callbacks.setOnFrameRecvCallback(onFrameRecvCallbackStatic);
        callbacks.setOnStreamCloseCallback(onStreamCloseCallbackStatic);
        callbacks.setOnHeaderCallback(onHeaderCallbackStatic);
        callbacks.setOnBeginHeadersCallback(onBeginHeadersCallbackStatic);

        nghttp2_session session(callbacks);
        session.setUserData(this);

        return session;
    }

    int onRequestRecv(int32_t streamId)
    {
        BMCWEB_LOG_DEBUG("on_request_recv");

        auto it = streams.find(streamId);
        if (it == streams.end())
        {
            close();
            return -1;
        }

        crow::Request& thisReq = it->second.req;
        BMCWEB_LOG_DEBUG("Handling {} \"{}\"", logPtr(&thisReq),
                         thisReq.url().encoded_path());

        crow::Response& thisRes = it->second.res;

        thisRes.setCompleteRequestHandler(
            [this, streamId](Response& completeRes) {
            BMCWEB_LOG_DEBUG("res.completeRequestHandler called");
            if (sendResponse(completeRes, streamId) != 0)
            {
                close();
                return;
            }
        });
        auto asyncResp =
            std::make_shared<bmcweb::AsyncResp>(std::move(it->second.res));
        handler->handle(thisReq, asyncResp);

        return 0;
    }

    int onFrameRecvCallback(const nghttp2_frame& frame)
    {
        BMCWEB_LOG_DEBUG("frame type {}", static_cast<int>(frame.hd.type));
        switch (frame.hd.type)
        {
            case NGHTTP2_DATA:
            case NGHTTP2_HEADERS:
                // Check that the client request has finished
                if ((frame.hd.flags & NGHTTP2_FLAG_END_STREAM) != 0)
                {
                    return onRequestRecv(frame.hd.stream_id);
                }
                break;
            default:
                break;
        }
        return 0;
    }

    static int onFrameRecvCallbackStatic(nghttp2_session* /* session */,
                                         const nghttp2_frame* frame,
                                         void* userData)
    {
        BMCWEB_LOG_DEBUG("on_frame_recv_callback");
        if (userData == nullptr)
        {
            BMCWEB_LOG_CRITICAL("user data was null?");
            return NGHTTP2_ERR_CALLBACK_FAILURE;
        }
        if (frame == nullptr)
        {
            BMCWEB_LOG_CRITICAL("frame was null?");
            return NGHTTP2_ERR_CALLBACK_FAILURE;
        }
        return userPtrToSelf(userData).onFrameRecvCallback(*frame);
    }

    static self_type& userPtrToSelf(void* userData)
    {
        // This method exists to keep the unsafe reinterpret cast in one
        // place.
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return *reinterpret_cast<self_type*>(userData);
    }

    static int onStreamCloseCallbackStatic(nghttp2_session* /* session */,
                                           int32_t streamId,
                                           uint32_t /*unused*/, void* userData)
    {
        BMCWEB_LOG_DEBUG("on_stream_close_callback stream {}", streamId);
        if (userData == nullptr)
        {
            BMCWEB_LOG_CRITICAL("user data was null?");
            return NGHTTP2_ERR_CALLBACK_FAILURE;
        }
        if (userPtrToSelf(userData).streams.erase(streamId) <= 0)
        {
            return -1;
        }
        return 0;
    }

    int onHeaderCallback(const nghttp2_frame& frame,
                         std::span<const uint8_t> name,
                         std::span<const uint8_t> value)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        std::string_view nameSv(reinterpret_cast<const char*>(name.data()),
                                name.size());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        std::string_view valueSv(reinterpret_cast<const char*>(value.data()),
                                 value.size());

        BMCWEB_LOG_DEBUG("on_header_callback name: {} value {}", nameSv,
                         valueSv);

        switch (frame.hd.type)
        {
            case NGHTTP2_HEADERS:
                if (frame.headers.cat != NGHTTP2_HCAT_REQUEST)
                {
                    break;
                }
                auto thisStream = streams.find(frame.hd.stream_id);
                if (thisStream == streams.end())
                {
                    BMCWEB_LOG_ERROR("Unknown stream{}", frame.hd.stream_id);
                    close();
                    return -1;
                }

                crow::Request& thisReq = thisStream->second.req;

                if (nameSv == ":path")
                {
                    thisReq.target(valueSv);
                }
                else if (nameSv == ":method")
                {
                    boost::beast::http::verb verb =
                        boost::beast::http::string_to_verb(valueSv);
                    if (verb == boost::beast::http::verb::unknown)
                    {
                        BMCWEB_LOG_ERROR("Unknown http verb {}", valueSv);
                        close();
                        return -1;
                    }
                    thisReq.req.method(verb);
                }
                else if (nameSv == ":scheme")
                {
                    // Nothing to check on scheme
                }
                else
                {
                    thisReq.req.set(nameSv, valueSv);
                }
                break;
        }
        return 0;
    }

    static int onHeaderCallbackStatic(nghttp2_session* /* session */,
                                      const nghttp2_frame* frame,
                                      const uint8_t* name, size_t namelen,
                                      const uint8_t* value, size_t vallen,
                                      uint8_t /* flags */, void* userData)
    {
        if (userData == nullptr)
        {
            BMCWEB_LOG_CRITICAL("user data was null?");
            return NGHTTP2_ERR_CALLBACK_FAILURE;
        }
        if (frame == nullptr)
        {
            BMCWEB_LOG_CRITICAL("frame was null?");
            return NGHTTP2_ERR_CALLBACK_FAILURE;
        }
        if (name == nullptr)
        {
            BMCWEB_LOG_CRITICAL("name was null?");
            return NGHTTP2_ERR_CALLBACK_FAILURE;
        }
        if (value == nullptr)
        {
            BMCWEB_LOG_CRITICAL("value was null?");
            return NGHTTP2_ERR_CALLBACK_FAILURE;
        }
        return userPtrToSelf(userData).onHeaderCallback(*frame, {name, namelen},
                                                        {value, vallen});
    }

    int onBeginHeadersCallback(const nghttp2_frame& frame)
    {
        if (frame.hd.type == NGHTTP2_HEADERS &&
            frame.headers.cat == NGHTTP2_HCAT_REQUEST)
        {
            BMCWEB_LOG_DEBUG("create stream for id {}", frame.hd.stream_id);

            Http2StreamData& stream = streams[frame.hd.stream_id];
            // http2 is by definition always tls
            stream.req.isSecure = true;
        }
        return 0;
    }

    static int onBeginHeadersCallbackStatic(nghttp2_session* /* session */,
                                            const nghttp2_frame* frame,
                                            void* userData)
    {
        BMCWEB_LOG_DEBUG("on_begin_headers_callback");
        if (userData == nullptr)
        {
            BMCWEB_LOG_CRITICAL("user data was null?");
            return NGHTTP2_ERR_CALLBACK_FAILURE;
        }
        if (frame == nullptr)
        {
            BMCWEB_LOG_CRITICAL("frame was null?");
            return NGHTTP2_ERR_CALLBACK_FAILURE;
        }
        return userPtrToSelf(userData).onBeginHeadersCallback(*frame);
    }

    static void afterWriteBuffer(const std::shared_ptr<self_type>& self,
                                 const boost::system::error_code& ec,
                                 size_t sendLength)
    {
        self->isWriting = false;
        BMCWEB_LOG_DEBUG("Sent {}", sendLength);
        if (ec)
        {
            self->close();
            return;
        }
        self->writeBuffer();
    }

    void writeBuffer()
    {
        if (isWriting)
        {
            return;
        }
        std::span<const uint8_t> data = ngSession.memSend();
        if (data.empty())
        {
            return;
        }
        isWriting = true;
        adaptor.async_write_some(
            boost::asio::buffer(data.data(), data.size()),
            std::bind_front(afterWriteBuffer, shared_from_this()));
    }

    void close()
    {
        if constexpr (std::is_same_v<Adaptor,
                                     boost::beast::ssl_stream<
                                         boost::asio::ip::tcp::socket>>)
        {
            adaptor.next_layer().close();
        }
        else
        {
            adaptor.close();
        }
    }

    void afterDoRead(const std::shared_ptr<self_type>& /*self*/,
                     const boost::system::error_code& ec,
                     size_t bytesTransferred)
    {
        BMCWEB_LOG_DEBUG("{} async_read_some {} Bytes", logPtr(this),
                         bytesTransferred);

        if (ec)
        {
            BMCWEB_LOG_ERROR("{} Error while reading: {}", logPtr(this),
                             ec.message());
            close();
            BMCWEB_LOG_DEBUG("{} from read(1)", logPtr(this));
            return;
        }
        std::span<uint8_t> bufferSpan{inBuffer.data(), bytesTransferred};
        ssize_t readLen = ngSession.memRecv(bufferSpan);
        if (readLen < 0 || static_cast<size_t>(readLen) != bufferSpan.size())
        {
            BMCWEB_LOG_ERROR("nghttp2_session_mem_recv returned {}", readLen);
            close();
            return;
        }
        writeBuffer();

        doRead();
    }

    void doRead()
    {
        BMCWEB_LOG_DEBUG("{} doRead", logPtr(this));
        adaptor.async_read_some(
            boost::asio::buffer(inBuffer),
            std::bind_front(&self_type::afterDoRead, this, shared_from_this()));
    }

    // A mapping from http2 stream ID to Stream Data
    std::unordered_map<int32_t, Http2StreamData> streams;

    std::array<uint8_t, 8192> inBuffer{};

    Adaptor adaptor;
    bool isWriting = false;

    nghttp2_session ngSession;

    Handler* handler;
    std::function<std::string()>& getCachedDateStr;

    using std::enable_shared_from_this<
        HTTP2Connection<Adaptor, Handler>>::shared_from_this;

    using std::enable_shared_from_this<
        HTTP2Connection<Adaptor, Handler>>::weak_from_this;
};
} // namespace crow
