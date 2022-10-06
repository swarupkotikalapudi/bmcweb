#pragma once

#include "app.hpp"
#include "dbus_utility.hpp"
#include "error_messages.hpp"
#include "query.hpp"
#include "registries/privilege_registry.hpp"
#include "str_utility.hpp"
#include "utils/chassis_utils.hpp"
#include "utils/fan_utils.hpp"

#include <boost/url/format.hpp>

#include <array>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

namespace redfish
{

inline void
    updateFanSensorList(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                        const std::string& chassisId,
                        const std::string& fanSensorPath, double value)
{
    std::string fanSensorName =
        sdbusplus::message::object_path(fanSensorPath).filename();
    if (fanSensorName.empty())
    {
        messages::internalError(asyncResp->res);
        return;
    }

    nlohmann::json::object_t item;
    item["SensorFanArrayExcerpt"]["DataSourceUri"] = boost::urls::format(
        "/redfish/v1/Chassis/{}/Sensors/{}", chassisId, fanSensorName);
    item["SensorFanArrayExcerpt"]["DeviceName"] = "Chassis #" + fanSensorName;
    item["SensorFanArrayExcerpt"]["SpeedRPM"] = value;

    nlohmann::json& fanSensorList =
        asyncResp->res.jsonValue["FanSpeedsPercent"];
    fanSensorList.emplace_back(std::move(item));
    std::sort(fanSensorList.begin(), fanSensorList.end(),
              [](const nlohmann::json& c1, const nlohmann::json& c2) {
        return c1["SensorFanArrayExcerpt"]["DataSourceUri"] <
               c2["SensorFanArrayExcerpt"]["DataSourceUri"];
    });
    asyncResp->res.jsonValue["FanSpeedsPercent@odata.count"] =
        fanSensorList.size();
}

inline void
    getFanSensorValue(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                      const std::string& chassisId,
                      const std::string& serviceName,
                      const std::string& fanSensorPath)
{
    if (serviceName.empty() || fanSensorPath.empty())
    {
        messages::internalError(asyncResp->res);
        return;
    }

    sdbusplus::asio::getProperty<double>(
        *crow::connections::systemBus, serviceName, fanSensorPath,
        "xyz.openbmc_project.Sensor.Value", "Value",
        [asyncResp, chassisId,
         fanSensorPath](const boost::system::error_code& ec, double value) {
        if (ec)
        {
            BMCWEB_LOG_ERROR("D-Bus response error for getFanSensorValue {}",
                             ec.value());
            messages::internalError(asyncResp->res);
            return;
        }

        updateFanSensorList(asyncResp, chassisId, fanSensorPath, value);
    });
}

inline void
    getFanSpeedsPercent(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                        const std::string& validChassisPath,
                        const std::string& chassisId)
{
    redfish::fan_utils::getFanPaths(
        asyncResp, validChassisPath,
        [asyncResp](
            const dbus::utility::MapperGetSubTreePathsResponse& fanPaths) {
        for (const auto& fanPath : fanPaths)
        {
            redfish::fan_utils::getFanSensorsObject(
                asyncResp, fanPath,
                std::bind_front(getFanSensorValue, asyncResp, chassisId));
        }
    });
}

inline void handleEnvironmentMetricsHead(
    App& app, const crow::Request& req,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& chassisId)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }

    auto respHandler = [asyncResp, chassisId](
                           const std::optional<std::string>& validChassisPath) {
        if (!validChassisPath)
        {
            messages::resourceNotFound(asyncResp->res, "Chassis", chassisId);
            return;
        }

        asyncResp->res.addHeader(
            boost::beast::http::field::link,
            "</redfish/v1/JsonSchemas/EnvironmentMetrics/EnvironmentMetrics.json>; rel=describedby");
    };

    redfish::chassis_utils::getValidChassisPath(asyncResp, chassisId,
                                                std::move(respHandler));
}

inline void handleEnvironmentMetricsGet(
    App& app, const crow::Request& req,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& chassisId)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }

    auto respHandler = [asyncResp, chassisId](
                           const std::optional<std::string>& validChassisPath) {
        if (!validChassisPath)
        {
            messages::resourceNotFound(asyncResp->res, "Chassis", chassisId);
            return;
        }

        asyncResp->res.addHeader(
            boost::beast::http::field::link,
            "</redfish/v1/JsonSchemas/EnvironmentMetrics/EnvironmentMetrics.json>; rel=describedby");
        asyncResp->res.jsonValue["@odata.type"] =
            "#EnvironmentMetrics.v1_3_0.EnvironmentMetrics";
        asyncResp->res.jsonValue["Name"] = "Chassis Environment Metrics";
        asyncResp->res.jsonValue["Id"] = "EnvironmentMetrics";
        asyncResp->res.jsonValue["@odata.id"] = boost::urls::format(
            "/redfish/v1/Chassis/{}/EnvironmentMetrics", chassisId);

        getFanSpeedsPercent(asyncResp, *validChassisPath, chassisId);
    };

    redfish::chassis_utils::getValidChassisPath(asyncResp, chassisId,
                                                std::move(respHandler));
}

inline void requestRoutesEnvironmentMetrics(App& app)
{
    BMCWEB_ROUTE(app, "/redfish/v1/Chassis/<str>/EnvironmentMetrics/")
        .privileges(redfish::privileges::headEnvironmentMetrics)
        .methods(boost::beast::http::verb::head)(
            std::bind_front(handleEnvironmentMetricsHead, std::ref(app)));

    BMCWEB_ROUTE(app, "/redfish/v1/Chassis/<str>/EnvironmentMetrics/")
        .privileges(redfish::privileges::getEnvironmentMetrics)
        .methods(boost::beast::http::verb::get)(
            std::bind_front(handleEnvironmentMetricsGet, std::ref(app)));
}

} // namespace redfish
