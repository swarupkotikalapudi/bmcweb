#pragma once

#include <app.hpp>
#include <include/obmc_websocket.hpp>

namespace crow
{
namespace obmc_console
{
static std::shared_ptr<obmc_websocket::Websocket> consWebSocket;

inline void requestRoutes(App& app)
{
    BMCWEB_ROUTE(app, "/console0")
        .privileges({{"ConfigureComponents", "ConfigureManager"}})
        .websocket()
        .onopen([](crow::websocket::Connection& conn,
                   const std::shared_ptr<bmcweb::AsyncResp>&) {
            BMCWEB_LOG_DEBUG << "Connection " << &conn << " opened";

            const std::string consoleName("\0obmc-console", 13);
            if (consWebSocket)
            {
                consWebSocket->addConnection(conn);
            }
            else
            {
                consWebSocket = std::make_shared<obmc_websocket::Websocket>(
                    consoleName, conn);
                consWebSocket->connect();
            }
        })
        .onclose([](crow::websocket::Connection& conn,
                    [[maybe_unused]] const std::string& reason) {
            BMCWEB_LOG_INFO << "Closing websocket. Reason: " << reason;

            if (consWebSocket && consWebSocket->removeConnection(conn))
            {
                consWebSocket = nullptr;
            }
        })
        .onmessage([]([[maybe_unused]] crow::websocket::Connection& conn,
                      const std::string& data, [[maybe_unused]] bool isBinary) {
            consWebSocket->inputBuffer += data;
            consWebSocket->doWrite();
        });
}
} // namespace obmc_console
} // namespace crow
