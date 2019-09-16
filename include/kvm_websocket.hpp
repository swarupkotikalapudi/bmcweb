#pragma once
#include <crow/app.h>
#include <crow/websocket.h>
#include <sys/socket.h>

#include <boost/container/flat_map.hpp>
#include <webserver_common.hpp>

namespace crow
{
namespace obmc_kvm
{

static constexpr const uint maxSessions = 4;
static constexpr const char* serverAddr = "127.0.0.1";
static constexpr const uint serverPort = 5900;

class KvmSession
{
  public:
    KvmSession(crow::websocket::Connection* conn) :
        conn(conn), doingWrite(false)
    {
        boost::asio::ip::tcp::endpoint endpoint(
            boost::asio::ip::make_address(serverAddr), serverPort);
        hostSocket = std::make_unique<boost::asio::ip::tcp::socket>(
            conn->get_io_context());
        hostSocket->async_connect(endpoint,
                                  std::bind(&KvmSession::connectHandler, this,
                                            std::placeholders::_1));
    }

    ~KvmSession()
    {
        hostSocket->shutdown(boost::asio::ip::tcp::socket::shutdown_both);
        hostSocket->close();
        hostSocket = nullptr;
        conn = nullptr;
    }

    void onMessage(const std::string& data)
    {
        if (hostSocket == nullptr || conn == nullptr)
        {
            return;
        }
        if (data.length() > inputBuffer.capacity())
        {
            BMCWEB_LOG_ERROR << "conn:" << &conn
                             << ", Buffer overrun when writing "
                             << data.length() << " bytes";
            conn->close("Buffer overrun");
            return;
        }

        BMCWEB_LOG_DEBUG << "conn:" << &conn << ", Read " << data.size()
                         << " bytes from websocket";
        boost::asio::buffer_copy(inputBuffer.prepare(data.size()),
                                 boost::asio::buffer(data));
        BMCWEB_LOG_DEBUG << "conn:" << &conn << ", Commiting " << data.size()
                         << " bytes from websocket";
        inputBuffer.commit(data.size());

        BMCWEB_LOG_DEBUG << "conn:" << &conn << ", inputbuffer size "
                         << inputBuffer.size();
        doWrite();
    }

  protected:
    void connectHandler(const boost::system::error_code& ec)
    {
        if (hostSocket == nullptr || conn == nullptr)
        {
            return;
        }
        if (ec)
        {
            BMCWEB_LOG_ERROR << "conn:" << &conn
                             << ", Couldn't connect to KVM socket port: " << ec;
            if (ec != boost::asio::error::operation_aborted)
            {
                conn->close("Error in connecting to KVM port");
            }
            return;
        }

        doRead();
    }

    void doRead()
    {
        if (hostSocket == nullptr || conn == nullptr)
        {
            return;
        }

        std::size_t bytes = outputBuffer.capacity() - outputBuffer.size();
        BMCWEB_LOG_DEBUG << "conn:" << &conn << ", Reading " << bytes
                         << " from kvm socket";
        hostSocket->async_read_some(
            outputBuffer.prepare(outputBuffer.capacity() - outputBuffer.size()),
            std::bind(&KvmSession::readDone, this, std::placeholders::_1,
                      std::placeholders::_2));
    }

    void readDone(const boost::system::error_code& ec, std::size_t bytesRead)
    {
        BMCWEB_LOG_DEBUG << "conn:" << &conn << ", read done.  Read "
                         << bytesRead << " bytes";
        if (hostSocket == nullptr || conn == nullptr)
        {
            return;
        }
        if (ec)
        {
            BMCWEB_LOG_ERROR << "conn:" << &conn
                             << ", Couldn't read from KVM socket port: " << ec;
            if (ec != boost::asio::error::operation_aborted)
            {
                conn->close("Error in connecting to KVM port");
            }
            return;
        }

        outputBuffer.commit(bytesRead);
        std::string_view payload(
            static_cast<const char*>(outputBuffer.data().data()), bytesRead);
        BMCWEB_LOG_DEBUG << "conn:" << &conn << ", Sending payload size "
                         << payload.size();
        conn->sendBinary(payload);
        outputBuffer.consume(bytesRead);

        doRead();
    }

    void doWrite()
    {
        if (hostSocket == nullptr || conn == nullptr)
        {
            return;
        }
        if (doingWrite)
        {
            BMCWEB_LOG_DEBUG << "conn:" << &conn
                             << ", Already writing.  Bailing out";
            return;
        }
        if (inputBuffer.size() == 0)
        {
            BMCWEB_LOG_DEBUG << "conn:" << &conn
                             << ", inputBuffer empty.  Bailing out";
            return;
        }

        doingWrite = true;
        hostSocket->async_write_some(inputBuffer.data(),
                                     std::bind(&KvmSession::WriteDone, this,
                                               std::placeholders::_1,
                                               std::placeholders::_2));
    }

    void WriteDone(const boost::system::error_code& ec,
                   std::size_t bytesWritten)
    {
        BMCWEB_LOG_DEBUG << "conn:" << &conn << ", Wrote " << bytesWritten
                         << "bytes";
        doingWrite = false;
        inputBuffer.consume(bytesWritten);

        if (hostSocket == nullptr || conn == nullptr)
        {
            return;
        }
        if (ec == boost::asio::error::eof)
        {
            conn->close("KVM socket port closed");
            return;
        }
        if (ec)
        {
            BMCWEB_LOG_ERROR << "conn:" << &conn
                             << ", Error in KVM socket write " << ec;
            if (ec != boost::asio::error::operation_aborted)
            {
                conn->close("Error in reading to host port");
            }
            return;
        }

        doWrite();
    }

    crow::websocket::Connection* conn;
    std::unique_ptr<boost::asio::ip::tcp::socket> hostSocket;
    boost::beast::flat_static_buffer<1024U * 50U> outputBuffer;
    boost::beast::flat_static_buffer<1024U> inputBuffer;
    bool doingWrite;
};

static boost::container::flat_map<crow::websocket::Connection*,
                                  std::unique_ptr<KvmSession>>
    sessions;

inline void requestRoutes(CrowApp& app)
{
    BMCWEB_ROUTE(app, "/kvm/0")
        .websocket()
        .onopen([](crow::websocket::Connection& conn) {
            BMCWEB_LOG_DEBUG << "Connection " << &conn << " opened";

            if (sessions.size() == maxSessions)
            {
                conn.close("Max sessions were already connected");
                return;
            }

            sessions[&conn] = std::make_unique<KvmSession>(&conn);
        })
        .onclose([](crow::websocket::Connection& conn,
                    const std::string& reason) { sessions.erase(&conn); })
        .onmessage([](crow::websocket::Connection& conn,
                      const std::string& data,
                      bool is_binary) { sessions[&conn]->onMessage(data); });
}

} // namespace obmc_kvm
} // namespace crow
