#pragma once

#include "app.hpp"
#include "dbus_utility.hpp"
#include "error_messages.hpp"
#include "query.hpp"
#include "registries/privilege_registry.hpp"
#include "utils/chassis_utils.hpp"

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
        return;
    }

    nlohmann::json item;
    item["DataSourceUri"] = boost::urls::format(
        "/redfish/v1/Chassis/{}/Sensors/{}", chassisId, fanSensorName);
    item["DeviceName"] = "Chassis Fan #" + fanSensorName;
    item["SpeedRPM"] = value;

    nlohmann::json& fanSensorList =
        asyncResp->res.jsonValue["FanSpeedsPercent"];
    fanSensorList.emplace_back(std::move(item));
    asyncResp->res.jsonValue["FanSpeedsPercent@odata.count"] =
        fanSensorList.size();
}

inline void getFanSensors(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                          const std::string& chassisId,
                          const std::string& service, const std::string& path)
{
    sdbusplus::asio::getProperty<double>(
        *crow::connections::systemBus, service, path,
        "xyz.openbmc_project.Sensor.Value", "Value",
        [asyncResp, chassisId, path](const boost::system::error_code ec,
                                     const double value) {
        if (ec)
        {
            BMCWEB_LOG_ERROR << "DBUS response error " << ec.message();
            messages::internalError(asyncResp->res);
            return;
        }

        updateFanSensorList(asyncResp, chassisId, path, value);
        });
}

inline void
    getFanSensorPaths(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                      const std::string& fanSensorPath,
                      const std::string& chassisId)
{
    constexpr std::array<std::string_view, 1> intefaces{
        "xyz.openbmc_project.Sensor.Value"};
    dbus::utility::getDbusObject(
        fanSensorPath, intefaces,
        [asyncResp, chassisId,
         fanSensorPath](const boost::system::error_code& ec,
                        const dbus::utility::MapperGetObject& object) {
        if (ec || object.size() != 1)
        {
            BMCWEB_LOG_ERROR << "DBUS response error " << ec.message();
            messages::internalError(asyncResp->res);
            return;
        }

        getFanSensors(asyncResp, chassisId, object.begin()->first,
                      fanSensorPath);
        });
}

inline void
    getFanSpeedsPercent(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                        const std::string& chassisPath,
                        const std::string& chassisId)
{
    dbus::utility::getAssociationEndPoints(
        chassisPath + "/cooled_by",
        [asyncResp,
         chassisId](const boost::system::error_code& ec,
                    const dbus::utility::MapperEndPoints& cooledEndpoints) {
        if (ec)
        {
            if (ec.value() != EBADR)
            {
                BMCWEB_LOG_ERROR << "D-Bus response error on Fan speed "
                                 << ec.message();
                messages::internalError(asyncResp->res);
            }
            return;
        }
        for (const auto& cooledEndpoint : cooledEndpoints)
        {
            dbus::utility::getAssociationEndPoints(
                cooledEndpoint + "/sensors",
                [asyncResp, chassisId](
                    const boost::system::error_code& ec1,
                    const dbus::utility::MapperEndPoints& sensorsEndpoints) {
                if (ec1)
                {
                    if (ec1.value() != EBADR)
                    {
                        BMCWEB_LOG_ERROR
                            << "D-Bus response error on assocation "
                            << ec1.message();
                        messages::internalError(asyncResp->res);
                    }
                    return;
                }

                for (const auto& sensorsEndpoint : sensorsEndpoints)
                {
                    getFanSensorPaths(asyncResp, sensorsEndpoint, chassisId);
                }
                });
        }
        });
}

inline void getPowerWatts(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                          const std::string& chassisId)
{
    constexpr std::array<std::string_view, 1> interfaces = {
        "xyz.openbmc_project.Sensor.Value"};
    dbus::utility::getSubTreePaths(
        "/xyz/openbmc_project/sensors", 0, interfaces,
        [asyncResp,
         chassisId](const boost::system::error_code& ec,
                    const dbus::utility::MapperGetSubTreePathsResponse& paths) {
        if (ec)
        {
            if (ec.value() != EBADR)
            {
                BMCWEB_LOG_ERROR << "DBUS response error for getSubTreePaths "
                                 << ec.value();
                messages::internalError(asyncResp->res);
            }
            return;
        }
        bool find = false;
        for (const auto& tempPath : paths)
        {
            sdbusplus::message::object_path path(tempPath);
            std::string leaf = path.filename();
            if (!leaf.compare("total_power"))
            {
                find = true;
                break;
            }
        }
        if (!find)
        {
            BMCWEB_LOG_DEBUG << "There is not total_power";
            return;
        }

        dbus::utility::getDbusObject(
            "/xyz/openbmc_project/sensors/power/total_power", {},
            [asyncResp,
             chassisId](const boost::system::error_code& ec1,
                        const dbus::utility::MapperGetObject& object) {
            if (ec1 || object.empty())
            {
                BMCWEB_LOG_ERROR << "DBUS response error: " << ec1.message();
                messages::internalError(asyncResp->res);
                return;
            }

            sdbusplus::asio::getProperty<double>(
                *crow::connections::systemBus, object.begin()->first,
                "/xyz/openbmc_project/sensors/power/total_power",
                "xyz.openbmc_project.Sensor.Value", "Value",
                [asyncResp, chassisId](const boost::system::error_code& ec2,
                                       double value) {
                if (ec2)
                {
                    if (ec2.value() != EBADR)
                    {
                        BMCWEB_LOG_ERROR << "Can't get Power Watts!"
                                         << ec2.message();
                        messages::internalError(asyncResp->res);
                    }
                    return;
                }
                asyncResp->res.jsonValue["PowerWatts"]["@odata.id"] =
                    boost::urls::format(
                        "/redfish/v1/Chassis/{}/Sensors/total_power",
                        chassisId);
                asyncResp->res.jsonValue["PowerWatts"]["DataSourceUri"] =
                    boost::urls::format(
                        "/redfish/v1/Chassis/{}/Sensors/total_power",
                        chassisId);
                asyncResp->res.jsonValue["PowerWatts"]["Reading"] = value;
                });
            });
        });
}

inline void
    setPowerSetPoint(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                     uint32_t powerCap)
{
    BMCWEB_LOG_DEBUG << "Set Power Limit Watts Set Point";

    sdbusplus::asio::setProperty(
        *crow::connections::systemBus, "xyz.openbmc_project.Settings",
        "/xyz/openbmc_project/control/host0/power_cap",
        "xyz.openbmc_project.Control.Power.Cap", "PowerCap", powerCap,
        [asyncResp](const boost::system::error_code& ec) {
        if (ec)
        {
            if (ec.value() != EBADR)
            {
                BMCWEB_LOG_ERROR << "Failed to set PowerCap " << ec.message();
                messages::internalError(asyncResp->res);
            }
            return;
        }
        });
}

inline void
    setPowerControlMode(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                        const std::string& controlMode)
{
    BMCWEB_LOG_DEBUG << "Set Power Limit Watts Control Mode";
    bool powerCapEnable = false;
    if (controlMode == "Disabled")
    {
        powerCapEnable = false;
    }
    else if (controlMode == "Automatic")
    {
        powerCapEnable = true;
    }
    else
    {
        BMCWEB_LOG_DEBUG << "Power Control Mode  does not support this mode :"
                         << controlMode;
        messages::propertyValueNotInList(asyncResp->res, controlMode,
                                         "ControlMode");
        return;
    }

    sdbusplus::asio::setProperty(
        *crow::connections::systemBus, "xyz.openbmc_project.Settings",
        "/xyz/openbmc_project/control/host0/power_cap",
        "xyz.openbmc_project.Control.Power.Cap", "PowerCapEnable",
        powerCapEnable, [asyncResp](const boost::system::error_code& ec) {
            if (ec)
            {
                if (ec.value() != EBADR)
                {
                    BMCWEB_LOG_ERROR << "Failed to set PowerCapEnable "
                                     << ec.message();
                    messages::internalError(asyncResp->res);
                }
                return;
            }
        });
}

inline void
    getPowerLimitWatts(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    sdbusplus::asio::getAllProperties(
        *crow::connections::systemBus, "xyz.openbmc_project.Settings",
        "/xyz/openbmc_project/control/host0/power_cap",
        "xyz.openbmc_project.Control.Power.Cap",
        [asyncResp](const boost::system::error_code& ec,
                    const dbus::utility::DBusPropertiesMap& propertiesList) {
        if (ec)
        {
            if (ec.value() != EBADR)
            {
                messages::internalError(asyncResp->res);
            }
            return;
        }

        asyncResp->res.jsonValue["PowerLimitWatts"]["SetPoint"] = 0;
        asyncResp->res.jsonValue["PowerLimitWatts"]["ControlMode"] =
            "Automatic";

        const uint32_t* powerCap = nullptr;
        const bool* powerCapEnable = nullptr;
        const uint32_t* minCap = nullptr;
        const uint32_t* maxCap = nullptr;

        const bool success = sdbusplus::unpackPropertiesNoThrow(
            dbus_utils::UnpackErrorPrinter(), propertiesList, "PowerCap",
            powerCap, "PowerCapEnable", powerCapEnable, "MinPowerCapValue",
            minCap, "MaxPowerCapValue", maxCap);

        if (!success)
        {
            messages::internalError(asyncResp->res);
            return;
        }

        if (powerCap != nullptr)
        {
            asyncResp->res.jsonValue["PowerLimitWatts"]["SetPoint"] = *powerCap;
        }

        if (powerCapEnable != nullptr && !*powerCapEnable)
        {
            asyncResp->res.jsonValue["PowerLimitWatts"]["ControlMode"] =
                "Disabled";
        }

        if (minCap != nullptr)
        {
            asyncResp->res.jsonValue["PowerLimitWatts"]["AllowableMin"] =
                *minCap;
        }

        if (maxCap != nullptr)
        {
            asyncResp->res.jsonValue["PowerLimitWatts"]["AllowableMax"] =
                *maxCap;
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
        getPowerWatts(asyncResp, chassisId);
        getPowerLimitWatts(asyncResp);
    };

    redfish::chassis_utils::getValidChassisPath(asyncResp, chassisId,
                                                std::move(respHandler));
}

inline void handleEnvironmentMetricsPatch(
    App& app, const crow::Request& req,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& chassisId)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }

    std::optional<nlohmann::json> powerLimitWatts;
    if (!json_util::readJsonPatch(req, asyncResp->res, "PowerLimitWatts",
                                  powerLimitWatts))
    {
        return;
    }

    if (!powerLimitWatts)
    {
        return;
    }

    std::optional<uint32_t> setPoint;
    std::optional<std::string> controlMode;
    if (!json_util::readJson(*powerLimitWatts, asyncResp->res, "SetPoint",
                             setPoint, "ControlMode", controlMode))
    {
        return;
    }

    redfish::chassis_utils::getValidChassisPath(
        asyncResp, chassisId,
        [asyncResp, chassisId, setPoint,
         controlMode](const std::optional<std::string>& validChassisPath) {
        if (!validChassisPath)
        {
            messages::resourceNotFound(asyncResp->res, "Chassis", chassisId);
            return;
        }

        if (setPoint)
        {
            setPowerSetPoint(asyncResp, *setPoint);
        }

        if (controlMode)
        {
            setPowerControlMode(asyncResp, *controlMode);
        }
        });
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

    BMCWEB_ROUTE(app, "/redfish/v1/Chassis/<str>/EnvironmentMetrics/")
        .privileges(redfish::privileges::patchEnvironmentMetrics)
        .methods(boost::beast::http::verb::patch)(
            std::bind_front(handleEnvironmentMetricsPatch, std::ref(app)));
}

} // namespace redfish
