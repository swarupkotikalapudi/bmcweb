/*
// Copyright (c) 2020 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
*/
#pragma once
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

namespace crow
{

enum class ConnState
{
    initializing,
    connected,
    closed
};

class HttpClient : public std::enable_shared_from_this<HttpClient>
{
  private:
    boost::beast::tcp_stream conn;
    boost::beast::flat_buffer buffer;
    boost::beast::http::request<boost::beast::http::string_body> req;
    boost::beast::http::response<boost::beast::http::string_body> res;
    boost::asio::ip::tcp::resolver::results_type endpoint;
    std::vector<std::pair<std::string, std::string>> headers;
    ConnState state;
    std::string host;
    std::string port;

  public:
    explicit HttpClient(boost::asio::io_context& ioc, const std::string& destIP,
                        const std::string& destPort) :
        conn(ioc),
        host(destIP), port(destPort)
    {
        boost::asio::ip::tcp::resolver resolver(ioc);
        endpoint = resolver.resolve(host, port);
        state = ConnState::initializing;
    }

    void doConnectAndSend(const std::string& path, const std::string& data)
    {
        BMCWEB_LOG_DEBUG << "doConnectAndSend " << host << ":" << port;

        req.version(static_cast<int>(11)); // HTTP 1.1
        req.target(path);
        req.method(boost::beast::http::verb::post);

        // Set headers
        for (const auto& [key, value] : headers)
        {
            req.set(key, value);
        }
        req.set(boost::beast::http::field::host, host);
        req.keep_alive(true);

        req.body() = data;
        req.prepare_payload();

        // Set a timeout on the operation
        conn.expires_after(std::chrono::seconds(30));
        conn.async_connect(endpoint, [this, self(shared_from_this())](
                                         const boost::beast::error_code& ec,
                                         const boost::asio::ip::tcp::resolver::
                                             results_type::endpoint_type& ep) {
            if (ec)
            {
                BMCWEB_LOG_ERROR << "Connect " << ep
                                 << " failed: " << ec.message();
                return;
            }
            state = ConnState::connected;
            BMCWEB_LOG_DEBUG << "Connected to: " << ep;

            sendMessage();
        });
    }

    void sendMessage()
    {
        if (state != ConnState::connected)
        {
            BMCWEB_LOG_DEBUG << "Not connected to: " << host;
            return;
        }

        // Set a timeout on the operation
        conn.expires_after(std::chrono::seconds(30));

        // Send the HTTP request to the remote host
        boost::beast::http::async_write(
            conn, req,
            [this,
             self(shared_from_this())](const boost::beast::error_code& ec,
                                       const std::size_t& bytesTransferred) {
                if (ec)
                {
                    BMCWEB_LOG_ERROR << "sendMessage() failed: "
                                     << ec.message();
                    this->doClose();
                    return;
                }
                BMCWEB_LOG_DEBUG << "sendMessage() bytes transferred: "
                                 << bytesTransferred;
                boost::ignore_unused(bytesTransferred);

                this->recvMessage();
            });
    }

    void recvMessage()
    {
        if (state != ConnState::connected)
        {
            BMCWEB_LOG_DEBUG << "Not connected to: " << host;
            return;
        }

        // Receive the HTTP response
        boost::beast::http::async_read(
            conn, buffer, res,
            [this,
             self(shared_from_this())](const boost::beast::error_code& ec,
                                       const std::size_t& bytesTransferred) {
                if (ec)
                {
                    BMCWEB_LOG_ERROR << "recvMessage() failed: "
                                     << ec.message();
                    this->doClose();
                    return;
                }
                BMCWEB_LOG_DEBUG << "recvMessage() bytes transferred: "
                                 << bytesTransferred;
                boost::ignore_unused(bytesTransferred);

                // Discard received data. We are not interested.
                BMCWEB_LOG_DEBUG << "recvMessage() data: " << res;

                this->doClose();
            });
    }

    void doClose()
    {
        boost::beast::error_code ec;
        conn.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);

        state = ConnState::closed;
        // not_connected happens sometimes so don't bother reporting it.
        if (ec && ec != boost::beast::errc::not_connected)
        {
            BMCWEB_LOG_ERROR << "shutdown failed: " << ec.message();
            return;
        }
        BMCWEB_LOG_DEBUG << "Connection closed gracefully";
    }

    void setHeaders(
        const std::vector<std::pair<std::string, std::string>>& httpHeaders)
    {
        headers = httpHeaders;
    }

    ConnState getState()
    {
        return state;
    }
};

} // namespace crow
