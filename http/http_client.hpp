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
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/basic_endpoint.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/version.hpp>
#include <boost/lexical_cast.hpp>

#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <queue>
#include <string>

namespace crow
{

static constexpr uint8_t maxRequestQueueSize = 50;
static constexpr unsigned int httpReadBodyLimit = 8000;

enum class ConnState
{
    initialized,
    connected,
    idle,
    closed,
    suspended,
    terminated,
    abortConnection,
    retry
};

class HttpClient : public std::enable_shared_from_this<HttpClient>
{
  private:
    boost::beast::tcp_stream conn;
    boost::asio::steady_timer timer;
    boost::beast::flat_static_buffer<httpReadBodyLimit> buffer;
    boost::beast::http::request<boost::beast::http::string_body> req;
    std::optional<
        boost::beast::http::response_parser<boost::beast::http::string_body>>
        parser;
    boost::asio::ip::tcp::endpoint endpoint;
    boost::circular_buffer_space_optimized<std::string> requestDataQueue{};
    std::string subId;
    std::string host;
    std::string port;
    std::string uri;
    uint32_t retryCount;
    uint32_t maxRetryAttempts;
    uint32_t retryIntervalSecs;
    std::string retryPolicyAction;
    bool runningTimer;
    ConnState state;
    bool connBusy;

    void doResolve()
    {
        connBusy = true;
        BMCWEB_LOG_DEBUG << "Trying to resolve: " << host << ":" << port;
        uint64_t flag = 0;
        crow::connections::systemBus->async_method_call(
            [self(shared_from_this())](
                const boost::system::error_code ec,
                const std::vector<
                    std::tuple<int32_t, int32_t, std::vector<uint8_t>>>& resp,
                const std::string& hostName, const uint64_t flagNum) {
                if (ec)
                {
                    BMCWEB_LOG_ERROR << "Resolve failed: " << ec.message();
                    self->state = ConnState::retry;
                    self->connBusy = false;
                    self->handleConnState();
                    return;
                }
                BMCWEB_LOG_DEBUG << "ResolveHostname returned: " << hostName
                                 << ":" << flagNum;

                // Extract the IP address from the response
                std::vector<boost::asio::ip::tcp::endpoint> endpointList;
                for (auto resolveList : resp)
                {
                    std::vector<uint8_t> ipAddress = std::get<2>(resolveList);
                    boost::asio::ip::tcp::endpoint endpoint;
                    if (ipAddress.size() == 4) // ipv4 address
                    {
                        BMCWEB_LOG_DEBUG << "ipv4 address";
                        boost::asio::ip::address_v4 ipv4Addr(
                            {ipAddress[0], ipAddress[1], ipAddress[2],
                             ipAddress[3]});
                        endpoint.address(ipv4Addr);
                        endpoint.port(
                            boost::lexical_cast<uint16_t>(self->port.c_str()));
                    }
                    else if (ipAddress.size() == 16) // ipv6 address
                    {
                        boost::asio::ip::address_v6 ipv6Addr(
                            {ipAddress[0], ipAddress[1], ipAddress[2],
                             ipAddress[3], ipAddress[4], ipAddress[5],
                             ipAddress[6], ipAddress[7], ipAddress[8],
                             ipAddress[9], ipAddress[10], ipAddress[11],
                             ipAddress[12], ipAddress[13], ipAddress[14],
                             ipAddress[15]});

                        endpoint.address(ipv6Addr);
                        endpoint.port(
                            boost::lexical_cast<uint16_t>(self->port.c_str()));
                        BMCWEB_LOG_DEBUG << "ipv6 address";
                    }
                    else
                    {
                        BMCWEB_LOG_ERROR
                            << "Resolve failed to fetch the IP address";
                        self->state = ConnState::retry;
                        self->connBusy = false;
                        self->handleConnState();
                    }
                    BMCWEB_LOG_DEBUG << "resolved endpoint is : " << endpoint;
                    endpointList.push_back(endpoint);
                }
                self->connBusy = false;
                self->doConnect(endpointList);
            },
            "org.freedesktop.resolve1", "/org/freedesktop/resolve1",
            "org.freedesktop.resolve1.Manager", "ResolveHostname", 0, host,
            AF_UNSPEC, flag);
    }

    void doConnect(
        const std::vector<boost::asio::ip::tcp::endpoint>& endpointList)
    {
        connBusy = true;

        BMCWEB_LOG_DEBUG << "Trying to connect to: " << host << ":" << port;

        conn.expires_after(std::chrono::seconds(30));
        conn.async_connect(
            endpointList, [self(shared_from_this())](
                              const boost::beast::error_code ec,
                              const boost::asio::ip::tcp::endpoint& endpoint) {
                if (ec)
                {
                    BMCWEB_LOG_ERROR << "Connect " << endpoint
                                     << " failed: " << ec.message();
                    self->state = ConnState::retry;
                    self->connBusy = false;
                    self->handleConnState();
                    return;
                }
                self->state = ConnState::connected;
                BMCWEB_LOG_DEBUG << "Connected to: " << endpoint;

                self->connBusy = false;
                self->handleConnState();
            });
    }

    void sendMessage(const std::string& data)
    {
        connBusy = true;

        BMCWEB_LOG_DEBUG << __FUNCTION__ << "(): " << host << ":" << port;

        req.body() = data;
        req.prepare_payload();

        // Send the HTTP request to the remote host
        boost::beast::http::async_write(
            conn, req,
            [self(shared_from_this())](const boost::beast::error_code& ec,
                                       const std::size_t& bytesTransferred) {
                if (ec)
                {
                    BMCWEB_LOG_ERROR << "sendMessage() failed: "
                                     << ec.message();
                    // Start fresh connection
                    self->state = ConnState::retry;
                    self->connBusy = false;
                    self->handleConnState();
                    return;
                }
                BMCWEB_LOG_DEBUG << "sendMessage() bytes transferred: "
                                 << bytesTransferred;
                boost::ignore_unused(bytesTransferred);

                self->recvMessage();
            });
    }

    void recvMessage()
    {
        parser.emplace(std::piecewise_construct, std::make_tuple());
        parser->body_limit(httpReadBodyLimit);
        // Since these are all push style eventing, we are not
        // bothered about response body parsing.
        // Check only for the response header
        parser->skip(true);

        // Receive the HTTP response
        boost::beast::http::async_read(
            conn, buffer, *parser,
            [self(shared_from_this())](const boost::beast::error_code& ec,
                                       const std::size_t& bytesTransferred) {
                if (ec)
                {
                    BMCWEB_LOG_ERROR << "recvMessage() failed: "
                                     << ec.message();
                    // Start fresh connection
                    self->state = ConnState::retry;
                    self->connBusy = false;
                    self->handleConnState();
                    return;
                }
                BMCWEB_LOG_DEBUG << "recvMessage() bytes transferred: "
                                 << bytesTransferred;
                BMCWEB_LOG_DEBUG << "recvMessage() data: "
                                 << self->parser->get();

                // Send is successful, Lets remove data from queue
                // check for next request data in queue.
                if (!self->requestDataQueue.empty())
                {
                    self->requestDataQueue.pop_front();
                }
                self->state = ConnState::idle;

                // Keep the connection alive if server supports it
                // Else close the connection
                BMCWEB_LOG_DEBUG << "recvMessage() keepalive : "
                                 << self->parser->keep_alive();
                if (!self->parser->keep_alive())
                {
                    // Abort the connection since server is not keep-alive
                    // enabled
                    self->state = ConnState::abortConnection;
                }
                // Transfer ownership of the response
                self->parser->release();

                self->connBusy = false;
                self->handleConnState();
            });
    }

    void doClose()
    {
        connBusy = true;
        boost::beast::error_code ec;
        conn.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        conn.close();

        // not_connected happens sometimes so don't bother reporting it.
        if (ec && ec != boost::beast::errc::not_connected)
        {
            BMCWEB_LOG_ERROR << "shutdown failed: " << ec.message();
            return;
        }
        BMCWEB_LOG_DEBUG << "Connection closed gracefully";
        if ((state != ConnState::suspended) && (state != ConnState::terminated))
        {
            state = ConnState::closed;
            connBusy = false;
            handleConnState();
        }
    }

    void checkQueueAndRetry()
    {
        if (retryCount >= maxRetryAttempts)
        {
            BMCWEB_LOG_ERROR << "Maximum number of retries reached.";

            // Clear queue.
            while (!requestDataQueue.empty())
            {
                requestDataQueue.pop_front();
            }

            BMCWEB_LOG_DEBUG << "Retry policy: " << retryPolicyAction;
            if (retryPolicyAction == "TerminateAfterRetries")
            {
                // TODO: delete subscription
                state = ConnState::terminated;
            }
            if (retryPolicyAction == "SuspendRetries")
            {
                state = ConnState::suspended;
            }
            // Reset the retrycount to zero so that client can try connecting
            // again if needed
            retryCount = 0;
            handleConnState();
            return;
        }

        if (runningTimer)
        {
            BMCWEB_LOG_DEBUG << "Retry timer is already running.";
            return;
        }
        runningTimer = true;

        retryCount++;

        BMCWEB_LOG_DEBUG << "Attempt retry after " << retryIntervalSecs
                         << " seconds. RetryCount = " << retryCount;
        timer.expires_after(std::chrono::seconds(retryIntervalSecs));
        timer.async_wait(
            [self = shared_from_this()](const boost::system::error_code ec) {
                if (ec)
                {
                    BMCWEB_LOG_ERROR << "async_wait failed: " << ec.message();
                    // Ignore the error and continue the retry loop to attempt
                    // sending the event as per the retry policy
                }
                self->runningTimer = false;
                self->connBusy = false;
                // Lets close connection and start from resolve.
                self->doClose();
            });
        return;
    }

    void handleConnState()
    {
        if (connBusy)
        {
            BMCWEB_LOG_DEBUG << "Async operation is already in progress";
            return;
        }
        switch (state)
        {
            case ConnState::initialized:
            case ConnState::closed:
            {
                if (requestDataQueue.empty())
                {
                    BMCWEB_LOG_DEBUG << "requestDataQueue is empty";
                    return;
                }
                doResolve();
                break;
            }
            case ConnState::suspended:
            case ConnState::terminated:
            {
                doClose();
                break;
            }
            case ConnState::retry:
            {
                // In case of failures during connect and handshake
                // the retry policy will be applied
                checkQueueAndRetry();
                break;
            }
            case ConnState::connected:
            case ConnState::idle:
            {
                // State idle means, previous attempt is successful
                // State connected means, client connection is established
                // successfully
                if (requestDataQueue.empty())
                {
                    BMCWEB_LOG_DEBUG << "requestDataQueue is empty";
                    return;
                }
                std::string data = requestDataQueue.front();
                sendMessage(data);
                break;
            }
            case ConnState::abortConnection:
            {
                // Server did not want to keep alive the session
                doClose();
                break;
            }
            default:
                break;
        }
    }

  public:
    explicit HttpClient(boost::asio::io_context& ioc, const std::string& id,
                        const std::string& destIP, const std::string& destPort,
                        const std::string& destUri) :
        conn(ioc),
        timer(ioc), req(boost::beast::http::verb::post, destUri, 11), subId(id),
        host(destIP), port(destPort), uri(destUri), retryCount(0),
        maxRetryAttempts(5), retryIntervalSecs(0),
        retryPolicyAction("TerminateAfterRetries"), runningTimer(false),
        state(ConnState::initialized), connBusy(false)
    {
        // Set the request header
        req.set(boost::beast::http::field::host, host);
        req.set(boost::beast::http::field::content_type, "application/json");
        req.keep_alive(true);

        requestDataQueue.set_capacity(maxRequestQueueSize);
    }

    void sendData(const std::string& data)
    {
        if ((state == ConnState::suspended) || (state == ConnState::terminated))
        {
            return;
        }
        requestDataQueue.push_back(data);
        handleConnState();
        return;
    }

    void addHeaders(
        const std::vector<std::pair<std::string, std::string>>& httpHeaders)
    {
        // Set custom headers
        for (const auto& [key, value] : httpHeaders)
        {
            req.set(key, value);
        }
    }

    void setRetryConfig(const uint32_t retryAttempts,
                        const uint32_t retryTimeoutInterval)
    {
        maxRetryAttempts = retryAttempts;
        retryIntervalSecs = retryTimeoutInterval;
    }

    void setRetryPolicy(const std::string& retryPolicy)
    {
        retryPolicyAction = retryPolicy;
    }
};

} // namespace crow
