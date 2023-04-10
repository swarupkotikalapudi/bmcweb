#pragma once
#include "app.hpp"
#include "async_resp.hpp"

#include <sys/socket.h>

#include <boost/asio/local/stream_protocol.hpp>
#include <boost/container/flat_set.hpp>
#include <websocket.hpp>

namespace crow
{
namespace obmc_console
{

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static std::unique_ptr<boost::asio::local::stream_protocol::socket> hostSocket;

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static std::array<char, 4096> outputBuffer;
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static std::string inputBuffer;

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static boost::container::flat_set<crow::websocket::Connection*> sessions;

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static bool doingWrite = false;

inline void doWrite()
{
    if (doingWrite)
    {
        BMCWEB_LOG_DEBUG << "Already writing.  Bailing out";
        return;
    }

    if (inputBuffer.empty())
    {
        BMCWEB_LOG_DEBUG << "Outbuffer empty.  Bailing out";
        return;
    }

    if (!hostSocket)
    {
        BMCWEB_LOG_ERROR << "doWrite(): Socket closed.";
        return;
    }

    doingWrite = true;
    hostSocket->async_write_some(
        boost::asio::buffer(inputBuffer.data(), inputBuffer.size()),
        [](boost::beast::error_code ec, std::size_t bytesWritten) {
        doingWrite = false;
        inputBuffer.erase(0, bytesWritten);

        if (ec == boost::asio::error::eof)
        {
            for (crow::websocket::Connection* session : sessions)
            {
                session->close("Error in reading to host port");
            }
            return;
        }
        if (ec)
        {
            BMCWEB_LOG_ERROR << "Error in host serial write " << ec;
            return;
        }
        doWrite();
        });
}

inline void doRead()
{
    if (!hostSocket)
    {
        BMCWEB_LOG_ERROR << "doRead(): Socket closed.";
        return;
    }

    BMCWEB_LOG_DEBUG << "Reading from socket";
    hostSocket->async_read_some(
        boost::asio::buffer(outputBuffer.data(), outputBuffer.size()),
        [](const boost::system::error_code& ec, std::size_t bytesRead) {
        BMCWEB_LOG_DEBUG << "read done.  Read " << bytesRead << " bytes";
        if (ec)
        {
            BMCWEB_LOG_ERROR << "Couldn't read from host serial port: " << ec;
            for (crow::websocket::Connection* session : sessions)
            {
                session->close("Error in connecting to host port");
            }
            return;
        }
        std::string_view payload(outputBuffer.data(), bytesRead);
        for (crow::websocket::Connection* session : sessions)
        {
            session->sendBinary(payload);
        }
        doRead();
        });
}

inline void connectHandler(const boost::system::error_code& ec)
{
    if (ec)
    {
        BMCWEB_LOG_ERROR << "Couldn't connect to host serial port: " << ec;
        for (crow::websocket::Connection* session : sessions)
        {
            session->close("Error in connecting to host port");
        }
        return;
    }

    doWrite();
    doRead();
}

// Return true if connection is still active.
inline bool connectionActive(crow::websocket::Connection& conn)
{
    return !(sessions.find(&conn) == sessions.end());
}

// Remove the connection if it is still active.
inline bool removeConnection(crow::websocket::Connection& conn)
{
    if (!connectionActive(conn))
    {
        BMCWEB_LOG_ERROR << "Couldn't find the connection";
        return false;
    }
    else
    {
        sessions.erase(&conn);
        if (sessions.empty())
        {
            hostSocket = nullptr;
            inputBuffer.clear();
            inputBuffer.shrink_to_fit();
        }
        return true;
    }
}

inline void connectConsoleSocket(crow::websocket::Connection& conn,
                                 const boost::system::error_code& ec,
                                 const std::vector<uint8_t>& socketNameBytes)
{
    if (ec)
    {
        BMCWEB_LOG_ERROR << "Failed to get socketName property of the console."
                         << " DBUS error: " << ec.message();
        if (removeConnection(conn))
        {
            conn.close("Failed to get socketName property of the console");
        }
        return;
    }

    // Make sure that connection is still open.
    if (!connectionActive(conn))
    {
        return;
    }

    std::string consoleSocketName(socketNameBytes.begin(),
                                  socketNameBytes.end());

    // Console socket name starts with null character
    BMCWEB_LOG_DEBUG << "Console web socket path: " << conn.req.target()
                     << " Console socketName: " << consoleSocketName.substr(1);

    if (hostSocket == nullptr)
    {
        boost::asio::local::stream_protocol::endpoint ep(consoleSocketName);

        hostSocket =
            std::make_unique<boost::asio::local::stream_protocol::socket>(
                conn.getIoContext());
        hostSocket->async_connect(ep, connectHandler);
    }
    else
    {
        // Connection is not from same endpoint so don't allow to shared it
        if (consoleSocketName != hostSocket->remote_endpoint().path())
        {
            if (removeConnection(conn))
            {
                conn.close("Connection from different enpoint is not allowed.");
            }
        }
    }
}

inline void
    findMatchingConsole(crow::websocket::Connection& conn,
                        const boost::system::error_code& ec,
                        const dbus::utility::MapperGetSubTreeResponse& subtree)
{
    bool consoleFound = false;

    // Make sure that connection is still open.
    if (!connectionActive(conn))
    {
        return;
    }

    BMCWEB_LOG_DEBUG << "Web socket path: " << conn.req.target();

    if (ec)
    {
        BMCWEB_LOG_WARNING << "getSubTree() for consoles failed. DBUS error:"
                           << ec.message();
        if (removeConnection(conn))
        {
            conn.close("getSubTree() for consoles failed.");
        }
        return;
    }

    // Iterate over all retrieved ObjectPaths.
    for (const auto& object : subtree)
    {
        const std::string& path = object.first;
        sdbusplus::message::object_path objPath(object.first);
        std::string webSocketPath("/" + objPath.filename());

        BMCWEB_LOG_DEBUG << "Console Object path = " << path
                         << " webSocketPath = " << webSocketPath;

        // Look for matching rules string for the console
        if (conn.req.target() != webSocketPath)
        {
            continue;
        }

        const std::vector<std::pair<std::string, std::vector<std::string>>>&
            connectionNames = object.second;
        if (connectionNames.empty())
        {
            continue;
        }

        const auto& connection = connectionNames[0];
        const std::string& service = connection.first;

        BMCWEB_LOG_DEBUG << "Console connection service: " << service;

        consoleFound = true;

        // This Socket name propery returns stream of bytes as
        // it can have valid null characters.
        sdbusplus::asio::getProperty<std::vector<uint8_t>>(
            *crow::connections::systemBus, service, path,
            "xyz.openbmc_project.Console.Access", "SocketName",
            [&conn](const boost::system::error_code& ec1,
                    const std::vector<uint8_t>& socketNameBytes) {
            connectConsoleSocket(conn, ec1, socketNameBytes);
            });

        break;
    }

    // Could not find the console
    if (!consoleFound)
    {
        BMCWEB_LOG_WARNING << "Console not found.";
        if (removeConnection(conn))
        {
            conn.close("Console not found");
        }
    }
}

// Query consoles from DBUS and find the matching to the
// rules string.
inline void onOpen(crow::websocket::Connection& conn)
{
    BMCWEB_LOG_DEBUG << "Connection " << &conn << " opened";

    // Save the connection in the map
    sessions.insert(&conn);

    // mapper call lambda
    constexpr std::array<std::string_view, 1> interfaces = {
        "xyz.openbmc_project.Console.Access"};

    dbus::utility::getSubTree(
        "/xyz/openbmc_project/console", 1, interfaces,
        [&conn](const boost::system::error_code& ec,
                const dbus::utility::MapperGetSubTreeResponse& subtree) {
        findMatchingConsole(conn, ec, subtree);
        });
}

inline void requestRoutes(App& app)
{
    BMCWEB_ROUTE(app, "/console0")
        .privileges({{"ConfigureComponents", "ConfigureManager"}})
        .websocket()
        .onopen(onOpen)
        .onclose([](crow::websocket::Connection& conn,
                    [[maybe_unused]] const std::string& reason) {
            BMCWEB_LOG_INFO << "Closing websocket. Reason: " << reason;

            removeConnection(conn);
        })
        .onmessage([]([[maybe_unused]] crow::websocket::Connection& conn,
                      const std::string& data, [[maybe_unused]] bool isBinary) {
            inputBuffer += data;
            doWrite();
        });
}
} // namespace obmc_console
} // namespace crow
