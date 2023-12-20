#pragma once

#include "app.hpp"
#include "async_resp.hpp"
#include "http_request.hpp"
#include "privileges.hpp"
#include "query.hpp"
#include "registries/privilege_registry.hpp"
#include "routing.hpp"

#include <nlohmann/json.hpp>
#include <sdbusplus/asio/property.hpp>

#include <string>

namespace redfish
{

constexpr auto healthMonitorServiceName = "xyz.openbmc_project.HealthMon";
constexpr auto valueInterface = "xyz.openbmc_project.Metric.Value";
constexpr auto valueProperty = "Value";

inline void afterGetFreeStorageStatistics(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const boost::system::error_code& ec, double freeStorage)
{
    if (ec)
    {
        BMCWEB_LOG_ERROR("afterGetFreeStorageStatistics failed, Dbus error {}",
                         ec);
        return;
    }
    asyncResp->res.jsonValue["FreeStorageSpaceKiB"] = freeStorage / 1000;
}

inline void managerGetStorageStatistics(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    constexpr auto freeStorageObjPath =
        "/xyz/openbmc_project/metric/bmc/storage/rw";

    sdbusplus::asio::getProperty<double>(
        *crow::connections::systemBus, healthMonitorServiceName,
        freeStorageObjPath, valueInterface, valueProperty,
        std::bind_front(afterGetFreeStorageStatistics, asyncResp));
}

inline void afterGetProcessorKernelStatistics(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const boost::system::error_code& ec, double kernelCPU)
{
    if (ec)
    {
        BMCWEB_LOG_ERROR(
            "afterGetProcessorKernelStatistics failed, Dbus error {}", ec);
        return;
    }
    asyncResp->res.jsonValue["ProcessorStatistics"]["KernelPercent"] =
        kernelCPU;
}

inline void afterGetProcessorUserStatistics(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const boost::system::error_code& ec, double userCPU)
{
    if (ec)
    {
        BMCWEB_LOG_ERROR(
            "afterGetProcessorUserStatistics failed, Dbus error {}", ec);
        return;
    }
    asyncResp->res.jsonValue["ProcessorStatistics"]["UserPercent"] = userCPU;
}

inline void managerGetProcessorStatistics(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    constexpr auto kernelCPUObjPath =
        "/xyz/openbmc_project/metric/bmc/cpu/kernel";
    constexpr auto userCPUObjPath = "/xyz/openbmc_project/metric/bmc/cpu/user";

    sdbusplus::asio::getProperty<double>(
        *crow::connections::systemBus, healthMonitorServiceName,
        kernelCPUObjPath, valueInterface, valueProperty,
        std::bind_front(afterGetProcessorKernelStatistics, asyncResp));

    sdbusplus::asio::getProperty<double>(
        *crow::connections::systemBus, healthMonitorServiceName, userCPUObjPath,
        valueInterface, valueProperty,
        std::bind_front(afterGetProcessorUserStatistics, asyncResp));
}

inline void afterGetAvailableMemoryStatistics(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const boost::system::error_code& ec, double availableMemory)
{
    if (ec)
    {
        BMCWEB_LOG_ERROR(
            "afterGetAvailableMemoryStatistics failed, Dbus error {}", ec);
        return;
    }
    asyncResp->res.jsonValue["MemoryStatistics"]["AvailableBytes"] =
        availableMemory;
}

inline void afterGetBufferedAndCachedMemoryStatistics(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const boost::system::error_code& ec, double bufferedAndCachedMemory)
{
    if (ec)
    {
        BMCWEB_LOG_ERROR(
            "afterGetBufferedAndCachedMemoryStatistics failed, Dbus error {}",
            ec);
        return;
    }
    asyncResp->res.jsonValue["MemoryStatistics"]["BuffersAndCacheBytes"] =
        bufferedAndCachedMemory;
}

inline void afterGetFreeMemoryStatistics(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const boost::system::error_code& ec, double freeMemory)
{
    if (ec)
    {
        BMCWEB_LOG_ERROR("afterGetFreeMemoryStatistics failed, Dbus error {}",
                         ec);
        return;
    }
    asyncResp->res.jsonValue["MemoryStatistics"]["FreeBytes"] = freeMemory;
}

inline void afterGetSharedMemoryStatistics(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const boost::system::error_code& ec, double sharedMemory)
{
    if (ec)
    {
        BMCWEB_LOG_ERROR("afterGetSharedMemoryStatistics failed, Dbus error {}",
                         ec);
        return;
    }
    asyncResp->res.jsonValue["MemoryStatistics"]["SharedBytes"] = sharedMemory;
}

inline void afterGetTotalMemoryStatistics(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const boost::system::error_code& ec, double totalMemory)
{
    if (ec)
    {
        BMCWEB_LOG_ERROR("afterGetTotalMemoryStatistics failed, Dbus error {}",
                         ec);
        return;
    }
    asyncResp->res.jsonValue["MemoryStatistics"]["TotalBytes"] = totalMemory;
}

inline void managerGetMemoryStatistics(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    constexpr auto availableMemoryObjPath =
        "/xyz/openbmc_project/metric/bmc/memory/available";
    constexpr auto bufferedAndCachedMemoryObjPath =
        "/xyz/openbmc_project/metric/bmc/memory/buffered_and_cached";
    constexpr auto freeMemoryObjPath =
        "/xyz/openbmc_project/metric/bmc/memory/free";
    constexpr auto sharedMemoryObjPath =
        "/xyz/openbmc_project/metric/bmc/memory/shared";
    constexpr auto totalMemoryObjPath =
        "/xyz/openbmc_project/metric/bmc/memory/total";

    sdbusplus::asio::getProperty<double>(
        *crow::connections::systemBus, healthMonitorServiceName,
        availableMemoryObjPath, valueInterface, valueProperty,
        std::bind_front(afterGetAvailableMemoryStatistics, asyncResp));

    sdbusplus::asio::getProperty<double>(
        *crow::connections::systemBus, healthMonitorServiceName,
        bufferedAndCachedMemoryObjPath, valueInterface, valueProperty,
        std::bind_front(afterGetBufferedAndCachedMemoryStatistics, asyncResp));

    sdbusplus::asio::getProperty<double>(
        *crow::connections::systemBus, healthMonitorServiceName,
        freeMemoryObjPath, valueInterface, valueProperty,
        std::bind_front(afterGetFreeMemoryStatistics, asyncResp));

    sdbusplus::asio::getProperty<double>(
        *crow::connections::systemBus, healthMonitorServiceName,
        sharedMemoryObjPath, valueInterface, valueProperty,
        std::bind_front(afterGetSharedMemoryStatistics, asyncResp));

    sdbusplus::asio::getProperty<double>(
        *crow::connections::systemBus, healthMonitorServiceName,
        totalMemoryObjPath, valueInterface, valueProperty,
        std::bind_front(afterGetTotalMemoryStatistics, asyncResp));
}

inline void afterGetManagerStartTime(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const boost::system::error_code& ec, uint64_t bmcwebResetTime)
{
    if (ec)
    {
        // Not all servers will be running in systemd, so ignore the error.
        return;
    }
    using std::chrono::steady_clock;

    std::chrono::duration<steady_clock::rep, std::micro> usReset{
        bmcwebResetTime};
    steady_clock::time_point resetTime{usReset};

    steady_clock::time_point now = steady_clock::now();

    steady_clock::duration runTime = now - resetTime;

    if (runTime < steady_clock::duration::zero())
    {
        BMCWEB_LOG_CRITICAL("Uptime was negative????");
        messages::internalError(asyncResp->res);
        return;
    }

    // Floor to the closest millisecond
    using Milli = std::chrono::duration<steady_clock::rep, std::milli>;
    Milli milli = std::chrono::floor<Milli>(runTime);

    using SecondsFloat = std::chrono::duration<double>;
    SecondsFloat sec = std::chrono::duration_cast<SecondsFloat>(milli);

    asyncResp->res.jsonValue["ServiceRootUptimeSeconds"] = sec.count();
}

inline void managerGetServiceRootUptime(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    sdbusplus::asio::getProperty<uint64_t>(
        *crow::connections::systemBus, "org.freedesktop.systemd1",
        "/org/freedesktop/systemd1/unit/bmcweb_2eservice",
        "org.freedesktop.systemd1.Unit", "ActiveEnterTimestampMonotonic",
        std::bind_front(afterGetManagerStartTime, asyncResp));
}
/**
 * handleManagerDiagnosticData supports ManagerDiagnosticData.
 * It retrieves BMC health information from various DBus resources and returns
 * the information through the response.
 */
inline void handleManagerDiagnosticDataGet(
    crow::App& app, const crow::Request& req,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }
    asyncResp->res.jsonValue["@odata.type"] =
        "#ManagerDiagnosticData.v1_2_0.ManagerDiagnosticData";
    asyncResp->res.jsonValue["@odata.id"] =
        "/redfish/v1/Managers/bmc/ManagerDiagnosticData";
    asyncResp->res.jsonValue["Id"] = "ManagerDiagnosticData";
    asyncResp->res.jsonValue["Name"] = "Manager Diagnostic Data";

    managerGetServiceRootUptime(asyncResp);
    managerGetProcessorStatistics(asyncResp);
    managerGetMemoryStatistics(asyncResp);
    managerGetStorageStatistics(asyncResp);
}

inline void requestRoutesManagerDiagnosticData(App& app)
{
    BMCWEB_ROUTE(app, "/redfish/v1/Managers/bmc/ManagerDiagnosticData")
        .privileges(redfish::privileges::getManagerDiagnosticData)
        .methods(boost::beast::http::verb::get)(
            std::bind_front(handleManagerDiagnosticDataGet, std::ref(app)));
}

} // namespace redfish
