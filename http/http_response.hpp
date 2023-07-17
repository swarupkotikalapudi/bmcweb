#pragma once
#include "logging.hpp"
#include "utils/hex_utils.hpp"

#include <boost/beast/http/basic_dynamic_body.hpp>
#include <boost/beast/http/buffer_body.hpp>
#include <boost/beast/http/file_body.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>
#include <nlohmann/json.hpp>

#include <optional>
#include <string>
#include <string_view>

namespace crow
{

template <typename Adaptor, typename Handler>
class Connection;

template <typename Ret = std::nullptr_t>
inline Ret safeVisit(auto callable, auto& res)
{
    try
    {
        if constexpr (std::is_same_v<Ret, std::nullptr_t>)
        {
            std::visit(std::move(callable), res);
            return std::nullptr_t{};
        }
        else
        {
            return std::visit(std::move(callable), res);
        }
    }
    catch (std::bad_variant_access& /*unused*/)
    {}
    return Ret{};
}

inline void safeMove(auto callable)
{
    try
    {
        callable();
    }
    catch (std::bad_variant_access& /*unused*/)
    {}
}

struct Response
{
    template <typename Adaptor, typename Handler>
    friend class crow::Connection;
    using string_body_response_type =
        boost::beast::http::response<boost::beast::http::string_body>;
    using file_body_response_type =
        boost::beast::http::response<boost::beast::http::file_body>;

    using response_type =
        std::variant<string_body_response_type, file_body_response_type>;

    std::optional<response_type> genericResponse;

    nlohmann::json jsonValue;

    void addHeader(const std::string_view key, const std::string_view value)
    {
        safeVisit([key, value](auto&& res) { res.set(key, value); },
                  genericResponse.value());
    }

    void addHeader(boost::beast::http::field key, std::string_view value)
    {
        safeVisit([key, value](auto&& res) { res.set(key, value); },
                  genericResponse.value());
    }
    void clearHeader(boost::beast::http::field key)
    {
        safeVisit([key](auto&& res) { res.erase(key); },
                  genericResponse.value());
    }

    Response() : genericResponse(string_body_response_type{}) {}

    Response(Response&& res) noexcept :
        genericResponse(std::move(res.genericResponse)),
        jsonValue(std::move(res.jsonValue)), completed(res.completed)
    {
        // See note in operator= move handler for why this is needed.
        if (!res.completed)
        {
            completeRequestHandler = std::move(res.completeRequestHandler);
            res.completeRequestHandler = nullptr;
        }
        isAliveHelper = res.isAliveHelper;
        res.isAliveHelper = nullptr;
    }

    ~Response() = default;

    Response(const Response&) = delete;

    Response& operator=(const Response& r) = delete;

    Response& operator=(Response&& r) noexcept
    {
        BMCWEB_LOG_DEBUG("Moving response containers; this: {}; other: {}",
                         logPtr(this), logPtr(&r));
        if (this == &r)
        {
            return *this;
        }
        safeMove(
            [this, &r]() { genericResponse = std::move(r.genericResponse); });

        r.genericResponse.emplace(response_type{});
        jsonValue = std::move(r.jsonValue);

        // Only need to move completion handler if not already completed
        // Note, there are cases where we might move out of a Response object
        // while in a completion handler for that response object.  This check
        // is intended to prevent destructing the functor we are currently
        // executing from in that case.
        if (!r.completed)
        {
            completeRequestHandler = std::move(r.completeRequestHandler);
            r.completeRequestHandler = nullptr;
        }
        else
        {
            completeRequestHandler = nullptr;
        }
        completed = r.completed;
        isAliveHelper = std::move(r.isAliveHelper);
        r.isAliveHelper = nullptr;
        return *this;
    }

    void result(unsigned v)
    {
        safeVisit([v](auto&& res) { res.result(v); }, genericResponse.value());
    }

    void result(boost::beast::http::status v)
    {
        safeVisit([v](auto&& res) { res.result(v); }, genericResponse.value());
    }

    boost::beast::http::status result() const
    {
        return safeVisit<boost::beast::http::status>(
            [](auto&& res) { return res.result(); }, genericResponse.value());
    }

    unsigned resultInt() const
    {
        return safeVisit<unsigned>([](auto&& res) { return res.result_int(); },
                                   genericResponse.value());
    }

    std::string_view reason() const
    {
        return safeVisit<std::string_view>(
            [](auto&& res) { return res.reason(); }, genericResponse.value());
    }

    bool isCompleted() const noexcept
    {
        return completed;
    }
    boost::beast::http::fields fields()
    {
        return safeVisit<boost::beast::http::fields>(
            [](auto&& res) { return res.base(); }, genericResponse.value());
    }
    template <typename Alternative>
    void updateAlternative()
    {
        if (!std::holds_alternative<Alternative>(genericResponse.value()))
        {
            Alternative altbody{};
            std::visit(
                [&altbody](auto&& other) { altbody.base() = other.base(); },
                genericResponse.value());

            genericResponse.emplace(std::move(altbody));
        }
    }
    std::string& body()
    {
        updateAlternative<string_body_response_type>();
        return std::get<string_body_response_type>(*genericResponse).body();
    }

    bool openFile(const std::filesystem::path& path)
    {
        boost::beast::http::file_body::value_type file;
        boost::beast::error_code ec;
        file.open(path.c_str(), boost::beast::file_mode::read, ec);
        if (ec)
        {
            return false;
        }
        updateAlternative<file_body_response_type>();
        std::get<file_body_response_type>(*genericResponse).body() =
            std::move(file);
        return true;
    }

    std::string_view getHeaderValue(std::string_view key) const
    {
        return safeVisit<std::string_view>(
            [key](auto&& res) { return res.base()[key]; },
            genericResponse.value());
    }

    void keepAlive(bool k)
    {
        safeVisit([k](auto&& res) { res.keep_alive(k); },
                  genericResponse.value());
    }

    bool keepAlive() const
    {
        return safeVisit<bool>([](auto&& res) { return res.keep_alive(); },
                               genericResponse.value());
    }

    void preparePayload()
    {
        safeVisit([this](auto&& res) { return preparePayload(res); },
                  genericResponse.value());
    }
    void preparePayload(auto& bodyResponse)
    {
        // This code is a throw-free equivalent to
        // beast::http::message::prepare_payload
        boost::optional<uint64_t> pSize = bodyResponse.payload_size();
        using boost::beast::http::status;
        using boost::beast::http::status_class;
        using boost::beast::http::to_status_class;
        if (!pSize)
        {
            pSize = 0;
        }
        else
        {
            bool is1XXReturn = to_status_class(bodyResponse.result()) ==
                               status_class::informational;
            if (*pSize > 0 &&
                (is1XXReturn || bodyResponse.result() == status::no_content ||
                 bodyResponse.result() == status::not_modified))
            {
                BMCWEB_LOG_CRITICAL(
                    "{} Response content provided but code was no-content or not_modified, which aren't allowed to have a body",
                    logPtr(this));
                pSize = 0;
                body().clear();
            }
        }
        bodyResponse.content_length(*pSize);
    }

    void clear()
    {
        BMCWEB_LOG_DEBUG("{} Clearing response containers", logPtr(this));

        genericResponse.emplace(string_body_response_type{});
        jsonValue.clear();
        completed = false;
        expectedHash = std::nullopt;
    }

    void write(std::string_view bodyPart)
    {
        body() += std::string(bodyPart);
    }

    std::string computeEtag() const
    {
        // Only set etag if this request succeeded
        if (result() != boost::beast::http::status::ok)
        {
            return "";
        }
        // and the json response isn't empty
        if (jsonValue.empty())
        {
            return "";
        }
        size_t hashval = std::hash<nlohmann::json>{}(jsonValue);
        return "\"" + intToHexString(hashval, 8) + "\"";
    }

    void end()
    {
        std::string etag = computeEtag();
        if (!etag.empty())
        {
            addHeader(boost::beast::http::field::etag, etag);
        }
        if (completed)
        {
            BMCWEB_LOG_ERROR("{} Response was ended twice", logPtr(this));
            return;
        }
        completed = true;
        BMCWEB_LOG_DEBUG("{} calling completion handler", logPtr(this));
        if (completeRequestHandler)
        {
            BMCWEB_LOG_DEBUG("{} completion handler was valid", logPtr(this));
            completeRequestHandler(*this);
        }
    }

    bool isAlive() const
    {
        return isAliveHelper && isAliveHelper();
    }

    void setCompleteRequestHandler(std::function<void(Response&)>&& handler)
    {
        BMCWEB_LOG_DEBUG("{} setting completion handler", logPtr(this));
        completeRequestHandler = std::move(handler);

        // Now that we have a new completion handler attached, we're no longer
        // complete
        completed = false;
    }

    std::function<void(Response&)> releaseCompleteRequestHandler()
    {
        BMCWEB_LOG_DEBUG("{} releasing completion handler{}", logPtr(this),
                         static_cast<bool>(completeRequestHandler));
        std::function<void(Response&)> ret = completeRequestHandler;
        completeRequestHandler = nullptr;
        completed = true;
        return ret;
    }

    void setIsAliveHelper(std::function<bool()>&& handler)
    {
        isAliveHelper = std::move(handler);
    }

    std::function<bool()> releaseIsAliveHelper()
    {
        std::function<bool()> ret = std::move(isAliveHelper);
        isAliveHelper = nullptr;
        return ret;
    }

    void setHashAndHandleNotModified()
    {
        // Can only hash if we have content that's valid
        if (jsonValue.empty() || result() != boost::beast::http::status::ok)
        {
            return;
        }
        size_t hashval = std::hash<nlohmann::json>{}(jsonValue);
        std::string hexVal = "\"" + intToHexString(hashval, 8) + "\"";
        addHeader(boost::beast::http::field::etag, hexVal);
        if (expectedHash && hexVal == *expectedHash)
        {
            jsonValue = nullptr;
            result(boost::beast::http::status::not_modified);
        }
    }

    void setExpectedHash(std::string_view hash)
    {
        expectedHash = hash;
    }

  private:
    std::optional<std::string> expectedHash;
    bool completed = false;
    std::function<void(Response&)> completeRequestHandler;
    std::function<bool()> isAliveHelper;
};
} // namespace crow
