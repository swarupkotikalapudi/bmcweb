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
#include "query.hpp"
#include "registries/privilege_registry.hpp"
#include "utils/collection.hpp"
#include "utils/dbus_utils.hpp"
#include "utils/pcie_util.hpp"

#include <boost/system/linux_error.hpp>
#include <boost/url/format.hpp>
#include <sdbusplus/asio/property.hpp>
#include <sdbusplus/unpack_properties.hpp>

namespace redfish
{

static constexpr const char* inventoryPath = "/xyz/openbmc_project/inventory";
static constexpr std::array<std::string_view, 1> pcieDeviceInterface = {
    "xyz.openbmc_project.Inventory.Item.PCIeDevice"};

static inline void handlePCIeDevicePath(
    const std::string& pcieDeviceId,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const dbus::utility::MapperGetSubTreePathsResponse& pcieDevicePaths,
    const std::function<void(const std::string& pcieDevicePath,
                             const std::string& service)>& callback)

{
    for (const std::string& pcieDevicePath : pcieDevicePaths)
    {
        std::string pciecDeviceName =
            sdbusplus::message::object_path(pcieDevicePath).filename();
        if (pciecDeviceName.empty() || pciecDeviceName != pcieDeviceId)
        {
            continue;
        }

        dbus::utility::getDbusObject(
            pcieDevicePath, {},
            [pcieDevicePath, asyncResp,
             callback](const boost::system::error_code& ec,
                       const dbus::utility::MapperGetObject& object) {
            if (ec || object.empty())
            {
                BMCWEB_LOG_ERROR << "DBUS response error " << ec;
                messages::internalError(asyncResp->res);
                return;
            }
            callback(pcieDevicePath, object.begin()->first);
            });
        return;
    }

    BMCWEB_LOG_WARNING << "PCIe Device not found";
    messages::resourceNotFound(asyncResp->res, "PCIeDevice", pcieDeviceId);
}

static inline void getValidPCIeDevicePath(
    const std::string& pcieDeviceId,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::function<void(const std::string& pcieDevicePath,
                             const std::string& service)>& callback)
{
    dbus::utility::getSubTreePaths(
        inventoryPath, 0, pcieDeviceInterface,
        [pcieDeviceId, asyncResp,
         callback](const boost::system::error_code& ec,
                   const dbus::utility::MapperGetSubTreePathsResponse&
                       pcieDevicePaths) {
        if (ec)
        {
            BMCWEB_LOG_ERROR << "D-Bus response error on GetSubTree " << ec;
            messages::internalError(asyncResp->res);
            return;
        }
        handlePCIeDevicePath(pcieDeviceId, asyncResp, pcieDevicePaths,
                             callback);
        return;
        });
}

static inline void handlePCIeDeviceCollectionGet(
    crow::App& app, const crow::Request& req,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& systemName)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }
    if constexpr (bmcwebEnableMultiHost)
    {
        // Option currently returns no systems.  TBD
        messages::resourceNotFound(asyncResp->res, "ComputerSystem",
                                   systemName);
        return;
    }
    if (systemName != "system")
    {
        messages::resourceNotFound(asyncResp->res, "ComputerSystem",
                                   systemName);
        return;
    }

    asyncResp->res.addHeader(boost::beast::http::field::link,
                             "</redfish/v1/JsonSchemas/PCIeDeviceCollection/"
                             "PCIeDeviceCollection.json>; rel=describedby");
    asyncResp->res.jsonValue["@odata.type"] =
        "#PCIeDeviceCollection.PCIeDeviceCollection";
    asyncResp->res.jsonValue["@odata.id"] =
        "/redfish/v1/Systems/system/PCIeDevices";
    asyncResp->res.jsonValue["Name"] = "PCIe Device Collection";
    asyncResp->res.jsonValue["Description"] = "Collection of PCIe Devices";
    asyncResp->res.jsonValue["Members"] = nlohmann::json::array();
    asyncResp->res.jsonValue["Members@odata.count"] = 0;

    pcie_util::getPCIeDeviceList(asyncResp, "Members");
}

inline void requestRoutesSystemPCIeDeviceCollection(App& app)
{
    /**
     * Functions triggers appropriate requests on DBus
     */
    BMCWEB_ROUTE(app, "/redfish/v1/Systems/<str>/PCIeDevices/")
        .privileges(redfish::privileges::getPCIeDeviceCollection)
        .methods(boost::beast::http::verb::get)(
            std::bind_front(handlePCIeDeviceCollectionGet, std::ref(app)));
}

inline void
    getPCIeDeviceHealth(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                        const std::string& pcieDevicePath,
                        const std::string& service)
{
    sdbusplus::asio::getProperty<bool>(
        *crow::connections::systemBus, service, pcieDevicePath,
        "xyz.openbmc_project.State.Decorator.OperationalStatus", "Functional",
        [asyncResp](const boost::system::error_code& ec, const bool value) {
        if (ec)
        {
            if (ec.value() != EBADR)
            {
                BMCWEB_LOG_ERROR << "DBUS response error for Health "
                                 << ec.value();
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

inline void
    getPCIeDeviceState(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                       const std::string& pcieDevicePath,
                       const std::string& service)
{
    sdbusplus::asio::getProperty<bool>(
        *crow::connections::systemBus, service, pcieDevicePath,
        "xyz.openbmc_project.Inventory.Item", "Present",
        [asyncResp](const boost::system::error_code& ec, const bool value) {
        if (ec)
        {
            if (ec.value() != EBADR)
            {
                BMCWEB_LOG_ERROR << "DBUS response error for State";
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

inline void
    getPCIeDeviceAsset(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                       const std::string& pcieDevicePath,
                       const std::string& service)
{
    sdbusplus::asio::getAllProperties(
        *crow::connections::systemBus, service, pcieDevicePath,
        "xyz.openbmc_project.Inventory.Decorator.Asset",
        [pcieDevicePath, asyncResp{asyncResp}](
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

inline void addPCIeDeviceProperties(
    crow::Response& resp, const std::string& pcieDeviceId,
    const dbus::utility::DBusPropertiesMap& pcieDevProperties)
{
    const std::string* deviceType = nullptr;
    const std::string* generationInUse = nullptr;
    const int64_t* lanesInUse = nullptr;

    const bool success = sdbusplus::unpackPropertiesNoThrow(
        dbus_utils::UnpackErrorPrinter(), pcieDevProperties, "DeviceType",
        deviceType, "GenerationInUse", generationInUse, "LanesInUse",
        lanesInUse);

    if (!success)
    {
        messages::internalError(resp);
        return;
    }

    if (deviceType != nullptr && !deviceType->empty())
    {
        resp.jsonValue["PCIeInterface"]["DeviceType"] = *deviceType;
    }

    if (generationInUse != nullptr)
    {
        std::optional<pcie_device::PCIeTypes> redfishGenerationInUse =
            pcie_util::redfishPcieGenerationFromDbus(*generationInUse);

        if (!redfishGenerationInUse)
        {
            BMCWEB_LOG_WARNING << "Unknown PCIe Device Generation: "
                               << *generationInUse;
        }
        else
        {
            if (*redfishGenerationInUse == pcie_device::PCIeTypes::Invalid)
            {
                BMCWEB_LOG_ERROR << "Invalid PCIe Device Generation: "
                                 << *generationInUse;
                messages::internalError(resp);
                return;
            }
            resp.jsonValue["PCIeInterface"]["PCIeType"] =
                *redfishGenerationInUse;
        }
    }

    // The default value of LanesInUse is 0, and the field will be
    // left as off if it is a default value.
    if (lanesInUse != nullptr && *lanesInUse != 0)
    {
        resp.jsonValue["PCIeInterface"]["LanesInUse"] = *lanesInUse;
    }

    resp.jsonValue["PCIeFunctions"]["@odata.id"] = boost::urls::format(
        "/redfish/v1/Systems/system/PCIeDevices/{}/PCIeFunctions",
        pcieDeviceId);
}

inline void getPCIeDeviceProperties(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& pcieDevicePath, const std::string& service,
    const std::function<void(
        const dbus::utility::DBusPropertiesMap& pcieDevProperties)>&& callback)
{
    sdbusplus::asio::getAllProperties(
        *crow::connections::systemBus, service, pcieDevicePath,
        "xyz.openbmc_project.Inventory.Item.PCIeDevice",
        [asyncResp,
         callback](const boost::system::error_code& ec,
                   const dbus::utility::DBusPropertiesMap& pcieDevProperties) {
        if (ec)
        {
            if (ec.value() != EBADR)
            {
                BMCWEB_LOG_ERROR << "DBUS response error for Properties";
                messages::internalError(asyncResp->res);
            }
            return;
        }
        callback(pcieDevProperties);
        });
}

inline void addPCIeDeviceCommonProperties(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& pcieDeviceId)
{
    asyncResp->res.addHeader(
        boost::beast::http::field::link,
        "</redfish/v1/JsonSchemas/PCIeDevice/PCIeDevice.json>; rel=describedby");
    asyncResp->res.jsonValue["@odata.type"] = "#PCIeDevice.v1_9_0.PCIeDevice";
    asyncResp->res.jsonValue["@odata.id"] = boost::urls::format(
        "/redfish/v1/Systems/system/PCIeDevices/{}", pcieDeviceId);
    asyncResp->res.jsonValue["Name"] = "PCIe Device";
    asyncResp->res.jsonValue["Id"] = pcieDeviceId;
    asyncResp->res.jsonValue["Status"]["State"] = "Enabled";
    asyncResp->res.jsonValue["Status"]["Health"] = "OK";
}

inline void
    handlePCIeDeviceGet(App& app, const crow::Request& req,
                        const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                        const std::string& systemName,
                        const std::string& pcieDeviceId)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }
    if constexpr (bmcwebEnableMultiHost)
    {
        // Option currently returns no systems.  TBD
        messages::resourceNotFound(asyncResp->res, "ComputerSystem",
                                   systemName);
        return;
    }
    if (systemName != "system")
    {
        messages::resourceNotFound(asyncResp->res, "ComputerSystem",
                                   systemName);
        return;
    }

    getValidPCIeDevicePath(
        pcieDeviceId, asyncResp,
        [asyncResp, pcieDeviceId](const std::string& pcieDevicePath,
                                  const std::string& service) {
        addPCIeDeviceCommonProperties(asyncResp, pcieDeviceId);
        getPCIeDeviceAsset(asyncResp, pcieDevicePath, service);
        getPCIeDeviceState(asyncResp, pcieDevicePath, service);
        getPCIeDeviceHealth(asyncResp, pcieDevicePath, service);
        getPCIeDeviceProperties(
            asyncResp, pcieDevicePath, service,
            [asyncResp, pcieDeviceId](
                const dbus::utility::DBusPropertiesMap& pcieDevProperties) {
            addPCIeDeviceProperties(asyncResp->res, pcieDeviceId,
                                    pcieDevProperties);
            });
        });
}

inline void requestRoutesSystemPCIeDevice(App& app)
{
    BMCWEB_ROUTE(app, "/redfish/v1/Systems/<str>/PCIeDevices/<str>/")
        .privileges(redfish::privileges::getPCIeDevice)
        .methods(boost::beast::http::verb::get)(
            std::bind_front(handlePCIeDeviceGet, std::ref(app)));
}

inline void addPCIeFunctionList(
    crow::Response& res, const std::string& pcieDeviceId,
    const dbus::utility::DBusPropertiesMap& pcieDevProperties)
{
    nlohmann::json& pcieFunctionList = res.jsonValue["Members"];
    pcieFunctionList = nlohmann::json::array();
    static constexpr const int maxPciFunctionNum = 8;

    for (int functionNum = 0; functionNum < maxPciFunctionNum; functionNum++)
    {
        // Check if this function exists by
        // looking for a device ID
        std::string devIDProperty = "Function" + std::to_string(functionNum) +
                                    "DeviceId";
        const std::string* property = nullptr;
        for (const auto& propEntry : pcieDevProperties)
        {
            if (propEntry.first == devIDProperty)
            {
                property = std::get_if<std::string>(&propEntry.second);
                break;
            }
        }
        if (property == nullptr || property->empty())
        {
            continue;
        }

        nlohmann::json::object_t pcieFunction;
        pcieFunction["@odata.id"] = boost::urls::format(
            "/redfish/v1/Systems/system/PCIeDevices/{}/PCIeFunctions/{}",
            pcieDeviceId, std::to_string(functionNum));
        pcieFunctionList.emplace_back(std::move(pcieFunction));
    }
    res.jsonValue["PCIeFunctions@odata.count"] = pcieFunctionList.size();
}

inline void handlePCIeFunctionCollectionGet(
    App& app, const crow::Request& req,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& systemName, const std::string& pcieDeviceId)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }
    if constexpr (bmcwebEnableMultiHost)
    {
        // Option currently returns no systems.  TBD
        messages::resourceNotFound(asyncResp->res, "ComputerSystem",
                                   systemName);
        return;
    }

    getValidPCIeDevicePath(
        pcieDeviceId, asyncResp,
        [asyncResp, pcieDeviceId](const std::string& pcieDevicePath,
                                  const std::string& service) {
        asyncResp->res.addHeader(
            boost::beast::http::field::link,
            "</redfish/v1/JsonSchemas/PCIeFunctionCollection/PCIeFunctionCollection.json>; rel=describedby");
        asyncResp->res.jsonValue["@odata.type"] =
            "#PCIeFunctionCollection.PCIeFunctionCollection";
        asyncResp->res.jsonValue["@odata.id"] = boost::urls::format(
            "/redfish/v1/Systems/system/PCIeDevices/{}/PCIeFunctions",
            pcieDeviceId);
        asyncResp->res.jsonValue["Name"] = "PCIe Function Collection";
        asyncResp->res.jsonValue["Description"] =
            "Collection of PCIe Functions for PCIe Device " + pcieDeviceId;
        getPCIeDeviceProperties(
            asyncResp, pcieDevicePath, service,
            [asyncResp, pcieDeviceId](
                const dbus::utility::DBusPropertiesMap& pcieDevProperties) {
            addPCIeFunctionList(asyncResp->res, pcieDeviceId,
                                pcieDevProperties);
            });
        });
}

inline void requestRoutesSystemPCIeFunctionCollection(App& app)
{
    /**
     * Functions triggers appropriate requests on DBus
     */
    BMCWEB_ROUTE(app,
                 "/redfish/v1/Systems/<str>/PCIeDevices/<str>/PCIeFunctions/")
        .privileges(redfish::privileges::getPCIeFunctionCollection)
        .methods(boost::beast::http::verb::get)(
            std::bind_front(handlePCIeFunctionCollectionGet, std::ref(app)));
}

inline bool validatePCIeFunctionId(
    uint64_t pcieFunctionId,
    const dbus::utility::DBusPropertiesMap& pcieDevProperties)
{
    std::string functionName = "Function" + std::to_string(pcieFunctionId);
    std::string devIDProperty = functionName + "DeviceId";

    const std::string* devIdProperty = nullptr;
    for (const auto& property : pcieDevProperties)
    {
        if (property.first == devIDProperty)
        {
            devIdProperty = std::get_if<std::string>(&property.second);
            break;
        }
    }
    return (devIdProperty != nullptr && !devIdProperty->empty());
}

inline void addPCIeFunctionProperties(
    crow::Response& resp, uint64_t pcieFunctionId,
    const dbus::utility::DBusPropertiesMap& pcieDevProperties)
{
    std::string functionName = "Function" + std::to_string(pcieFunctionId);
    for (const auto& property : pcieDevProperties)
    {
        const std::string* strProperty =
            std::get_if<std::string>(&property.second);

        if (property.first == functionName + "DeviceId")
        {
            resp.jsonValue["DeviceId"] = *strProperty;
        }
        if (property.first == functionName + "VendorId")
        {
            resp.jsonValue["VendorId"] = *strProperty;
        }
        // TODO: FunctionType and DeviceClass are Redfish enums. The D-Bus
        // property strings should be mapped correctly to ensure these
        // strings are Redfish enum values. For now just check for empty.
        if (property.first == functionName + "FunctionType")
        {
            if (!strProperty->empty())
            {
                resp.jsonValue["FunctionType"] = *strProperty;
            }
        }
        if (property.first == functionName + "DeviceClass")
        {
            if (!strProperty->empty())
            {
                resp.jsonValue["DeviceClass"] = *strProperty;
            }
        }
        if (property.first == functionName + "ClassCode")
        {
            resp.jsonValue["ClassCode"] = *strProperty;
        }
        if (property.first == functionName + "RevisionId")
        {
            resp.jsonValue["RevisionId"] = *strProperty;
        }
        if (property.first == functionName + "SubsystemId")
        {
            resp.jsonValue["SubsystemId"] = *strProperty;
        }
        if (property.first == functionName + "SubsystemVendorId")
        {
            resp.jsonValue["SubsystemVendorId"] = *strProperty;
        }
    }
}

inline void addPCIeFunctionCommonProperties(crow::Response& resp,
                                            const std::string& pcieDeviceId,
                                            uint64_t pcieFunctionId)
{
    resp.addHeader(
        boost::beast::http::field::link,
        "</redfish/v1/JsonSchemas/PCIeFunction/PCIeFunction.json>; rel=describedby");
    resp.jsonValue["@odata.type"] = "#PCIeFunction.v1_2_3.PCIeFunction";
    resp.jsonValue["@odata.id"] = boost::urls::format(
        "/redfish/v1/Systems/system/PCIeDevices/{}/PCIeFunctions/{}",
        pcieDeviceId, std::to_string(pcieFunctionId));
    resp.jsonValue["Name"] = "PCIe Function";
    resp.jsonValue["Id"] = std::to_string(pcieFunctionId);
    resp.jsonValue["FunctionId"] = pcieFunctionId;
    resp.jsonValue["Links"]["PCIeDevice"]["@odata.id"] = boost::urls::format(
        "/redfish/v1/Systems/system/PCIeDevices/{}", pcieDeviceId);
}

inline void
    handlePCIeFunctionGet(App& app, const crow::Request& req,
                          const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                          const std::string& systemName,
                          const std::string& pcieDeviceId,
                          const std::string& pcieFunctionIdStr)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }
    if constexpr (bmcwebEnableMultiHost)
    {
        // Option currently returns no systems.  TBD
        messages::resourceNotFound(asyncResp->res, "ComputerSystem",
                                   systemName);
        return;
    }
    if (systemName != "system")
    {
        messages::resourceNotFound(asyncResp->res, "ComputerSystem",
                                   systemName);
        return;
    }

    uint64_t pcieFunctionId = 0;
    std::from_chars_result result = std::from_chars(
        &*pcieFunctionIdStr.begin(), &*pcieFunctionIdStr.end(), pcieFunctionId);
    if (result.ec != std::errc{} || result.ptr != &*pcieFunctionIdStr.end())
    {
        messages::resourceNotFound(asyncResp->res, "PCIeFunction",
                                   pcieFunctionIdStr);
        return;
    }

    getValidPCIeDevicePath(pcieDeviceId, asyncResp,
                           [asyncResp, pcieDeviceId,
                            pcieFunctionId](const std::string& pcieDevicePath,
                                            const std::string& service) {
        getPCIeDeviceProperties(
            asyncResp, pcieDevicePath, service,
            [asyncResp, pcieDeviceId, pcieFunctionId](
                const dbus::utility::DBusPropertiesMap& pcieDevProperties) {
            addPCIeFunctionCommonProperties(asyncResp->res, pcieDeviceId,
                                            pcieFunctionId);
            addPCIeFunctionProperties(asyncResp->res, pcieFunctionId,
                                      pcieDevProperties);
            });
    });
}

inline void requestRoutesSystemPCIeFunction(App& app)
{
    BMCWEB_ROUTE(
        app, "/redfish/v1/Systems/<str>/PCIeDevices/<str>/PCIeFunctions/<str>/")
        .privileges(redfish::privileges::getPCIeFunction)
        .methods(boost::beast::http::verb::get)(
            std::bind_front(handlePCIeFunctionGet, std::ref(app)));
}

} // namespace redfish
