/*
// Copyright (c) 2019 Intel Corporation
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

#include "health.hpp"
#include "multi_storage_helper.hpp"
#include "openbmc_dbus_rest.hpp"

#include <app.hpp>
#include <dbus_utility.hpp>
#include <registries/privilege_registry.hpp>
#include <sdbusplus/asio/property.hpp>
#include <utils/location_utils.hpp>

#include <span>
#include <unordered_set>
#include <variant>

namespace redfish
{
inline void requestRoutesStorageCollection(App& app)
{
    BMCWEB_ROUTE(app, "/redfish/v1/Systems/system/Storage/")
        .privileges(redfish::privileges::getStorageCollection)
        .methods(boost::beast::http::verb::get)(
            [](const crow::Request&,
               const std::shared_ptr<bmcweb::AsyncResp>& asyncResp) {
                asyncResp->res.jsonValue["@odata.type"] =
                    "#StorageCollection.StorageCollection";
                asyncResp->res.jsonValue["@odata.id"] =
                    "/redfish/v1/Systems/system/Storage";
                asyncResp->res.jsonValue["Name"] = "Storage Collection";
                collection_util::getCollectionMembers(
                    asyncResp, "/redfish/v1/Systems/system/Storage",
                    {"xyz.openbmc_project.Inventory.Item.Storage"});
            });
}

inline void getDrives(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                      const std::shared_ptr<HealthPopulate>& health,
                      const std::string& parent, const std::string& storageId)
{
    crow::connections::systemBus->async_method_call(
        [asyncResp, health, parent,
         storageId](const boost::system::error_code ec,
                    const std::vector<std::string>& driveList) {
            if (ec)
            {
                BMCWEB_LOG_ERROR << "Drive mapper call error";
                messages::internalError(asyncResp->res);
                return;
            }

            nlohmann::json& driveArray = asyncResp->res.jsonValue["Drives"];
            driveArray = nlohmann::json::array();
            auto& count = asyncResp->res.jsonValue["Drives@odata.count"];
            count = 0;

            health->inventory.insert(health->inventory.end(), driveList.begin(),
                                     driveList.end());

            for (const std::string& drive : driveList)
            {
                if (!validSubpath(drive, parent))
                {
                    continue;
                }

                sdbusplus::message::object_path object(drive);
                if (object.filename().empty())
                {
                    BMCWEB_LOG_ERROR << "Failed to find filename in " << drive;
                    return;
                }
                driveArray.push_back(
                    {{"@odata.id", "/redfish/v1/Systems/system/Storage/" +
                                       storageId + "/Drives/" +
                                       object.filename()}});
            }

            count = driveArray.size();
        },
        "xyz.openbmc_project.ObjectMapper",
        "/xyz/openbmc_project/object_mapper",
        "xyz.openbmc_project.ObjectMapper", "GetSubTreePaths",
        "/xyz/openbmc_project/inventory", int32_t(0),
        std::array<const char*, 1>{"xyz.openbmc_project.Inventory.Item.Drive"});
}

inline void getStorageControllerLocation(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& path, const std::string& service,
    const std::vector<std::string>& interfaces, size_t index)
{
    nlohmann::json_pointer<nlohmann::json> locationPtr =
        "/StorageControllers"_json_pointer / index / "Location";
    for (const std::string& interface : interfaces)
    {
        if (interface == "xyz.openbmc_project.Inventory.Decorator.LocationCode")
        {
            location_util::getLocationCode(asyncResp, service, path,
                                           locationPtr);
        }
        if (location_util::isConnector(interface))
        {
            std::optional<std::string> locationType =
                location_util::getLocationType(interface);
            if (!locationType)
            {
                BMCWEB_LOG_DEBUG
                    << "getLocationType for StorageController failed for "
                    << interface;
                continue;
            }
            asyncResp->res
                .jsonValue[locationPtr]["PartLocation"]["LocationType"] =
                *locationType;
        }
    }
}

inline void
    findStorageControllers(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                           const crow::openbmc_mapper::GetSubTreeType& subtree,
                           const std::shared_ptr<HealthPopulate>& health,
                           const std::string& parent,
                           const std::string& storageId)
{
    sdbusplus::asio::getProperty<std::vector<std::string>>(
        *crow::connections::systemBus, "xyz.openbmc_project.ObjectMapper",
        parent + "/StorageController", "xyz.openbmc_project.Association",
        "endpoints",
        [asyncResp, storageId, subtree,
         health](const boost::system::error_code ec,
                 const std::vector<std::string>& storageControllerList) {
            if (ec)
            {
                return;
            }

            if (storageControllerList.empty())
            {
                BMCWEB_LOG_DEBUG << storageId << " has no storage controller";
                return;
            }

            std::unordered_map<std::string,
                               std::pair<std::string, std::vector<std::string>>>
                storageControllerServices;

            nlohmann::json& root =
                asyncResp->res.jsonValue["StorageControllers"];
            root = nlohmann::json::array();
            for (const auto& [path, interfaceDict] : subtree)
            {
                if (interfaceDict.size() != 1)
                {
                    BMCWEB_LOG_ERROR << "Connection size "
                                     << interfaceDict.size()
                                     << ", not equal to 1";
                    continue;
                }
                storageControllerServices.emplace(
                    path, std::make_pair(interfaceDict.front().first,
                                         interfaceDict.front().second));
            }

            for (const std::string& storageController : storageControllerList)
            {
                auto storageControllerService =
                    storageControllerServices.find(storageController);
                if (storageControllerService == storageControllerServices.end())
                {
                    continue;
                }
                sdbusplus::message::object_path path(storageController);
                const std::string& id = path.filename();
                if (id.empty())
                {
                    BMCWEB_LOG_ERROR << "filename() is empty in " << path.str;
                    continue;
                }

                size_t index = root.size();
                nlohmann::json& storageControllerJson =
                    root.emplace_back(nlohmann::json::object());
                storageControllerJson["@odata.type"] =
                    "#Storage.v1_7_0.StorageController";
                storageControllerJson["@odata.id"] =
                    "/redfish/v1/Systems/system/Storage/" + storageId +
                    "#/StorageControllers/" + std::to_string(index);
                storageControllerJson["Name"] = id;
                storageControllerJson["MemberId"] = id;
                storageControllerJson["Status"]["State"] = "Enabled";

                const std::string& service =
                    storageControllerService->second.first;
                getStorageControllerLocation(
                    asyncResp, storageController, service,
                    storageControllerService->second.second, index);

                sdbusplus::asio::getProperty<bool>(
                    *crow::connections::systemBus, service, path,
                    "xyz.openbmc_project.Inventory.Item", "Present",
                    [asyncResp, index](const boost::system::error_code ec2,
                                       bool enabled) {
                        // this interface isn't necessary, only check it
                        // if we get a good return
                        if (ec2)
                        {
                            return;
                        }
                        if (!enabled)
                        {
                            asyncResp->res.jsonValue["StorageControllers"]
                                                    [index]["Status"]["State"] =
                                "Disabled";
                        }
                    });

                crow::connections::systemBus->async_method_call(
                    [asyncResp,
                     index](const boost::system::error_code ec2,
                            const std::vector<std::pair<
                                std::string, dbus::utility::DbusVariantType>>&
                                propertiesList) {
                        if (ec2)
                        {
                            // this interface isn't necessary
                            return;
                        }
                        for (const std::pair<std::string,
                                             dbus::utility::DbusVariantType>&
                                 property : propertiesList)
                        {
                            // Store DBus properties that are also
                            // Redfish properties with same name and a
                            // string value
                            const std::string& propertyName = property.first;
                            nlohmann::json& object =
                                asyncResp->res
                                    .jsonValue["StorageControllers"][index];
                            if ((propertyName == "PartNumber") ||
                                (propertyName == "SerialNumber") ||
                                (propertyName == "Manufacturer") ||
                                (propertyName == "Model"))
                            {
                                const std::string* value =
                                    std::get_if<std::string>(&property.second);
                                if (value == nullptr)
                                {
                                    // illegal property
                                    messages::internalError(asyncResp->res);
                                    return;
                                }
                                object[propertyName] = *value;
                            }
                        }
                    },
                    service, storageController,
                    "org.freedesktop.DBus.Properties", "GetAll",
                    "xyz.openbmc_project.Inventory.Decorator.Asset");

                auto subHealth = std::make_shared<HealthPopulate>(
                    asyncResp, storageControllerJson["Status"]);
                subHealth->inventory.emplace_back(path);
                health->inventory.emplace_back(path);
                health->children.emplace_back(subHealth);
            }
        });
}

inline void
    getStorageControllers(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                          const std::shared_ptr<HealthPopulate>& health,
                          const std::string& parent,
                          const std::string& storageId)
{
    crow::connections::systemBus->async_method_call(
        [asyncResp, health, parent,
         storageId](const boost::system::error_code ec,
                    const crow::openbmc_mapper::GetSubTreeType& subtree) {
            if (ec || !subtree.size())
            {
                // doesn't have to be there
                return;
            }

            findStorageControllers(asyncResp, subtree, health, parent,
                                   storageId);
        },
        "xyz.openbmc_project.ObjectMapper",
        "/xyz/openbmc_project/object_mapper",
        "xyz.openbmc_project.ObjectMapper", "GetSubTree",
        "/xyz/openbmc_project/inventory", int32_t(0),
        std::array<const char*, 1>{
            "xyz.openbmc_project.Inventory.Item.StorageController"});
}

inline void
    getStorageLocation(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                       const std::string& path, const std::string& service,
                       const std::vector<std::string>& interfaces)
{
    for (const auto& interface : interfaces)
    {
        if (interface == "xyz.openbmc_project.Inventory.Decorator.LocationCode")
        {
            location_util::getLocationCode(asyncResp, service, path,
                                           "/PhysicalLocation"_json_pointer);
        }
        if (location_util::isConnector(interface))
        {
            std::optional<std::string> locationType =
                location_util::getLocationType(interface);
            if (!locationType)
            {
                BMCWEB_LOG_DEBUG << "getLocationType for Storage failed for "
                                 << interface;
                continue;
            }
            asyncResp->res
                .jsonValue["PhysicalLocation"]["PartLocation"]["LocationType"] =
                *locationType;
        }
    }
}

inline void requestRoutesStorage(App& app)
{
    BMCWEB_ROUTE(app, "/redfish/v1/Systems/system/Storage/<str>/")
        .privileges(redfish::privileges::getStorage)
        .methods(
            boost::beast::http::verb::get)([](const crow::Request&,
                                              const std::shared_ptr<
                                                  bmcweb::AsyncResp>& asyncResp,
                                              const std::string& storageId) {
            crow::connections::systemBus->async_method_call(
                [asyncResp, storageId](
                    const boost::system::error_code ec,
                    const crow::openbmc_mapper::GetSubTreeType& subtree) {
                    if (ec)
                    {
                        BMCWEB_LOG_DEBUG
                            << "requestRoutesStorage DBUS response error";
                        messages::resourceNotFound(asyncResp->res,
                                                   "#Storage.v1_7_1.Storage",
                                                   storageId);
                        return;
                    }

                    auto storage = std::find_if(
                        subtree.begin(), subtree.end(),
                        [&storageId](
                            const std::pair<
                                std::string,
                                std::vector<std::pair<
                                    std::string, std::vector<std::string>>>>&
                                object) {
                            return sdbusplus::message::object_path(object.first)
                                       .filename() == storageId;
                        });

                    if (storage == subtree.end())
                    {
                        messages::resourceNotFound(asyncResp->res,
                                                   "#Storage.v1_7_1.Storage",
                                                   storageId);
                        return;
                    }

                    const std::string& storagePath = storage->first;
                    const std::vector<
                        std::pair<std::string, std::vector<std::string>>>&
                        connectionNames = storage->second;

                    if (connectionNames.size() != 1)
                    {
                        BMCWEB_LOG_ERROR << "Connection size "
                                         << connectionNames.size()
                                         << ", greater than 1";
                        messages::resourceNotFound(asyncResp->res,
                                                   "#Storage.v1_7_1.Storage",
                                                   storageId);
                        return;
                    }

                    asyncResp->res.jsonValue["@odata.type"] =
                        "#Storage.v1_7_1.Storage";
                    asyncResp->res.jsonValue["@odata.id"] =
                        "/redfish/v1/Systems/system/Storage/" + storageId;
                    asyncResp->res.jsonValue["Name"] = "Storage";
                    asyncResp->res.jsonValue["Id"] = storageId;
                    asyncResp->res.jsonValue["Status"]["State"] = "Enabled";

                    auto health = std::make_shared<HealthPopulate>(asyncResp);
                    health->populate();

                    getDrives(asyncResp, health, storagePath, storageId);
                    getStorageControllers(asyncResp, health, storagePath,
                                          storageId);
                    getStorageLocation(asyncResp, connectionNames[0].first,
                                       storagePath, connectionNames[0].second);
                },
                "xyz.openbmc_project.ObjectMapper",
                "/xyz/openbmc_project/object_mapper",
                "xyz.openbmc_project.ObjectMapper", "GetSubTree",
                "/xyz/openbmc_project/inventory", 0,
                std::array<std::string, 1>{
                    "xyz.openbmc_project.Inventory.Item.Storage"});
        });
}

inline void getDriveAsset(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                          const std::string& connectionName,
                          const std::string& path)
{
    crow::connections::systemBus->async_method_call(
        [asyncResp](const boost::system::error_code ec,
                    const std::vector<
                        std::pair<std::string, dbus::utility::DbusVariantType>>&
                        propertiesList) {
            if (ec)
            {
                // this interface isn't necessary
                return;
            }
            for (const std::pair<std::string, dbus::utility::DbusVariantType>&
                     property : propertiesList)
            {
                // Store DBus properties that are also
                // Redfish properties with same name and a
                // string value
                const std::string& propertyName = property.first;
                if ((propertyName == "PartNumber") ||
                    (propertyName == "SerialNumber") ||
                    (propertyName == "Manufacturer") ||
                    (propertyName == "Model"))
                {
                    const std::string* value =
                        std::get_if<std::string>(&property.second);
                    if (value == nullptr)
                    {
                        // illegal property
                        messages::internalError(asyncResp->res);
                        return;
                    }
                    asyncResp->res.jsonValue[propertyName] = *value;
                }
            }
        },
        connectionName, path, "org.freedesktop.DBus.Properties", "GetAll",
        "xyz.openbmc_project.Inventory.Decorator.Asset");
}

inline void getDrivePresent(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                            const std::string& connectionName,
                            const std::string& path)
{
    sdbusplus::asio::getProperty<bool>(
        *crow::connections::systemBus, connectionName, path,
        "xyz.openbmc_project.Inventory.Item", "Present",
        [asyncResp, path](const boost::system::error_code ec,
                          const bool enabled) {
            // this interface isn't necessary, only check it if
            // we get a good return
            if (ec)
            {
                return;
            }

            if (!enabled)
            {
                asyncResp->res.jsonValue["Status"]["State"] = "Disabled";
            }
        });
}

inline void getDriveState(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                          const std::string& connectionName,
                          const std::string& path)
{
    sdbusplus::asio::getProperty<bool>(
        *crow::connections::systemBus, connectionName, path,
        "xyz.openbmc_project.State.Drive", "Rebuilding",
        [asyncResp](const boost::system::error_code ec, const bool updating) {
            // this interface isn't necessary, only check it
            // if we get a good return
            if (ec)
            {
                return;
            }

            // updating and disabled in the backend shouldn't be
            // able to be set at the same time, so we don't need
            // to check for the race condition of these two
            // calls
            if (updating)
            {
                asyncResp->res.jsonValue["Status"]["State"] = "Updating";
            }
        });
}

inline void
    getDriveLocation(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                     const std::string& path, const std::string& service,
                     const std::vector<std::string>& interfaces)
{
    for (const std::string& interface : interfaces)
    {
        if (interface == "xyz.openbmc_project.Inventory.Decorator.LocationCode")
        {
            location_util::getLocationCode(asyncResp, service, path,
                                           "/PhysicalLocation"_json_pointer);
        }
        if (location_util::isConnector(interface))
        {
            std::optional<std::string> locationType =
                location_util::getLocationType(interface);
            if (!locationType)
            {
                BMCWEB_LOG_DEBUG << "getLocationType for Drive failed for "
                                 << interface;
                continue;
            }
            asyncResp->res
                .jsonValue["PhysicalLocation"]["PartLocation"]["LocationType"] =
                *locationType;
        }
    }
}

inline void requestRoutesDrive(App& app)
{
    BMCWEB_ROUTE(app, "/redfish/v1/Systems/system/Storage/<str>/Drives/<str>/")
        .privileges(redfish::privileges::getDrive)
        .methods(
            boost::beast::http::verb::get)([](const crow::Request&,
                                              const std::shared_ptr<
                                                  bmcweb::AsyncResp>& asyncResp,
                                              const std::string& storageId,
                                              const std::string& driveId) {
            crow::connections::systemBus->async_method_call(
                [asyncResp, storageId,
                 driveId](const boost::system::error_code ec,
                          const crow::openbmc_mapper::GetSubTreeType& subtree) {
                    if (ec)
                    {
                        BMCWEB_LOG_ERROR << "Drive mapper call error";
                        messages::internalError(asyncResp->res);
                        return;
                    }

                    auto drive = std::find_if(
                        subtree.begin(), subtree.end(),
                        [&driveId](const std::pair<
                                   std::string,
                                   std::vector<std::pair<
                                       std::string, std::vector<std::string>>>>&
                                       object) {
                            return sdbusplus::message::object_path(object.first)
                                       .filename() == driveId;
                        });

                    if (drive == subtree.end())
                    {
                        messages::resourceNotFound(asyncResp->res, "Drive",
                                                   driveId);
                        return;
                    }

                    const std::string& path = drive->first;
                    const std::vector<
                        std::pair<std::string, std::vector<std::string>>>&
                        connectionNames = drive->second;

                    asyncResp->res.jsonValue["@odata.type"] =
                        "#Drive.v1_7_0.Drive";
                    asyncResp->res.jsonValue["@odata.id"] =
                        "/redfish/v1/Systems/system/Storage/" + storageId +
                        "/Drives/" + driveId;
                    asyncResp->res.jsonValue["Name"] = driveId;
                    asyncResp->res.jsonValue["Id"] = driveId;

                    if (connectionNames.size() != 1)
                    {
                        BMCWEB_LOG_ERROR << "Connection size "
                                         << connectionNames.size()
                                         << ", not equal to 1";
                        messages::internalError(asyncResp->res);
                        return;
                    }

                    // TODO(wltu): Connection to Main Chassis if connection to
                    // System.
                    // Use Association to link to Chassis otherwise.
                    getChassisId(
                        asyncResp, path,
                        [asyncResp](std::optional<std::string> chassisId) {
                            if (chassisId)
                            {
                                asyncResp->res.jsonValue["Links"]["Chassis"] = {
                                    {"@odata.id",
                                     "/redfish/v1/Chassis/" + *chassisId}};
                            }
                        });

                    // default it to Enabled
                    asyncResp->res.jsonValue["Status"]["State"] = "Enabled";

                    auto health = std::make_shared<HealthPopulate>(asyncResp);
                    health->inventory.emplace_back(path);
                    health->populate();

                    const std::string& connectionName =
                        connectionNames[0].first;

                    getDriveAsset(asyncResp, connectionName, path);
                    getDrivePresent(asyncResp, connectionName, path);
                    getDriveState(asyncResp, connectionName, path);
                    getDriveLocation(asyncResp, connectionName, path,
                                     connectionNames[0].second);

                    asyncResp->res.jsonValue["Actions"]["#Drive.Reset"] = {
                        {"target", "/redfish/v1/Systems/system/Storage/" +
                                       storageId + "/Drives/" + driveId +
                                       "/Actions/Drive.Reset"},
                        {"@Redfish.ActionInfo",
                         "/redfish/v1/Systems/system/Storage/" + storageId +
                             "/Drives/" + driveId + "/ResetActionInfo/"}};
                },
                "xyz.openbmc_project.ObjectMapper",
                "/xyz/openbmc_project/object_mapper",
                "xyz.openbmc_project.ObjectMapper", "GetSubTree",
                "/xyz/openbmc_project/inventory", int32_t(0),
                std::array<const char*, 1>{
                    "xyz.openbmc_project.Inventory.Item.Drive"});
        });
}

/**
 * Function for the Drive to perform actions.
 *
 * @param[in] asyncResp - Shared pointer for completing asynchronous calls
 * @param[in] driveId   - D-bus filename to identify the Drive
 * @param[in] resetType - Reset type for the Drive
 */
inline void doDriveAction(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                          const std::string& driveId,
                          std::optional<std::string> resetType)
{
    const char* interfaceName = "xyz.openbmc_project.State.Drive";

    std::string action;
    if (!resetType || *resetType == "PowerCycle")
    {
        action = "xyz.openbmc_project.State.Drive.Transition.Powercycle";
    }
    else if (*resetType == "ForceReset")
    {
        action = "xyz.openbmc_project.State.Drive.Transition.Reboot";
    }
    else
    {
        BMCWEB_LOG_DEBUG << "Invalid property value for ResetType: "
                         << *resetType;
        messages::actionParameterNotSupported(asyncResp->res, *resetType,
                                              "ResetType");
        return;
    }

    BMCWEB_LOG_DEBUG << "Reset Drive with " << action;
    crow::connections::systemBus->async_method_call(
        [asyncResp, driveId, action,
         interfaceName](const boost::system::error_code ec,
                        const crow::openbmc_mapper::GetSubTreeType& subtree) {
            if (ec)
            {
                BMCWEB_LOG_ERROR << "Drive mapper call error";
                messages::internalError(asyncResp->res);
                return;
            }

            auto driveState = std::find_if(
                subtree.begin(), subtree.end(), [&driveId](auto& object) {
                    const sdbusplus::message::object_path path(object.first);
                    return path.filename() == driveId;
                });

            if (driveState == subtree.end())
            {
                messages::resourceNotFound(asyncResp->res, "Drive Action",
                                           driveId);
                return;
            }

            const std::string& path = driveState->first;
            const std::vector<std::pair<std::string, std::vector<std::string>>>&
                connectionNames = driveState->second;

            if (connectionNames.size() != 1)
            {
                BMCWEB_LOG_ERROR << "Connection size " << connectionNames.size()
                                 << ", not equal to 1";
                messages::internalError(asyncResp->res);
                return;
            }

            const std::string& connectionName = connectionNames[0].first;
            const char* destProperty = "RequestedDriveTransition";
            std::variant<std::string> dbusPropertyValue(action);

            crow::connections::systemBus->async_method_call(
                [asyncResp, action](const boost::system::error_code ec) {
                    if (ec)
                    {
                        BMCWEB_LOG_ERROR << "[Set] Bad D-Bus request error for "
                                         << action << " : " << ec;
                        messages::internalError(asyncResp->res);
                        return;
                    }

                    messages::success(asyncResp->res);
                },
                connectionName.c_str(), path.c_str(),
                "org.freedesktop.DBus.Properties", "Set", interfaceName,
                destProperty, dbusPropertyValue);
        },
        "xyz.openbmc_project.ObjectMapper",
        "/xyz/openbmc_project/object_mapper",
        "xyz.openbmc_project.ObjectMapper", "GetSubTree",
        "/xyz/openbmc_project/inventory", int32_t(0),
        std::array<const char*, 1>{interfaceName});
}

/**
 * DriveResetAction class supports the POST method for the Reset (reboot)
 * action.
 */
inline void requestDriveResetAction(App& app)
{
    BMCWEB_ROUTE(app, "/redfish/v1/Systems/system/Storage/<str>/Drives/<str>/"
                      "Actions/Drive.Reset/")
        .privileges(redfish::privileges::postDrive)
        .methods(boost::beast::http::verb::post)(
            [](const crow::Request& req,
               const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
               const std::string&, const std::string& driveId) {
                BMCWEB_LOG_DEBUG << "Post Drive Reset.";

                nlohmann::json jsonRequest;
                std::optional<std::string> resetType;
                // TODO(wltu): Update after
                // https://gerrit.openbmc-project.xyz/c/openbmc/bmcweb/+/49697
                if (json_util::processJsonFromRequest(asyncResp->res, req,
                                                      jsonRequest) &&
                    !jsonRequest["ResetType"].empty())
                {
                    resetType = jsonRequest["ResetType"];
                }

                crow::connections::systemBus->async_method_call(
                    [asyncResp, driveId, resetType](
                        const boost::system::error_code ec,
                        const crow::openbmc_mapper::GetSubTreeType& subtree) {
                        if (ec)
                        {
                            BMCWEB_LOG_ERROR << "Drive mapper call error";
                            messages::internalError(asyncResp->res);
                            return;
                        }

                        auto drive = std::find_if(
                            subtree.begin(), subtree.end(),
                            [&driveId](
                                const std::pair<std::string,
                                                std::vector<std::pair<
                                                    std::string,
                                                    std::vector<std::string>>>>&
                                    object) {
                                return sdbusplus::message::object_path(
                                           object.first)
                                           .filename() == driveId;
                            });

                        if (drive == subtree.end())
                        {
                            messages::resourceNotFound(
                                asyncResp->res, "Drive Action Reset", driveId);
                            return;
                        }

                        doDriveAction(asyncResp, driveId, resetType);
                    },
                    "xyz.openbmc_project.ObjectMapper",
                    "/xyz/openbmc_project/object_mapper",
                    "xyz.openbmc_project.ObjectMapper", "GetSubTree",
                    "/xyz/openbmc_project/inventory", int32_t(0),
                    std::array<const char*, 1>{
                        "xyz.openbmc_project.Inventory.Item.Drive"});
            });
}

/**
 * DriveResetActionInfo derived class for delivering Drive
 * ResetType AllowableValues using ResetInfo schema.
 */
inline void requestRoutesDriveResetActionInfo(App& app)
{
    BMCWEB_ROUTE(app, "/redfish/v1/Systems/system/Storage/<str>/Drives/<str>/"
                      "ResetActionInfo/")
        .privileges(redfish::privileges::getActionInfo)
        .methods(
            boost::beast::http::verb::get)([](const crow::Request&,
                                              const std::shared_ptr<
                                                  bmcweb::AsyncResp>& asyncResp,
                                              const std::string& storageId,
                                              const std::string& driveId) {
            crow::connections::systemBus->async_method_call(
                [asyncResp, storageId,
                 driveId](const boost::system::error_code ec,
                          const crow::openbmc_mapper::GetSubTreeType& subtree) {
                    if (ec)
                    {
                        BMCWEB_LOG_ERROR << "Drive mapper call error";
                        messages::internalError(asyncResp->res);
                        return;
                    }

                    auto drive = std::find_if(
                        subtree.begin(), subtree.end(),
                        [&driveId](const std::pair<
                                   std::string,
                                   std::vector<std::pair<
                                       std::string, std::vector<std::string>>>>&
                                       object) {
                            return sdbusplus::message::object_path(object.first)
                                       .filename() == driveId;
                        });

                    if (drive == subtree.end())
                    {
                        messages::resourceNotFound(
                            asyncResp->res, "Drive ResetActionInfo", driveId);
                        return;
                    }

                    asyncResp->res.jsonValue = {
                        {"@odata.type", "#ActionInfo.v1_1_2.ActionInfo"},
                        {"@odata.id", "/redfish/v1/Systems/system/Storage/" +
                                          storageId + "/Drives/" + driveId +
                                          "/ResetActionInfo/"},
                        {"Name", "Reset Action Info"},
                        {"Id", "ResetActionInfo"},
                        {"Parameters",
                         {{{"Name", "ResetType"},
                           {"Required", true},
                           {"DataType", "String"},
                           {"AllowableValues",
                            {"PowerCycle", "ForceRestart"}}}}}};
                },
                "xyz.openbmc_project.ObjectMapper",
                "/xyz/openbmc_project/object_mapper",
                "xyz.openbmc_project.ObjectMapper", "GetSubTree",
                "/xyz/openbmc_project/inventory", int32_t(0),
                std::array<const char*, 1>{
                    "xyz.openbmc_project.Inventory.Item.Drive"});
        });
}

} // namespace redfish
