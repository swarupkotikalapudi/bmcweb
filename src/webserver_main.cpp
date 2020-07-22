#include <app.h>
#include <systemd/sd-daemon.h>

#include <boost/asio/io_context.hpp>
#include <dbus_monitor.hpp>
#include <dbus_singleton.hpp>
#include <image_upload.hpp>
#include <kvm_websocket.hpp>
#include <login_routes.hpp>
#include <obmc_console.hpp>
#include <openbmc_dbus_rest.hpp>

#include <memory>
#ifdef BMCWEB_ENABLE_IBM_MANAGEMENT_CONSOLE
#include <event_dbus_monitor.hpp>
#include <ibm/management_console_rest.hpp>
#endif
#include <persistent_data_middleware.hpp>
#include <redfish.hpp>
#include <redfish_v1.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/server.hpp>
#include <security_headers_middleware.hpp>
#include <ssl_key_handler.hpp>
#include <vm_websocket.hpp>
#include <webassets.hpp>
#include <webserver_common.hpp>

#include <string>

#ifdef BMCWEB_ENABLE_VM_NBDPROXY
#include <nbd_proxy.hpp>
#endif

constexpr int defaultPort = 18080;

template <typename... Middlewares>
void setupSocket(crow::Crow<Middlewares...>& app)
{
    int listenFd = sd_listen_fds(0);
    if (1 == listenFd)
    {
        BMCWEB_LOG_INFO << "attempting systemd socket activation";
        if (sd_is_socket_inet(SD_LISTEN_FDS_START, AF_UNSPEC, SOCK_STREAM, 1,
                              0))
        {
            BMCWEB_LOG_INFO << "Starting webserver on socket handle "
                            << SD_LISTEN_FDS_START;
            app.socket(SD_LISTEN_FDS_START);
        }
        else
        {
            BMCWEB_LOG_INFO
                << "bad incoming socket, starting webserver on port "
                << defaultPort;
            app.port(defaultPort);
        }
    }
    else
    {
        BMCWEB_LOG_INFO << "Starting webserver on port " << defaultPort;
        app.port(defaultPort);
    }
}

int main(int argc, char** argv)
{
    crow::logger::setLogLevel(crow::LogLevel::Debug);

    auto io = std::make_shared<boost::asio::io_context>();
    CrowApp app(io);

    // Static assets need to be initialized before Authorization, because auth
    // needs to build the whitelist from the static routes

#ifdef BMCWEB_ENABLE_STATIC_HOSTING
    crow::webassets::requestRoutes(app);
#endif

#ifdef BMCWEB_ENABLE_KVM
    crow::obmc_kvm::requestRoutes(app);
#endif

#ifdef BMCWEB_ENABLE_REDFISH
    crow::redfish::requestRoutes(app);
#endif

#ifdef BMCWEB_ENABLE_DBUS_REST
    crow::dbus_monitor::requestRoutes(app);
    crow::image_upload::requestRoutes(app);
    crow::openbmc_mapper::requestRoutes(app);
#endif

#ifdef BMCWEB_ENABLE_HOST_SERIAL_WEBSOCKET
    crow::obmc_console::requestRoutes(app);
#endif

#ifdef BMCWEB_ENABLE_VM_WEBSOCKET
    crow::obmc_vm::requestRoutes(app);
#endif

#ifdef BMCWEB_ENABLE_IBM_MANAGEMENT_CONSOLE
    crow::ibm_mc::requestRoutes(app);
    crow::ibm_mc_lock::Lock::getInstance();
#endif

    crow::login_routes::requestRoutes(app);

    BMCWEB_LOG_INFO << "bmcweb (" << __DATE__ << ": " << __TIME__ << ')';
    setupSocket(app);

    crow::connections::systemBus =
        std::make_shared<sdbusplus::asio::connection>(*io);

#ifdef BMCWEB_ENABLE_VM_NBDPROXY
    crow::nbd_proxy::requestRoutes(app);
#endif

    redfish::RedfishService redfish(app);

#ifndef BMCWEB_ENABLE_REDFISH_DBUS_LOG_ENTRIES
    int rc = redfish::EventServiceManager::startEventLogMonitor(*io);
    if (rc)
    {
        BMCWEB_LOG_ERROR << "Redfish event handler setup failed...";
        return rc;
    }
#endif

#ifdef BMCWEB_ENABLE_IBM_MANAGEMENT_CONSOLE
    // Start BMC and Host state change dbus monitor
    crow::dbus_monitor::registerStateChangeSignal();
#endif

    app.run();
    io->run();

    crow::connections::systemBus.reset();
}
