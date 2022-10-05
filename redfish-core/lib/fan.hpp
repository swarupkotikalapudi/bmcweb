#pragma once

#include "app.hpp"
#include "dbus_utility.hpp"
#include "error_messages.hpp"
#include "query.hpp"
#include "registries/privilege_registry.hpp"
#include "utils/chassis_utils.hpp"

#include <boost/system/error_code.hpp>
#include <boost/url/format.hpp>
#include <sdbusplus/asio/property.hpp>

#include <functional>
#include <memory>
#include <optional>
#include <string>

namespace redfish
{

inline void updateFanList(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                          const std::string& chassisId,
                          const std::string& fanPath)
{
    std::string fanName = sdbusplus::message::object_path(fanPath).filename();
    if (fanName.empty())
    {
        return;
    }

    nlohmann::json item;
    item["@odata.id"] = boost::urls::format(
        "/redfish/v1/Chassis/{}/ThermalSubsystem/Fans/{}", chassisId, fanName);

    nlohmann::json& fanList = asyncResp->res.jsonValue["Members"];
    fanList.emplace_back(std::move(item));
    asyncResp->res.jsonValue["Members@odata.count"] = fanList.size();
}

inline void doFanCollection(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                            const std::string& chassisId,
                            const std::optional<std::string>& validChassisPath)
{
    if (!validChassisPath)
    {
        messages::resourceNotFound(asyncResp->res, "Chassis", chassisId);
        return;
    }

    asyncResp->res.addHeader(
        boost::beast::http::field::link,
        "</redfish/v1/JsonSchemas/FanCollection/FanCollection.json>; rel=describedby");
    asyncResp->res.jsonValue["@odata.type"] = "#FanCollection.FanCollection";
    asyncResp->res.jsonValue["@odata.id"] = boost::urls::format(
        "/redfish/v1/Chassis/{}/ThermalSubsystem/Fans", chassisId);
    asyncResp->res.jsonValue["Name"] = "Fan Collection";
    asyncResp->res.jsonValue["Description"] =
        "The collection of Fan resource instances " + chassisId;
    asyncResp->res.jsonValue["Members"] = nlohmann::json::array();
    asyncResp->res.jsonValue["Members@odata.count"] = 0;

    dbus::utility::getAssociationEndPoints(
        *validChassisPath + "/cooled_by",
        [asyncResp,
         chassisId](const boost::system::error_code& ec,
                    const dbus::utility::MapperEndPoints& endpoints) {
        if (ec)
        {
            if (ec.value() != EBADR)
            {
                messages::internalError(asyncResp->res);
                return;
            }
            return;
        }

        for (const auto& endpoint : endpoints)
        {
            updateFanList(asyncResp, chassisId, endpoint);
        }
        });
}

inline void
    handleFanCollectionHead(App& app, const crow::Request& req,
                            const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                            const std::string& chassisId)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }

    redfish::chassis_utils::getValidChassisPath(
        asyncResp, chassisId,
        [asyncResp,
         chassisId](const std::optional<std::string>& validChassisPath) {
        if (!validChassisPath)
        {
            messages::resourceNotFound(asyncResp->res, "Chassis", chassisId);
            return;
        }
        asyncResp->res.addHeader(
            boost::beast::http::field::link,
            "</redfish/v1/JsonSchemas/FanCollection/FanCollection.json>; rel=describedby");
        });
}

inline void
    handleFanCollectionGet(App& app, const crow::Request& req,
                           const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                           const std::string& chassisId)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }

    redfish::chassis_utils::getValidChassisPath(
        asyncResp, chassisId,
        std::bind_front(doFanCollection, asyncResp, chassisId));
}

inline void requestRoutesFanCollection(App& app)
{
    BMCWEB_ROUTE(app, "/redfish/v1/Chassis/<str>/ThermalSubsystem/Fans/")
        .privileges(redfish::privileges::headFanCollection)
        .methods(boost::beast::http::verb::head)(
            std::bind_front(handleFanCollectionHead, std::ref(app)));

    BMCWEB_ROUTE(app, "/redfish/v1/Chassis/<str>/ThermalSubsystem/Fans/")
        .privileges(redfish::privileges::getFanCollection)
        .methods(boost::beast::http::verb::get)(
            std::bind_front(handleFanCollectionGet, std::ref(app)));
}

inline bool checkFanId(const std::string& fanPath, const std::string& fanId)
{
    std::string fanName = sdbusplus::message::object_path(fanPath).filename();

    return !(fanName.empty() || fanName != fanId);
}

static inline void handleFanPath(
    const std::string& fanId,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const dbus::utility::MapperGetSubTreePathsResponse& fanPaths,
    const std::function<void(const std::string& fanPath,
                             const std::string& service)>& callback)

{
    for (const std::string& fanPath : fanPaths)
    {
        std::string fanName =
            sdbusplus::message::object_path(fanPath).filename();
        if (!checkFanId(fanPath, fanId))
        {
            continue;
        }
        dbus::utility::getDbusObject(
            fanPath, {},
            [fanPath, asyncResp,
             callback](const boost::system::error_code& ec,
                       const dbus::utility::MapperGetObject& object) {
            if (ec || object.empty())
            {
                BMCWEB_LOG_ERROR << "DBUS response error on getDbusObject"
                                 << ec;
                messages::internalError(asyncResp->res);
                return;
            }
            callback(fanPath, object.begin()->first);
            });

        return;
    }
    BMCWEB_LOG_WARNING << "Fan not found";
    messages::resourceNotFound(asyncResp->res, "Fan", fanId);
}

inline void getValidFanPath(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& validChassisPath, const std::string& fanId,
    const std::function<void(const std::string& fanPath,
                             const std::string& service)>& callback)
{
    std::string endpointPath = validChassisPath + "/cooled_by";
    dbus::utility::getAssociationEndPoints(
        endpointPath, [asyncResp, fanId, callback](
                          const boost::system::error_code& ec,
                          const dbus::utility::MapperEndPoints& fanPaths) {
            if (ec)
            {
                if (ec.value() != EBADR)
                {
                    BMCWEB_LOG_ERROR
                        << "D-Bus response error on getAssociationEndPoints "
                        << ec;
                    messages::internalError(asyncResp->res);
                    return;
                }
                return;
            }
            handleFanPath(fanId, asyncResp, fanPaths, callback);
            return;
        });
}

inline void addFanCommonProperties(crow::Response& resp,
                                   const std::string& chassisId,
                                   const std::string& fanId)
{
    resp.addHeader(boost::beast::http::field::link,
                   "</redfish/v1/JsonSchemas/Fan/Fan.json>; rel=describedby");
    resp.jsonValue["@odata.type"] = "#Fan.v1_3_0.Fan";
    resp.jsonValue["Name"] = "Fan";
    resp.jsonValue["Id"] = fanId;
    resp.jsonValue["@odata.id"] = boost::urls::format(
        "/redfish/v1/Chassis/{}/ThermalSubsystem/Fans/{}", chassisId, fanId);
    resp.jsonValue["Status"]["State"] = "Enabled";
    resp.jsonValue["Status"]["Health"] = "OK";
}

inline void getFanHealth(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                         const std::string& fanPath, const std::string& service)
{
    sdbusplus::asio::getProperty<bool>(
        *crow::connections::systemBus, service, fanPath,
        "xyz.openbmc_project.State.Decorator.OperationalStatus", "Functional",
        [asyncResp](const boost::system::error_code& ec, const bool value) {
        if (ec)
        {
            if (ec.value() != EBADR)
            {
                BMCWEB_LOG_ERROR << "DBUS response error for Health " << ec;
                messages::internalError(asyncResp->res);
            }
            return;
        }

        if (!value)
        {
            asyncResp->res.jsonValue["Status"]["Health"] = "Critical";
        }
        });
}

inline void getFanState(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                        const std::string& fanPath, const std::string& service)
{
    sdbusplus::asio::getProperty<bool>(
        *crow::connections::systemBus, service, fanPath,
        "xyz.openbmc_project.Inventory.Item", "Present",
        [asyncResp](const boost::system::error_code& ec, const bool value) {
        if (ec)
        {
            if (ec.value() != EBADR)
            {
                BMCWEB_LOG_ERROR << "DBUS response error for State " << ec;
                messages::internalError(asyncResp->res);
            }
            return;
        }

        if (!value)
        {
            asyncResp->res.jsonValue["Status"]["State"] = "Absent";
        }
        });
}

inline void getFanAsset(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                        const std::string& fanPath, const std::string& service)
{
    sdbusplus::asio::getAllProperties(
        *crow::connections::systemBus, service, fanPath,
        "xyz.openbmc_project.Inventory.Decorator.Asset",
        [fanPath, asyncResp{asyncResp}](
            const boost::system::error_code& ec,
            const dbus::utility::DBusPropertiesMap& assetList) {
        if (ec)
        {
            if (ec.value() != EBADR)
            {
                BMCWEB_LOG_ERROR << "DBUS response error for Properties"
                                 << ec.value();
                messages::internalError(asyncResp->res);
            }
            return;
        }
        const std::string* manufacturer = nullptr;
        const std::string* model = nullptr;
        const std::string* partNumber = nullptr;
        const std::string* serialNumber = nullptr;
        const std::string* sparePartNumber = nullptr;

        const bool success = sdbusplus::unpackPropertiesNoThrow(
            dbus_utils::UnpackErrorPrinter(), assetList, "Manufacturer",
            manufacturer, "Model", model, "PartNumber", partNumber,
            "SerialNumber", serialNumber, "SparePartNumber", sparePartNumber);
        if (!success)
        {
            messages::internalError(asyncResp->res);
            return;
        }
        if (manufacturer != nullptr)
        {
            asyncResp->res.jsonValue["Manufacturer"] = *manufacturer;
        }
        if (model != nullptr)
        {
            asyncResp->res.jsonValue["Model"] = *model;
        }
        if (partNumber != nullptr)
        {
            asyncResp->res.jsonValue["PartNumber"] = *partNumber;
        }
        if (serialNumber != nullptr)
        {
            asyncResp->res.jsonValue["SerialNumber"] = *serialNumber;
        }
        if (sparePartNumber != nullptr && !sparePartNumber->empty())
        {
            asyncResp->res.jsonValue["SparePartNumber"] = *sparePartNumber;
        }
        });
}

inline void getFanLocation(const std::shared_ptr<bmcweb::AsyncResp>& aResp,
                           const std::string& fanPath,
                           const std::string& service)
{
    sdbusplus::asio::getProperty<std::string>(
        *crow::connections::systemBus, service, fanPath,
        "xyz.openbmc_project.Inventory.Decorator.LocationCode", "LocationCode",
        [aResp](const boost::system::error_code& ec,
                const std::string& property) {
        if (ec)
        {
            if (ec.value() != EBADR)
            {
                BMCWEB_LOG_ERROR << "DBUS response error for Location"
                                 << ec.value();
                messages::internalError(aResp->res);
            }
            return;
        }
        aResp->res.jsonValue["Location"]["PartLocation"]["ServiceLabel"] =
            property;
        });
}

inline void
    getFanSpeedPercent(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                       const std::string& service,
                       const std::string& sensorPath,
                       const std::string& chassisId, const std::string& name)
{
    sdbusplus::asio::getProperty<double>(
        *crow::connections::systemBus, service, sensorPath,
        "xyz.openbmc_project.Sensor.Value", "Value",
        [asyncResp, sensorPath, chassisId,
         name](const boost::system::error_code ec, const double value) {
        if (ec)
        {
            if (ec.value() != EBADR)
            {
                BMCWEB_LOG_DEBUG << "DBUS response error " << ec.message();
                messages::internalError(asyncResp->res);
            }
        }
        else
        {
            asyncResp->res.jsonValue[name]["Reading"] = value;
        }
        });

    sdbusplus::asio::getProperty<uint64_t>(
        *crow::connections::systemBus, service, sensorPath,
        "xyz.openbmc_project.Control.FanSpeed", "Target",
        [asyncResp, sensorPath, chassisId,
         name](const boost::system::error_code ec, const uint64_t speed) {
        if (ec)
        {
            if (ec.value() != EBADR)
            {
                BMCWEB_LOG_DEBUG << "DBUS response error " << ec.message();
                messages::internalError(asyncResp->res);
            }
        }
        else
        {
            asyncResp->res.jsonValue[name]["SpeedRPM"] = speed;
        }
        });

    std::string sensorId =
        sdbusplus::message::object_path(sensorPath).filename();
    asyncResp->res.jsonValue[name]["DataSourceUri"] = boost::urls::format(
        "/redfish/v1/Chassis/{}/Sensors/{}", chassisId, sensorId);
}

inline void addFanSensorsSpeedPercent(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& chassisId,
    const dbus::utility::MapperEndPoints& sensorPaths)
{
    dbus::utility::getDbusObject(
        sensorPaths[0], {},
        [sensorPaths, asyncResp,
         chassisId](const boost::system::error_code& ec1,
                    const dbus::utility::MapperGetObject& object) {
        if (ec1 || object.empty())
        {
            messages::internalError(asyncResp->res);
            return;
        }

        getFanSpeedPercent(asyncResp, object[0].first, sensorPaths[0],
                           chassisId, "SpeedPercent");

        // To Do: Uncomment this code when Redfish adds support for dual motors
        // and update the fan version.
        /* if (sensorPaths.size() == 2)
        {
            getFanSpeedPercent(asyncResp, object[0].first, sensorPaths[1],
                               chassisId, "SecondarySpeedPercent");
        } */
        });
}

inline void
    addFanSensorsProperties(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                            const std::string& fanPath,
                            const std::string& chassisId)
{
    std::string sensorPath = fanPath + "/sensors";
    dbus::utility::getAssociationEndPoints(
        sensorPath, [asyncResp, chassisId](
                        const boost::system::error_code& ec,
                        const dbus::utility::MapperEndPoints& sensorPaths) {
            if (ec)
            {
                if (ec.value() != EBADR)
                {
                    messages::internalError(asyncResp->res);
                    return;
                }
                return;
            }
            if (sensorPaths.empty())
            {
                return;
            }
            addFanSensorsSpeedPercent(asyncResp, chassisId, sensorPaths);
        });
}

inline void doFanGet(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                     const std::string& chassisId, const std::string& fanId,
                     const std::optional<std::string>& validChassisPath)
{
    if (!validChassisPath)
    {
        messages::resourceNotFound(asyncResp->res, "Chassis", chassisId);
        return;
    }

    getValidFanPath(asyncResp, *validChassisPath, fanId,
                    [asyncResp, chassisId, fanId](const std::string& fanPath,
                                                  const std::string& service) {
        addFanCommonProperties(asyncResp->res, chassisId, fanId);
        getFanState(asyncResp, fanPath, service);
        getFanHealth(asyncResp, fanPath, service);
        getFanAsset(asyncResp, fanPath, service);
        getFanLocation(asyncResp, fanPath, service);
        addFanSensorsProperties(asyncResp, fanPath, chassisId);
    });
}

inline void handleFanHead(App& app, const crow::Request& req,
                          const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                          const std::string& chassisId,
                          const std::string& fanId)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }

    redfish::chassis_utils::getValidChassisPath(
        asyncResp, chassisId,
        [asyncResp, chassisId,
         fanId](const std::optional<std::string>& validChassisPath) {
        if (!validChassisPath)
        {
            messages::resourceNotFound(asyncResp->res, "Fan", fanId);
            return;
        }
        getValidFanPath(asyncResp, *validChassisPath, fanId,
                        [asyncResp](const std::string&, const std::string&) {
            asyncResp->res.addHeader(
                boost::beast::http::field::link,
                "</redfish/v1/JsonSchemas/Fan/Fan.json>; rel=describedby");
        });
        });
}

inline void handleFanGet(App& app, const crow::Request& req,
                         const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                         const std::string& chassisId, const std::string& fanId)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }

    redfish::chassis_utils::getValidChassisPath(
        asyncResp, chassisId,
        std::bind_front(doFanGet, asyncResp, chassisId, fanId));
}

inline void requestRoutesFan(App& app)
{
    BMCWEB_ROUTE(app, "/redfish/v1/Chassis/<str>/ThermalSubsystem/Fans/<str>/")
        .privileges(redfish::privileges::headFan)
        .methods(boost::beast::http::verb::head)(
            std::bind_front(handleFanHead, std::ref(app)));

    BMCWEB_ROUTE(app, "/redfish/v1/Chassis/<str>/ThermalSubsystem/Fans/<str>/")
        .privileges(redfish::privileges::getFan)
        .methods(boost::beast::http::verb::get)(
            std::bind_front(handleFanGet, std::ref(app)));
}

} // namespace redfish
