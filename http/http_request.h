#pragma once

#include "common.h"

#include "sessions.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/url/url_view.hpp>

namespace crow
{

struct FormPart
{
    boost::beast::http::fields fields;
    std::string content;
};

struct Request
{
    boost::beast::http::request<boost::beast::http::string_body>& req;
    boost::beast::http::fields& fields;
    std::string_view url{};
    boost::urls::url_view urlView{};
    boost::urls::url_view::params_type urlParams{};
    bool isSecure{false};

    const std::string& body;

    boost::container::small_vector<FormPart, 4> mime_fields;

    boost::asio::io_context* ioService{};
    boost::asio::ip::address ipAddress;

    std::shared_ptr<persistent_data::UserSession> session;

    std::string userRole{};
    Request(
        boost::beast::http::request<boost::beast::http::string_body>& reqIn) :
        req(reqIn),
        fields(reqIn.base()), body(reqIn.body())
    {}

    boost::beast::http::verb method() const
    {
        return req.method();
    }

    std::string_view getHeaderValue(std::string_view key) const
    {
        return req[key];
    }

    std::string_view getHeaderValue(boost::beast::http::field key) const
    {
        return req[key];
    }

    std::string_view methodString() const
    {
        return req.method_string();
    }

    std::string_view target() const
    {
        return req.target();
    }

    unsigned version() const
    {
        return req.version();
    }

    bool isUpgrade() const
    {
        return boost::beast::websocket::is_upgrade(req);
    }

    bool keepAlive() const
    {
        return req.keep_alive();
    }
};

} // namespace crow
