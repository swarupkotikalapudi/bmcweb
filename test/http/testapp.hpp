#pragma once

#include "async_resp.hpp"
#include "http_request.hpp"
#include "http_server.hpp"
#include "logging.hpp"
#include "utility.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#ifdef BMCWEB_ENABLE_SSL
#include <boost/asio/ssl/context.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>
#endif

#include <chrono>
#include <cstdint>
#include <functional>
#include <future>
#include <memory>
#include <string>
#include <utility>

#ifdef BMCWEB_ENABLE_SSL
using ssl_context_t = boost::asio::ssl::context;
#endif
template <typename Router>
class TestApp
{
  public:
#ifdef BMCWEB_ENABLE_SSL
    using ssl_socket_t = boost::beast::ssl_stream<boost::asio::ip::tcp::socket>;
    using ssl_server_t = crow::Server<TestApp, ssl_socket_t>;
#else
    using socket_t = boost::asio::ip::tcp::socket;
    using server_t = Server<TestApp, socket_t>;
#endif

    explicit TestApp(std::shared_ptr<boost::asio::io_context> ioIn =
                         std::make_shared<boost::asio::io_context>()) :
        io(std::move(ioIn))
    {}
    ~TestApp()
    {
        this->stop();
    }

    TestApp(const TestApp&) = delete;
    TestApp(TestApp&&) = delete;
    TestApp& operator=(const TestApp&) = delete;
    TestApp& operator=(const TestApp&&) = delete;

    template <typename Adaptor>
    void handleUpgrade(const crow::Request& req,
                       const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                       Adaptor&& adaptor)
    {
        router.handleUpgrade(req, asyncResp, std::forward<Adaptor>(adaptor));
    }

    void handle(crow::Request& req,
                const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
    {
        router.handle(req, asyncResp);
    }

    TestApp& socket(int existingSocket)
    {
        socketFd = existingSocket;
        return *this;
    }

    TestApp& port(std::uint16_t port)
    {
        portUint = port;
        return *this;
    }

    TestApp& bindaddr(std::string bindaddr)
    {
        bindaddrStr = std::move(bindaddr);
        return *this;
    }

    void validate()
    {
        router.validate();
    }

    void run()
    {
        validate();
#ifdef BMCWEB_ENABLE_SSL
        if (-1 == socketFd)
        {
            sslServer = std::make_unique<ssl_server_t>(
                this, bindaddrStr, portUint, sslContext, io);
        }
        else
        {
            sslServer = std::make_unique<ssl_server_t>(this, socketFd,
                                                       sslContext, io);
        }
        sslServer->run();

#else

        if (-1 == socketFd)
        {
            server = std::make_unique<server_t>(this, bindaddrStr, portUint,
#ifdef BMCWEB_ENABLE_SSL
                                                nullptr,
#endif
                                                io);
        }
        else
        {
            server = std::make_unique<server_t>(this, socketFd,
#ifdef BMCWEB_ENABLE_SSL
                                                nullptr,
#endif
                                                io);
        }
        server->run();

#endif
    }

    void stop()
    {
        io->stop();
    }

    void debugPrint()
    {
        router.debugPrint();
    }

    std::vector<const std::string*> getRoutes()
    {
        const std::string root;
        return router.getRoutes(root);
    }
    std::vector<const std::string*> getRoutes(const std::string& parent)
    {
        return router.getRoutes(parent);
    }

#ifdef BMCWEB_ENABLE_SSL
    TestApp& ssl(std::shared_ptr<boost::asio::ssl::context>&& ctx)
    {
        sslContext = std::move(ctx);

        return *this;
    }

    std::shared_ptr<ssl_context_t> sslContext = nullptr;

#else
    template <typename T>
    TestApp& ssl(T&&)
    {
        // We can't call .ssl() member function unless BMCWEB_ENABLE_SSL is
        // defined.
        static_assert(
            // make static_assert dependent to T; always false
            std::is_base_of<T, void>::value,
            "Define BMCWEB_ENABLE_SSL to enable ssl support.");
        return *this;
    }
#endif

  private:
    std::shared_ptr<boost::asio::io_context> io;
#ifdef BMCWEB_ENABLE_SSL
    uint16_t portUint = 443;
#else
    uint16_t portUint = 8081;
#endif
    std::string bindaddrStr = "0.0.0.0";
    int socketFd = -1;
    Router router;

#ifdef BMCWEB_ENABLE_SSL
    std::unique_ptr<ssl_server_t> sslServer;
#else
    std::unique_ptr<server_t> server;
#endif
};
