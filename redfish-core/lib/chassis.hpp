/*
// Copyright (c) 2018 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
*/
#pragma once

#include "app.hpp"
#include "dbus_utility.hpp"
#include "health.hpp"
#include "led.hpp"
#include "query.hpp"
#include "registries/privilege_registry.hpp"
#include "utils/collection.hpp"
#include "utils/dbus_utils.hpp"
#include "utils/json_utils.hpp"

#include <boost/system/error_code.hpp>
#include <boost/url/format.hpp>
#include <sdbusplus/asio/property.hpp>
#include <sdbusplus/unpack_properties.hpp>

#include <array>
#include <string_view>

namespace redfish
{

/**
 * @brief Retrieves chassis state properties over dbus
 *
 * @param[in] aResp - Shared pointer for completing asynchronous calls.
 * @param[in] chassisPtr - Json pointer for the Chassis.
 *
 * @return None.
 */
inline void getChassisState(std::shared_ptr<bmcweb::AsyncResp> aResp,
                            const nlohmann::json::json_pointer& chassisPtr)
{
    // crow::connections::systemBus->async_method_call(
    sdbusplus::asio::getProperty<std::string>(
        *crow::connections::systemBus, "xyz.openbmc_project.State.Chassis",
        "/xyz/openbmc_project/state/chassis0",
        "xyz.openbmc_project.State.Chassis", "CurrentPowerState",
        [aResp{std::move(aResp)},
         chassisPtr](const boost::system::error_code& ec,
                     const std::string& chassisState) {
        if (ec)
        {
            if (ec == boost::system::errc::host_unreachable)
            {
                // Service not available, no error, just don't return
                // chassis state info
                BMCWEB_LOG_DEBUG << "Service not available " << ec;
                return;
            }
            BMCWEB_LOG_DEBUG << "DBUS response error " << ec;
            messages::internalError(aResp->res);
            return;
        }

        BMCWEB_LOG_DEBUG << "Chassis state: " << chassisState;
        // Verify Chassis State
        if (chassisState == "xyz.openbmc_project.State.Chassis.PowerState.On")
        {
            aResp->res.jsonValue[chassisPtr]["PowerState"] = "On";
            aResp->res.jsonValue[chassisPtr]["Status"]["State"] = "Enabled";
        }
        else if (chassisState ==
                 "xyz.openbmc_project.State.Chassis.PowerState.Off")
        {
            aResp->res.jsonValue[chassisPtr]["PowerState"] = "Off";
            aResp->res.jsonValue[chassisPtr]["Status"]["State"] =
                "StandbyOffline";
        }
        });
}

inline void
    getIntrusionByService(std::shared_ptr<bmcweb::AsyncResp> aResp,
                          const nlohmann::json::json_pointer& chassisPtr,
                          const std::string& service,
                          const std::string& objPath)
{
    BMCWEB_LOG_DEBUG << "Get intrusion status by service \n";

    sdbusplus::asio::getProperty<std::string>(
        *crow::connections::systemBus, service, objPath,
        "xyz.openbmc_project.Chassis.Intrusion", "Status",
        [aResp{std::move(aResp)}, chassisPtr](
            const boost::system::error_code& ec, const std::string& value) {
        if (ec)
        {
            // do not add err msg in redfish response, because this is not
            //     mandatory property
            BMCWEB_LOG_ERROR << "DBUS response error " << ec << "\n";
            return;
        }

        aResp->res.jsonValue[chassisPtr]["PhysicalSecurity"]
                            ["IntrusionSensorNumber"] = 1;
        aResp->res.jsonValue[chassisPtr]["PhysicalSecurity"]
                            ["IntrusionSensor"] = value;
        });
}

/**
 * Retrieves physical security properties over dbus
 */
inline void
    getPhysicalSecurityData(std::shared_ptr<bmcweb::AsyncResp> aResp,
                            const nlohmann::json::json_pointer& chassisPtr)
{
    constexpr std::array<std::string_view, 1> interfaces = {
        "xyz.openbmc_project.Chassis.Intrusion"};
    dbus::utility::getSubTree(
        "/xyz/openbmc_project/Intrusion", 1, interfaces,
        [aResp{std::move(aResp)}],
        chassisPtr(const boost::system::error_code& ec,
                   const dbus::utility::MapperGetSubTreeResponse& subtree) {
        if (ec)
        {
            // do not add err msg in redfish response, because this is not
            //     mandatory property
            BMCWEB_LOG_INFO << "DBUS error: no matched iface " << ec << "\n";
            return;
        }
        // Iterate over all retrieved ObjectPaths.
        for (const auto& object : subtree)
        {
            if (!object.second.empty())
            {
                const auto service = object.second.front();
                getIntrusionByService(aResp, chassisPtr, service.first,
                                      object.first);
                return;
            }
        }
        });
}

inline void
    getChassisLocationCode(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                           const nlohmann::json::json_pointer& chassisPtr,
                           const std::string& connectionName,
                           const std::string& path)
{
    sdbusplus::asio::getProperty<std::string>(
        *crow::connections::systemBus, connectionName, path,
        "xyz.openbmc_project.Inventory.Decorator.LocationCode", "LocationCode",
        [asyncResp, chassisPtr](const boost::system::error_code& ec,
                                const std::string& property) {
        if (ec)
        {
            BMCWEB_LOG_DEBUG << "DBUS response error for Location";
            messages::internalError(asyncResp->res);
            return;
        }

        asyncResp->res.jsonValue[chassisPtr]["Location"]["PartLocation"]
                                ["ServiceLabel"] = property;
        });
}

inline void getChassisUUID(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                           const nlohmann::json::json_pointer& chassisPtr,
                           const std::string& connectionName,
                           const std::string& path)
{
    sdbusplus::asio::getProperty<std::string>(
        *crow::connections::systemBus, connectionName, path,
        "xyz.openbmc_project.Common.UUID", "UUID",
        [asyncResp, chassisPtr](const boost::system::error_code& ec,
                                const std::string& chassisUUID) {
        if (ec)
        {
            BMCWEB_LOG_DEBUG << "DBUS response error for UUID";
            messages::internalError(asyncResp->res);
            return;
        }
        asyncResp->res.jsonValue[chassisPtr]["UUID"] = chassisUUID;
        });
}

inline void getChassisData(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                           const std::optional<std::string>& maybeChassisId)
{
    constexpr std::array<std::string_view, 2> interfaces = {
        "xyz.openbmc_project.Inventory.Item.Board",
        "xyz.openbmc_project.Inventory.Item.Chassis"};

    if (maybeChassisId != std::nullopt)
    {
        getPhysicalSecurityData(asyncResp, ""_json_pointer);
    }
    else
    {
        asyncResp->res.jsonValue["Members"] = nlohmann::json::array_t();
    }

    dbus::utility::getSubTree(
        "/xyz/openbmc_project/inventory", 0, interfaces,
        [asyncResp, maybeChassisId](
            const boost::system::error_code& ec,
            const dbus::utility::MapperGetSubTreeResponse& subtree) {
        if (ec)
        {
            messages::internalError(asyncResp->res);
            return;
        }
        // Iterate over all retrieved ObjectPaths.
        nlohmann::json::json_pointer chassisPtr;
        size_t index = 0;
        dbus::utility::ManagedObjectType statuses;
        std::string globalInventoryPath = "-";
        for (const std::pair<
                 std::string,
                 std::vector<std::pair<std::string, std::vector<std::string>>>>&
                 object : subtree)
        {
            const std::string& path = object.first;
            const std::vector<std::pair<std::string, std::vector<std::string>>>&
                connectionNames = object.second;

            sdbusplus::message::object_path objPath(path);
            if (maybeChassisId != std::nullopt &&
                objPath.filename() != maybeChassisId.value())
            {
                continue;
            }
            if (connectionNames.empty())
            {
                BMCWEB_LOG_ERROR << "Got 0 Connection names";
                continue;
            }

            std::string chassisId = objPath.filename();
            auto health = std::make_shared<HealthPopulate>(asyncResp);

            dbus::utility::getAssociationEndPoints(
                path + "/all_sensors",
                [health](const boost::system::error_code& ec2,
                         const dbus::utility::MapperEndPoints& resp) {
                if (ec2)
                {
                    return; // no sensors = no failures
                }
                health->inventory = resp;
                });
            if (maybeChassisId == std::nullopt)
            {
                if (index == 0)
                {
                    health->populate();
                    statuses = health->statuses;
                    globalInventoryPath = health->globalInventoryPath;
                }
                else
                {
                    health->populated = true;
                    health->statuses = statuses;
                    health->globalInventoryPath = globalInventoryPath;
                }
                chassisPtr = "/Members"_json_pointer / index++;
                health->jsonPtr = chassisPtr;
                getPhysicalSecurityData(asyncResp, chassisPtr);
            }
            else
            {
                health->populate();
            }

            asyncResp->res.jsonValue[chassisPtr] = nlohmann::json::object_t();
            nlohmann::json& chassis = asyncResp->res.jsonValue[chassisPtr];
            chassis["@odata.type"] = "#Chassis.v1_22_0.Chassis";
            chassis["@odata.id"] = boost::urls::format("/redfish/v1/Chassis/{}",
                                                       chassisId);
            chassis["Name"] = "Chassis Collection";
            chassis["ChassisType"] = "RackMount";
            chassis["Actions"]["#Chassis.Reset"]["target"] =
                boost::urls::format(
                    "/redfish/v1/Chassis/{}/Actions/Chassis.Reset", chassisId);
            chassis["Actions"]["#Chassis.Reset"]["@Redfish.ActionInfo"] =
                boost::urls::format("/redfish/v1/Chassis/{}/ResetActionInfo",
                                    chassisId);
            chassis["PCIeDevices"]["@odata.id"] =
                "/redfish/v1/Systems/system/PCIeDevices";

            dbus::utility::getAssociationEndPoints(
                path + "/drive",
                [asyncResp, chassisPtr,
                 chassisId](const boost::system::error_code& ec3,
                            const dbus::utility::MapperEndPoints& resp) {
                if (ec3 || resp.empty())
                {
                    return; // no drives = no failures
                }

                nlohmann::json reference;
                reference["@odata.id"] = boost::urls::format(
                    "/redfish/v1/Chassis/{}/Drives", chassisId);
                asyncResp->res.jsonValue[chassisPtr]["Drives"] =
                    std::move(reference);
                });

            const std::string& connectionName = connectionNames[0].first;

            const std::vector<std::string>& interfaces2 =
                connectionNames[0].second;
            const std::array<const char*, 2> hasIndicatorLed = {
                "xyz.openbmc_project.Inventory.Item.Panel",
                "xyz.openbmc_project.Inventory.Item.Board.Motherboard"};

            const std::string assetTagInterface =
                "xyz.openbmc_project.Inventory.Decorator.AssetTag";
            const std::string replaceableInterface =
                "xyz.openbmc_project.Inventory.Decorator.Replaceable";
            for (const auto& interface : interfaces2)
            {
                if (interface == assetTagInterface)
                {
                    sdbusplus::asio::getProperty<std::string>(
                        *crow::connections::systemBus, connectionName, path,
                        assetTagInterface, "AssetTag",
                        [asyncResp, chassisPtr,
                         chassisId](const boost::system::error_code& ec2,
                                    const std::string& property) {
                        if (ec2)
                        {
                            BMCWEB_LOG_ERROR
                                << "DBus response error for AssetTag: " << ec2;
                            messages::internalError(asyncResp->res);
                            return;
                        }
                        asyncResp->res.jsonValue[chassisPtr]["AssetTag"] =
                            property;
                        });
                }
                else if (interface == replaceableInterface)
                {
                    sdbusplus::asio::getProperty<bool>(
                        *crow::connections::systemBus, connectionName, path,
                        replaceableInterface, "HotPluggable",
                        [asyncResp, chassisPtr,
                         chassisId](const boost::system::error_code& ec2,
                                    const bool property) {
                        if (ec2)
                        {
                            BMCWEB_LOG_ERROR
                                << "DBus response error for HotPluggable: "
                                << ec2;
                            messages::internalError(asyncResp->res);
                            return;
                        }
                        asyncResp->res.jsonValue[chassisPtr]["HotPluggable"] =
                            property;
                        });
                }
            }

            for (const char* interface : hasIndicatorLed)
            {
                if (std::find(interfaces2.begin(), interfaces2.end(),
                              interface) != interfaces2.end())
                {
                    getIndicatorLedState(asyncResp, chassisPtr);
                    getLocationIndicatorActive(asyncResp, chassisPtr);
                    break;
                }
            }

            sdbusplus::asio::getAllProperties(
                *crow::connections::systemBus, connectionName, path,
                "xyz.openbmc_project.Inventory.Decorator.Asset",
                [asyncResp, chassisPtr, chassisId(std::string(chassisId))](
                    const boost::system::error_code& /*ec2*/,
                    const dbus::utility::DBusPropertiesMap& propertiesList) {
                const std::string* partNumber = nullptr;
                const std::string* serialNumber = nullptr;
                const std::string* manufacturer = nullptr;
                const std::string* model = nullptr;
                const std::string* sparePartNumber = nullptr;

                const bool success = sdbusplus::unpackPropertiesNoThrow(
                    dbus_utils::UnpackErrorPrinter(), propertiesList,
                    "PartNumber", partNumber, "SerialNumber", serialNumber,
                    "Manufacturer", manufacturer, "Model", model,
                    "SparePartNumber", sparePartNumber);

                if (!success)
                {
                    messages::internalError(asyncResp->res);
                    return;
                }

                nlohmann::json& chassisJson =
                    asyncResp->res.jsonValue[chassisPtr];

                if (partNumber != nullptr)
                {
                    chassisJson["PartNumber"] = *partNumber;
                }

                if (serialNumber != nullptr)
                {
                    chassisJson["SerialNumber"] = *serialNumber;
                }

                if (manufacturer != nullptr)
                {
                    chassisJson["Manufacturer"] = *manufacturer;
                }

                if (model != nullptr)
                {
                    chassisJson["Model"] = *model;
                }

                // SparePartNumber is optional on D-Bus
                // so skip if it is empty
                if (sparePartNumber != nullptr && !sparePartNumber->empty())
                {
                    chassisJson["SparePartNumber"] = *sparePartNumber;
                }

                chassisJson["Name"] = chassisId;
                chassisJson["Id"] = chassisId;
#ifdef BMCWEB_ALLOW_DEPRECATED_POWER_THERMAL
                chassisJson["Thermal"]["@odata.id"] = boost::urls::format(
                    "/redfish/v1/Chassis/{}/Thermal", chassisId);
                // Power object
                chassisJson["Power"]["@odata.id"] = boost::urls::format(
                    "/redfish/v1/Chassis/{}/Power", chassisId);
#endif
#ifdef BMCWEB_NEW_POWERSUBSYSTEM_THERMALSUBSYSTEM
                chassisJson["ThermalSubsystem"]["@odata.id"] =
                    boost::urls::format(
                        "/redfish/v1/Chassis/{}/ThermalSubsystem", chassisId);
                chassisJson["PowerSubsystem"]["@odata.id"] =
                    boost::urls::format("/redfish/v1/Chassis/{}/PowerSubsystem",
                                        chassisId);
                chassisJson["EnvironmentMetrics"]["@odata.id"] =
                    boost::urls::format(
                        "/redfish/v1/Chassis/{}/EnvironmentMetrics", chassisId);
#endif
                // SensorCollection
                chassisJson["Sensors"]["@odata.id"] = boost::urls::format(
                    "/redfish/v1/Chassis/{}/Sensors", chassisId);
                chassisJson["Status"]["State"] = "Enabled";

                nlohmann::json::array_t computerSystems;
                nlohmann::json::object_t system;
                system["@odata.id"] = "/redfish/v1/Systems/system";
                computerSystems.emplace_back(std::move(system));
                chassisJson["Links"]["ComputerSystems"] =
                    std::move(computerSystems);

                nlohmann::json::array_t managedBy;
                nlohmann::json::object_t manager;
                manager["@odata.id"] = "/redfish/v1/Managers/bmc";
                managedBy.emplace_back(std::move(manager));
                chassisJson["Links"]["ManagedBy"] = std::move(managedBy);
                getChassisState(asyncResp, chassisPtr);
                });

            for (const auto& interface : interfaces2)
            {
                if (interface == "xyz.openbmc_project.Common.UUID")
                {
                    getChassisUUID(asyncResp, chassisPtr, connectionName, path);
                }
                else if (interface ==
                         "xyz.openbmc_project.Inventory.Decorator.LocationCode")
                {
                    getChassisLocationCode(asyncResp, chassisPtr,
                                           connectionName, path);
                }
            }

            // Only getting target chassis.
            if (maybeChassisId != std::nullopt)
            {
                return;
            }
        }

        if (maybeChassisId != std::nullopt)
        {
            // Couldn't find an object with that name.  return an error
            messages::resourceNotFound(asyncResp->res, "Chassis",
                                       maybeChassisId.value());
            return;
        }
        asyncResp->res.jsonValue["Members@odata.count"] = index;
        });
}

inline void handleChassisCollectionGet(
    App& app, const crow::Request& req,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    query_param::Query delegated;
    query_param::QueryCapabilities capabilities = {
        .canDelegateExpandLevel = 1,
    };
    if (!redfish::setUpRedfishRouteWithDelegation(app, req, asyncResp,
                                                  delegated, capabilities))
    {
        return;
    }

    asyncResp->res.jsonValue["@odata.type"] =
        "#ChassisCollection.ChassisCollection";
    asyncResp->res.jsonValue["@odata.id"] = "/redfish/v1/Chassis";
    asyncResp->res.jsonValue["Name"] = "Chassis Collection";

    if (delegated.expandLevel > 0 &&
        delegated.expandType != query_param::ExpandType::None)
    {
        BMCWEB_LOG_DEBUG << "Use efficient expand handler";
        getChassisData(asyncResp, std::nullopt);
    }
    else
    {
        BMCWEB_LOG_DEBUG << "Use default expand handler";
        constexpr std::array<std::string_view, 2> interfaces{
            "xyz.openbmc_project.Inventory.Item.Board",
            "xyz.openbmc_project.Inventory.Item.Chassis"};
        collection_util::getCollectionMembers(
            asyncResp, boost::urls::url("/redfish/v1/Chassis"), interfaces);
    }
}

/**
 * ChassisCollection derived class for delivering Chassis Collection Schema
 *  Functions triggers appropriate requests on DBus
 */
inline void requestRoutesChassisCollection(App& app)
{
    BMCWEB_ROUTE(app, "/redfish/v1/Chassis/")
        .privileges(redfish::privileges::getChassisCollection)
        .methods(boost::beast::http::verb::get)(
            std::bind_front(handleChassisCollectionGet, std::ref(app)));
}

inline void
    handleChassisGet(App& app, const crow::Request& req,
                     const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                     const std::string& chassisId)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }
    getChassisData(asyncResp, chassisId);
}

inline void
    handleChassisPatch(App& app, const crow::Request& req,
                       const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                       const std::string& param)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }
    std::optional<bool> locationIndicatorActive;
    std::optional<std::string> indicatorLed;

    if (param.empty())
    {
        return;
    }

    if (!json_util::readJsonPatch(
            req, asyncResp->res, "LocationIndicatorActive",
            locationIndicatorActive, "IndicatorLED", indicatorLed))
    {
        return;
    }

    // TODO (Gunnar): Remove IndicatorLED after enough time has passed
    if (!locationIndicatorActive && !indicatorLed)
    {
        return; // delete this when we support more patch properties
    }
    if (indicatorLed)
    {
        asyncResp->res.addHeader(
            boost::beast::http::field::warning,
            "299 - \"IndicatorLED is deprecated. Use LocationIndicatorActive instead.\"");
    }

    constexpr std::array<std::string_view, 2> interfaces = {
        "xyz.openbmc_project.Inventory.Item.Board",
        "xyz.openbmc_project.Inventory.Item.Chassis"};

    const std::string& chassisId = param;

    dbus::utility::getSubTree(
        "/xyz/openbmc_project/inventory", 0, interfaces,
        [asyncResp, chassisId, locationIndicatorActive,
         indicatorLed](const boost::system::error_code& ec,
                       const dbus::utility::MapperGetSubTreeResponse& subtree) {
        if (ec)
        {
            messages::internalError(asyncResp->res);
            return;
        }

        // Iterate over all retrieved ObjectPaths.
        for (const std::pair<
                 std::string,
                 std::vector<std::pair<std::string, std::vector<std::string>>>>&
                 object : subtree)
        {
            const std::string& path = object.first;
            const std::vector<std::pair<std::string, std::vector<std::string>>>&
                connectionNames = object.second;

            sdbusplus::message::object_path objPath(path);
            if (objPath.filename() != chassisId)
            {
                continue;
            }

            if (connectionNames.empty())
            {
                BMCWEB_LOG_ERROR << "Got 0 Connection names";
                continue;
            }

            const std::vector<std::string>& interfaces3 =
                connectionNames[0].second;

            const std::array<const char*, 2> hasIndicatorLed = {
                "xyz.openbmc_project.Inventory.Item.Panel",
                "xyz.openbmc_project.Inventory.Item.Board.Motherboard"};
            bool indicatorChassis = false;
            for (const char* interface : hasIndicatorLed)
            {
                if (std::find(interfaces3.begin(), interfaces3.end(),
                              interface) != interfaces3.end())
                {
                    indicatorChassis = true;
                    break;
                }
            }
            if (locationIndicatorActive)
            {
                if (indicatorChassis)
                {
                    setLocationIndicatorActive(asyncResp,
                                               *locationIndicatorActive);
                }
                else
                {
                    messages::propertyUnknown(asyncResp->res,
                                              "LocationIndicatorActive");
                }
            }
            if (indicatorLed)
            {
                if (indicatorChassis)
                {
                    setIndicatorLedState(asyncResp, *indicatorLed);
                }
                else
                {
                    messages::propertyUnknown(asyncResp->res, "IndicatorLED");
                }
            }
            return;
        }

        messages::resourceNotFound(asyncResp->res, "Chassis", chassisId);
        });
}

/**
 * Chassis override class for delivering Chassis Schema
 * Functions triggers appropriate requests on DBus
 */
inline void requestRoutesChassis(App& app)
{
    BMCWEB_ROUTE(app, "/redfish/v1/Chassis/<str>/")
        .privileges(redfish::privileges::getChassis)
        .methods(boost::beast::http::verb::get)(
            std::bind_front(handleChassisGet, std::ref(app)));

    BMCWEB_ROUTE(app, "/redfish/v1/Chassis/<str>/")
        .privileges(redfish::privileges::patchChassis)
        .methods(boost::beast::http::verb::patch)(
            std::bind_front(handleChassisPatch, std::ref(app)));
}

inline void
    doChassisPowerCycle(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    constexpr std::array<std::string_view, 1> interfaces = {
        "xyz.openbmc_project.State.Chassis"};

    // Use mapper to get subtree paths.
    dbus::utility::getSubTreePaths(
        "/", 0, interfaces,
        [asyncResp](
            const boost::system::error_code& ec,
            const dbus::utility::MapperGetSubTreePathsResponse& chassisList) {
        if (ec)
        {
            BMCWEB_LOG_DEBUG << "[mapper] Bad D-Bus request error: " << ec;
            messages::internalError(asyncResp->res);
            return;
        }

        const char* processName = "xyz.openbmc_project.State.Chassis";
        const char* interfaceName = "xyz.openbmc_project.State.Chassis";
        const char* destProperty = "RequestedPowerTransition";
        const std::string propertyValue =
            "xyz.openbmc_project.State.Chassis.Transition.PowerCycle";
        std::string objectPath = "/xyz/openbmc_project/state/chassis_system0";

        /* Look for system reset chassis path */
        if ((std::find(chassisList.begin(), chassisList.end(), objectPath)) ==
            chassisList.end())
        {
            /* We prefer to reset the full chassis_system, but if it doesn't
             * exist on some platforms, fall back to a host-only power reset
             */
            objectPath = "/xyz/openbmc_project/state/chassis0";
        }

        crow::connections::systemBus->async_method_call(
            [asyncResp](const boost::system::error_code& ec2) {
            // Use "Set" method to set the property value.
            if (ec2)
            {
                BMCWEB_LOG_DEBUG << "[Set] Bad D-Bus request error: " << ec2;
                messages::internalError(asyncResp->res);
                return;
            }

            messages::success(asyncResp->res);
            },
            processName, objectPath, "org.freedesktop.DBus.Properties", "Set",
            interfaceName, destProperty,
            dbus::utility::DbusVariantType{propertyValue});
        });
}

inline void handleChassisResetActionInfoPost(
    App& app, const crow::Request& req,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& /*chassisId*/)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }
    BMCWEB_LOG_DEBUG << "Post Chassis Reset.";

    std::string resetType;

    if (!json_util::readJsonAction(req, asyncResp->res, "ResetType", resetType))
    {
        return;
    }

    if (resetType != "PowerCycle")
    {
        BMCWEB_LOG_DEBUG << "Invalid property value for ResetType: "
                         << resetType;
        messages::actionParameterNotSupported(asyncResp->res, resetType,
                                              "ResetType");

        return;
    }
    doChassisPowerCycle(asyncResp);
}

/**
 * ChassisResetAction class supports the POST method for the Reset
 * action.
 * Function handles POST method request.
 * Analyzes POST body before sending Reset request data to D-Bus.
 */

inline void requestRoutesChassisResetAction(App& app)
{
    BMCWEB_ROUTE(app, "/redfish/v1/Chassis/<str>/Actions/Chassis.Reset/")
        .privileges(redfish::privileges::postChassis)
        .methods(boost::beast::http::verb::post)(
            std::bind_front(handleChassisResetActionInfoPost, std::ref(app)));
}

inline void handleChassisResetActionInfoGet(
    App& app, const crow::Request& req,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& chassisId)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }
    asyncResp->res.jsonValue["@odata.type"] = "#ActionInfo.v1_1_2.ActionInfo";
    asyncResp->res.jsonValue["@odata.id"] = boost::urls::format(
        "/redfish/v1/Chassis/{}/ResetActionInfo", chassisId);
    asyncResp->res.jsonValue["Name"] = "Reset Action Info";

    asyncResp->res.jsonValue["Id"] = "ResetActionInfo";
    nlohmann::json::array_t parameters;
    nlohmann::json::object_t parameter;
    parameter["Name"] = "ResetType";
    parameter["Required"] = true;
    parameter["DataType"] = "String";
    nlohmann::json::array_t allowed;
    allowed.emplace_back("PowerCycle");
    parameter["AllowableValues"] = std::move(allowed);
    parameters.emplace_back(std::move(parameter));

    asyncResp->res.jsonValue["Parameters"] = std::move(parameters);
}

/**
 * ChassisResetActionInfo derived class for delivering Chassis
 * ResetType AllowableValues using ResetInfo schema.
 */
inline void requestRoutesChassisResetActionInfo(App& app)
{
    BMCWEB_ROUTE(app, "/redfish/v1/Chassis/<str>/ResetActionInfo/")
        .privileges(redfish::privileges::getActionInfo)
        .methods(boost::beast::http::verb::get)(
            std::bind_front(handleChassisResetActionInfoGet, std::ref(app)));
}

} // namespace redfish
