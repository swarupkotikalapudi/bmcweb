#pragma once

#include "logging.hpp"
#include "utility.hpp"

#include <unistd.h>

#include <boost/beast/core/buffers_range.hpp>
#include <boost/beast/core/file_posix.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/system/error_code.hpp>

#include <string_view>

namespace bmcweb
{
struct HttpBody
{
    // Body concept requires specific naming of classes
    // NOLINTBEGIN(readability-identifier-naming)
    class writer;
    class reader;
    class value_type;
    // NOLINTEND(readability-identifier-naming)
};

enum class EncodingType
{
    Raw,
    Base64,
};

class HttpBody::value_type
{
    boost::beast::file_posix fileHandle;
    std::optional<size_t> fileSize;
    std::string strBody;

  public:
    EncodingType encodingType = EncodingType::Raw;

    ~value_type() = default;
    value_type() = default;
    explicit value_type(EncodingType enc) : encodingType(enc) {}
    explicit value_type(std::string_view str) : strBody(str) {}

    value_type(value_type&& other) noexcept :
        fileHandle(std::move(other.fileHandle)), fileSize(other.fileSize),
        strBody(std::move(other.strBody)), encodingType(other.encodingType)
    {}

    value_type& operator=(value_type&& other) noexcept
    {
        fileHandle = std::move(other.fileHandle);
        fileSize = other.fileSize;
        strBody = std::move(other.strBody);
        encodingType = other.encodingType;

        return *this;
    }

    // Overload copy constructor, because posix doesn't have dup(), but linux
    // does
    value_type(const value_type& other) :
        fileSize(other.fileSize), strBody(other.strBody),
        encodingType(other.encodingType)
    {
        fileHandle.native_handle(dup(other.fileHandle.native_handle()));
    }

    value_type& operator=(const value_type& other)
    {
        if (this != &other)
        {
            fileSize = other.fileSize;
            strBody = other.strBody;
            encodingType = other.encodingType;
            fileHandle.native_handle(dup(other.fileHandle.native_handle()));
        }
        return *this;
    }

    const boost::beast::file_posix& file()
    {
        return fileHandle;
    }

    std::string& str()
    {
        return strBody;
    }

    const std::string& str() const
    {
        return strBody;
    }

    std::optional<size_t> payloadSize() const
    {
        if (!fileHandle.is_open())
        {
            return strBody.size();
        }
        if (fileSize)
        {
            if (encodingType == EncodingType::Base64)
            {
                return crow::utility::Base64Encoder::encodedSize(*fileSize);
            }
        }
        return fileSize;
    }

    void clear()
    {
        strBody.clear();
        strBody.shrink_to_fit();
        fileHandle = boost::beast::file_posix();
        fileSize = std::nullopt;
        encodingType = EncodingType::Raw;
    }

    void open(const char* path, boost::beast::file_mode mode,
              boost::system::error_code& ec)
    {
        fileHandle.open(path, mode, ec);
        if (ec)
        {
            return;
        }
        boost::system::error_code ec2;
        uint64_t size = fileHandle.size(ec2);
        if (!ec2)
        {
            BMCWEB_LOG_INFO("File size was {} bytes", size);
            fileSize = static_cast<size_t>(size);
        }
        else
        {
            BMCWEB_LOG_WARNING("Failed to read file size on {}", path);
        }
        ec = {};
    }

    void setFd(int fd, boost::system::error_code& ec)
    {
        fileHandle.native_handle(fd);

        boost::system::error_code ec2;
        uint64_t size = fileHandle.size(ec2);
        if (!ec2)
        {
            if (size != 0 && size < std::numeric_limits<size_t>::max())
            {
                fileSize = static_cast<size_t>(size);
            }
        }
        ec = {};
    }
};

class HttpBody::writer
{
  public:
    using const_buffers_type = boost::asio::const_buffer;

  private:
    std::string buf;
    crow::utility::Base64Encoder encoder;

    value_type& body;
    size_t sent = 0;
    constexpr static size_t readBufSize = 4096;
    std::array<char, readBufSize> fileReadBuf{};

  public:
    template <bool IsRequest, class Fields>
    writer(boost::beast::http::header<IsRequest, Fields>& /*header*/,
           value_type& bodyIn) : body(bodyIn)
    {}

    static void init(boost::beast::error_code& ec)
    {
        ec = {};
    }

    boost::optional<std::pair<const_buffers_type, bool>>
        get(boost::beast::error_code& ec)
    {
        return getWithMaxSize(ec, std::numeric_limits<size_t>::max());
    }

    boost::optional<std::pair<const_buffers_type, bool>>
        getWithMaxSize(boost::beast::error_code& ec, size_t maxSize)
    {
        std::pair<const_buffers_type, bool> ret;
        if (!body.file().is_open())
        {
            size_t remain = body.str().size() - sent;
            size_t toReturn = std::min(maxSize, remain);
            ret.first = const_buffers_type(&body.str()[sent], toReturn);

            sent += toReturn;
            ret.second = sent < body.str().size();
            BMCWEB_LOG_INFO("Returning {} bytes more={}", ret.first.size(),
                            ret.second);
            return ret;
        }
        size_t readReq = std::min(fileReadBuf.size(), maxSize);
        size_t read = body.file().read(fileReadBuf.data(), readReq, ec);
        if (ec)
        {
            BMCWEB_LOG_CRITICAL("Failed to read from file");
            return boost::none;
        }

        std::string_view chunkView(fileReadBuf.data(), read);
        BMCWEB_LOG_INFO("Read {} bytes from file", read);
        // If the number of bytes read equals the amount requested, we haven't
        // reached EOF yet
        ret.second = read == readReq;
        if (body.encodingType == EncodingType::Base64)
        {
            buf.clear();
            buf.reserve(
                crow::utility::Base64Encoder::encodedSize(chunkView.size()));
            encoder.encode(chunkView, buf);
            if (!ret.second)
            {
                encoder.finalize(buf);
            }
            ret.first = const_buffers_type(buf.data(), buf.size());
        }
        else
        {
            ret.first = const_buffers_type(chunkView.data(), chunkView.size());
        }
        return ret;
    }
};

class HttpBody::reader
{
    value_type& value;

  public:
    template <bool IsRequest, class Fields>
    reader(boost::beast::http::header<IsRequest, Fields>& /*headers*/,
           value_type& body) : value(body)
    {}

    void init(const boost::optional<std::uint64_t>& contentLength,
              boost::beast::error_code& ec)
    {
        if (contentLength)
        {
            if (!value.file().is_open())
            {
                value.str().reserve(static_cast<size_t>(*contentLength));
            }
        }
        ec = {};
    }

    template <class ConstBufferSequence>
    std::size_t put(const ConstBufferSequence& buffers,
                    boost::system::error_code& ec)
    {
        size_t extra = boost::beast::buffer_bytes(buffers);
        for (const auto b : boost::beast::buffers_range_ref(buffers))
        {
            const char* ptr = static_cast<const char*>(b.data());
            value.str() += std::string_view(ptr, b.size());
        }
        ec = {};
        return extra;
    }

    static void finish(boost::system::error_code& ec)
    {
        ec = {};
    }
};

} // namespace bmcweb
