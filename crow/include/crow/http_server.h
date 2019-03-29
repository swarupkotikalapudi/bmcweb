#pragma once

#include <atomic>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/ssl/context.hpp>
#if BOOST_VERSION >= 107000
#include <boost/beast/ssl/ssl_stream.hpp>
#else
#include <boost/beast/experimental/core/ssl_stream.hpp>
#endif
#include <boost/date_time/posix_time/posix_time.hpp>
#include <chrono>
#include <cstdint>
#include <future>
#include <memory>
#include <ssl_key_handler.hpp>
#include <utility>
#include <vector>

#include "crow/http_connection.h"
#include "crow/logging.h"
#include "crow/timer_queue.h"

namespace crow
{
using namespace boost;
using tcp = asio::ip::tcp;

template <typename Handler, typename Adaptor = boost::asio::ip::tcp::socket,
          typename... Middlewares>
class Server
{
  public:
    Server(Handler* handler, std::unique_ptr<tcp::acceptor>&& acceptor,
           std::tuple<Middlewares...>* middlewares = nullptr,
           boost::asio::ssl::context* adaptor_ctx = nullptr,
           std::shared_ptr<boost::asio::io_context> io =
               std::make_shared<boost::asio::io_context>()) :
        ioService(std::move(io)),
        acceptor(std::move(acceptor)),
        signals(*ioService, SIGINT, SIGTERM, SIGHUP), tickTimer(*ioService),
        handler(handler), middlewares(middlewares), adaptorCtx(adaptor_ctx)
    {
    }

    Server(Handler* handler, const std::string& bindaddr, uint16_t port,
           std::tuple<Middlewares...>* middlewares = nullptr,
           boost::asio::ssl::context* adaptor_ctx = nullptr,
           std::shared_ptr<boost::asio::io_context> io =
               std::make_shared<boost::asio::io_context>()) :
        Server(handler,
               std::make_unique<tcp::acceptor>(
                   *io, tcp::endpoint(boost::asio::ip::make_address(bindaddr),
                                      port)),
               middlewares, adaptor_ctx, io)
    {
    }

    Server(Handler* handler, int existing_socket,
           std::tuple<Middlewares...>* middlewares = nullptr,
           boost::asio::ssl::context* adaptor_ctx = nullptr,
           std::shared_ptr<boost::asio::io_context> io =
               std::make_shared<boost::asio::io_context>()) :
        Server(handler,
               std::make_unique<tcp::acceptor>(*io, boost::asio::ip::tcp::v6(),
                                               existing_socket),
               middlewares, adaptor_ctx, io)
    {
    }

    void setTickFunction(std::chrono::milliseconds d, std::function<void()> f)
    {
        tickInterval = d;
        tickFunction = f;
    }

    void onTick()
    {
        tickFunction();
        tickTimer.expires_from_now(
            boost::posix_time::milliseconds(tickInterval.count()));
        tickTimer.async_wait([this](const boost::system::error_code& ec) {
            if (ec)
            {
                return;
            }
            onTick();
        });
    }

    void updateDateStr()
    {
        auto lastTimeT = time(0);
        tm myTm{};

#ifdef _MSC_VER
        gmtime_s(&my_tm, &last_time_t);
#else
        gmtime_r(&lastTimeT, &myTm);
#endif
        dateStr.resize(100);
        size_t dateStrSz =
            strftime(&dateStr[0], 99, "%a, %d %b %Y %H:%M:%S GMT", &myTm);
        dateStr.resize(dateStrSz);
    };

    void run()
    {
        loadCertificate();
        updateDateStr();

        getCachedDateStr = [this]() -> std::string {
            static std::chrono::time_point<std::chrono::steady_clock>
                lastDateUpdate = std::chrono::steady_clock::now();
            if (std::chrono::steady_clock::now() - lastDateUpdate >=
                std::chrono::seconds(10))
            {
                lastDateUpdate = std::chrono::steady_clock::now();
                updateDateStr();
            }
            return this->dateStr;
        };

        boost::asio::deadline_timer timer(*ioService);
        timer.expires_from_now(boost::posix_time::seconds(1));

        std::function<void(const boost::system::error_code& ec)> timerHandler;
        timerHandler = [&](const boost::system::error_code& ec) {
            if (ec)
            {
                return;
            }
            timerQueue.process();
            timer.expires_from_now(boost::posix_time::seconds(1));
            timer.async_wait(timerHandler);
        };
        timer.async_wait(timerHandler);

        if (tickFunction && tickInterval.count() > 0)
        {
            tickTimer.expires_from_now(
                boost::posix_time::milliseconds(tickInterval.count()));
            tickTimer.async_wait([this](const boost::system::error_code& ec) {
                if (ec)
                {
                    return;
                }
                onTick();
            });
        }

        BMCWEB_LOG_INFO << serverName << " server is running, local endpoint "
                        << acceptor->local_endpoint();
        startAsyncWaitForSignal();
        doAccept();
    }

    void loadCertificate()
    {
#ifdef BMCWEB_ENABLE_SSL
        std::string sslPemFile("server.pem");
        std::cout << "Building SSL Context\n";
        ensuressl::ensureOpensslKeyPresentAndValid(sslPemFile);
        std::cout << "SSL Enabled\n";
        boost::asio::ssl::context sslContext =
            ensuressl::getSslContext(sslPemFile);
        handler->ssl(std::move(sslContext));
#endif
    }

    void startAsyncWaitForSignal()
    {
        signals.async_wait([this](const boost::system::error_code& ec,
                                  int signalNo) {
            if (ec)
            {
                BMCWEB_LOG_INFO << "Error in signal handler" << ec.message();
            }
            else
            {
                if (signalNo == SIGHUP)
                {
                    BMCWEB_LOG_INFO << "Receivied reload signal";
                    loadCertificate();
                    this->startAsyncWaitForSignal();
                }
                else
                {
                    stop();
                }
            }
        });
    }

    void stop()
    {
        ioService->stop();
    }

    void doAccept()
    {
        std::optional<Adaptor> adaptorTemp;
        if constexpr (std::is_same<Adaptor,
                                   boost::beast::ssl_stream<
                                       boost::asio::ip::tcp::socket>>::value)
        {
            adaptorTemp = Adaptor(*ioService, *adaptorCtx);
            Connection<Adaptor, Handler, Middlewares...>* p =
                new Connection<Adaptor, Handler, Middlewares...>(
                    *ioService, handler, serverName, middlewares,
                    getCachedDateStr, timerQueue,
                    std::move(adaptorTemp.value()));

            acceptor->async_accept(p->socket().next_layer(),
                                   [this, p](boost::system::error_code ec) {
                                       if (!ec)
                                       {
                                           boost::asio::post(
                                               *this->ioService,
                                               [p] { p->start(); });
                                       }
                                       else
                                       {
                                           delete p;
                                       }
                                       doAccept();
                                   });
        }
        else
        {
            adaptorTemp = Adaptor(*ioService);
            Connection<Adaptor, Handler, Middlewares...>* p =
                new Connection<Adaptor, Handler, Middlewares...>(
                    *ioService, handler, serverName, middlewares,
                    getCachedDateStr, timerQueue,
                    std::move(adaptorTemp.value()));

            acceptor->async_accept(
                p->socket(), [this, p](boost::system::error_code ec) {
                    if (!ec)
                    {
                        boost::asio::post(*this->ioService,
                                          [p] { p->start(); });
                    }
                    else
                    {
                        delete p;
                    }
                    doAccept();
                });
        }
    }

  private:
    std::shared_ptr<asio::io_context> ioService;
    detail::TimerQueue timerQueue;
    std::function<std::string()> getCachedDateStr;
    std::unique_ptr<tcp::acceptor> acceptor;
    boost::asio::signal_set signals;
    boost::asio::deadline_timer tickTimer;

    std::string dateStr;

    Handler* handler;
    std::string serverName = "iBMC";

    std::chrono::milliseconds tickInterval{};
    std::function<void()> tickFunction;

    std::tuple<Middlewares...>* middlewares;

#ifdef BMCWEB_ENABLE_SSL
    bool useSsl{false};
#endif
    boost::asio::ssl::context* adaptorCtx;
}; // namespace crow
} // namespace crow
