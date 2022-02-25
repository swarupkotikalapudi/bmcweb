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

#include <app.hpp>
#include <boost/system/linux_error.hpp>
#include <dbus_utility.hpp>
#include <registries/privilege_registry.hpp>
#include <sdbusplus/asio/property.hpp>
#include <utils/dbus_utils.hpp>

namespace redfish
{

static constexpr char const* pcieService = "xyz.openbmc_project.PCIe";
static constexpr char const* pciePath = "/xyz/openbmc_project/PCIe";
static constexpr char const* pcieDeviceInterface =
    "xyz.openbmc_project.PCIe.Device";

static inline void
    getPCIeDeviceList(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                      const std::string& name)
{
    auto getPCIeMapCallback =
        [asyncResp, name](const boost::system::error_code ec,
                          const dbus::utility::MapperGetSubTreePathsResponse&
                              pcieDevicePaths) {
            if (ec)
            {
                BMCWEB_LOG_DEBUG << "no PCIe device paths found ec: "
                                 << ec.message();
                // Not an error, system just doesn't have PCIe info
                return;
            }
            nlohmann::json& pcieDeviceList = asyncResp->res.jsonValue[name];
            pcieDeviceList = nlohmann::json::array();
            for (const std::string& pcieDevicePath : pcieDevicePaths)
            {
                size_t devStart = pcieDevicePath.rfind('/');
                if (devStart == std::string::npos)
                {
                    continue;
                }

                std::string devName = pcieDevicePath.substr(devStart + 1);
                if (devName.empty())
                {
                    continue;
                }
                pcieDeviceList.push_back(
                    {{"@odata.id",
                      "/redfish/v1/Systems/system/PCIeDevices/" + devName}});
            }
            asyncResp->res.jsonValue[name + "@odata.count"] =
                pcieDeviceList.size();
        };
    crow::connections::systemBus->async_method_call(
        std::move(getPCIeMapCallback), "xyz.openbmc_project.ObjectMapper",
        "/xyz/openbmc_project/object_mapper",
        "xyz.openbmc_project.ObjectMapper", "GetSubTreePaths",
        std::string(pciePath) + "/", 1, std::array<std::string, 0>());
}

inline void requestRoutesSystemPCIeDeviceCollection(App& app)
{
    /**
     * Functions triggers appropriate requests on DBus
     */
    BMCWEB_ROUTE(app, "/redfish/v1/Systems/system/PCIeDevices/")
        .privileges(redfish::privileges::getPCIeDeviceCollection)
        .methods(boost::beast::http::verb::get)(
            [](const crow::Request&,
               const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)

            {
                asyncResp->res.jsonValue = {
                    {"@odata.type",
                     "#PCIeDeviceCollection.PCIeDeviceCollection"},
                    {"@odata.id", "/redfish/v1/Systems/system/PCIeDevices"},
                    {"Name", "PCIe Device Collection"},
                    {"Description", "Collection of PCIe Devices"},
                    {"Members", nlohmann::json::array()},
                    {"Members@odata.count", 0}};
                getPCIeDeviceList(asyncResp, "Members");
            });
}

inline std::optional<std::string>
    redfishPcieGenerationFromDbus(const std::string& generationInUse)
{
    if (generationInUse ==
        "xyz.openbmc_project.Inventory.Item.PCIeSlot.Generations.Gen1")
    {
        return "Gen1";
    }
    if (generationInUse ==
        "xyz.openbmc_project.Inventory.Item.PCIeSlot.Generations.Gen2")
    {
        return "Gen2";
    }
    if (generationInUse ==
        "xyz.openbmc_project.Inventory.Item.PCIeSlot.Generations.Gen3")
    {
        return "Gen3";
    }
    if (generationInUse ==
        "xyz.openbmc_project.Inventory.Item.PCIeSlot.Generations.Gen4")
    {
        return "Gen4";
    }
    if (generationInUse ==
        "xyz.openbmc_project.Inventory.Item.PCIeSlot.Generations.Gen5")
    {
        return "Gen5";
    }
    if (generationInUse.empty() ||
        generationInUse ==
            "xyz.openbmc_project.Inventory.Item.PCIeSlot.Generations.Unknown")
    {
        return "";
    }

    // The value is not unknown or Gen1-5, need return an internal error.
    return std::nullopt;
}

inline void requestRoutesSystemPCIeDevice(App& app)
{
    BMCWEB_ROUTE(app, "/redfish/v1/Systems/system/PCIeDevices/<str>/")
        .privileges(redfish::privileges::getPCIeDevice)
        .methods(boost::beast::http::verb::get)(
            [](const crow::Request&,
               const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
               const std::string& device)

            {
                auto getPCIeDeviceCallback = [asyncResp, device](
                                                 const boost::system::error_code
                                                     ec,
                                                 const dbus::utility::
                                                     DBusPropertiesMap&
                                                         pcieDevProperties) {
                    if (ec)
                    {
                        BMCWEB_LOG_DEBUG
                            << "failed to get PCIe Device properties ec: "
                            << ec.value() << ": " << ec.message();
                        if (ec.value() ==
                            boost::system::linux_error::bad_request_descriptor)
                        {
                            messages::resourceNotFound(asyncResp->res,
                                                       "PCIeDevice", device);
                        }
                        else
                        {
                            messages::internalError(asyncResp->res);
                        }
                        return;
                    }

                    const std::string* manufacturer = nullptr;
                    const std::string* deviceType = nullptr;
                    const std::string* generationInUse = nullptr;

                    const bool success = sdbusplus::unpackPropertiesNoThrow(
                        dbus_utils::UnpackErrorPrinter(), pcieDevProperties,
                        "Manufacturer", manufacturer, "DeviceType", deviceType,
                        "GenerationInUse", generationInUse);

                    if (!success)
                    {
                        messages::internalError(asyncResp->res);
                        return;
                    }

                    asyncResp->res.jsonValue = {
                        {"@odata.type", "#PCIeDevice.v1_4_0.PCIeDevice"},
                        {"@odata.id",
                         "/redfish/v1/Systems/system/PCIeDevices/" + device},
                        {"Name", "PCIe Device"},
                        {"Id", device}};

                    if (manufacturer)
                    {
                        asyncResp->res.jsonValue["Manufacturer"] =
                            *manufacturer;
                    }

                    if (deviceType)
                    {
                        asyncResp->res.jsonValue["DeviceType"] = *deviceType;
                    }

                    if (generationInUse)
                    {
                        std::optional<std::string> redfishGenerationInUse =
                            redfishPcieGenerationFromDbus(*generationInUse);
                        if (!redfishGenerationInUse)
                        {
                            messages::internalError(asyncResp->res);
                            return;
                        }
                        if (redfishGenerationInUse->empty())
                        {
                            // unknown, no need to handle
                            return;
                        }
                        asyncResp->res.jsonValue["PCIeInterface"]["PCIeType"] =
                            *redfishGenerationInUse;
                    }

                    asyncResp->res.jsonValue["PCIeFunctions"] = {
                        {"@odata.id",
                         "/redfish/v1/Systems/system/PCIeDevices/" + device +
                             "/PCIeFunctions"}};
                };

                std::string escapedPath = std::string(pciePath) + "/" + device;
                dbus::utility::escapePathForDbus(escapedPath);
                sdbusplus::asio::getAllProperties(
                    *crow::connections::systemBus, pcieService, escapedPath,
                    pcieDeviceInterface, std::move(getPCIeDeviceCallback));
            });
}

inline void requestRoutesSystemPCIeFunctionCollection(App& app)
{
    /**
     * Functions triggers appropriate requests on DBus
     */
    BMCWEB_ROUTE(app,
                 "/redfish/v1/Systems/system/PCIeDevices/<str>/PCIeFunctions/")
        .privileges(redfish::privileges::getPCIeFunctionCollection)
        .methods(boost::beast::http::verb::get)(
            [](const crow::Request&,
               const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
               const std::string& device)

            {
                asyncResp->res.jsonValue = {
                    {"@odata.type",
                     "#PCIeFunctionCollection.PCIeFunctionCollection"},
                    {"@odata.id", "/redfish/v1/Systems/system/PCIeDevices/" +
                                      device + "/PCIeFunctions"},
                    {"Name", "PCIe Function Collection"},
                    {"Description",
                     "Collection of PCIe Functions for PCIe Device " + device}};

                auto getPCIeDeviceCallback = [asyncResp, device](
                                                 const boost::system::error_code
                                                     ec,
                                                 const dbus::utility::
                                                     DBusPropertiesMap&
                                                         pcieDevProperties) {
                    if (ec)
                    {
                        BMCWEB_LOG_DEBUG
                            << "failed to get PCIe Device properties ec: "
                            << ec.value() << ": " << ec.message();
                        if (ec.value() ==
                            boost::system::linux_error::bad_request_descriptor)
                        {
                            messages::resourceNotFound(asyncResp->res,
                                                       "PCIeDevice", device);
                        }
                        else
                        {
                            BMCWEB_LOG_DEBUG
                                << "failed to get PCIe Device properties ec: "
                                << ec.value() << ": " << ec.message();
                            if (ec.value() == boost::system::linux_error::
                                                  bad_request_descriptor)
                            {
                                messages::resourceNotFound(
                                    asyncResp->res, "PCIeDevice", device);
                            }
                            else
                            {
                                messages::internalError(asyncResp->res);
                            }
                            return;
                        }

                        nlohmann::json& pcieFunctionList =
                            asyncResp->res.jsonValue["Members"];
                        pcieFunctionList = nlohmann::json::array();
                        static constexpr const int maxPciFunctionNum = 8;
                        for (int functionNum = 0;
                             functionNum < maxPciFunctionNum; functionNum++)
                        {
                            // Check if this function exists by looking for a
                            // device ID
                            std::string devIDProperty =
                                "Function" + std::to_string(functionNum) +
                                "DeviceId";
                            auto it = std::find_if(
                                pcieDevProperties.begin(),
                                pcieDevProperties.end(),
                                [&devIDProperty](
                                    const std::pair<
                                        std::string,
                                        dbus::utility::DbusVariantType>& item) {
                                    return item.first == devIDProperty;
                                });
                            if (it != pcieDevProperties.end())
                            {
                                const std::string* property =
                                    std::get_if<std::string>(&it->second);
                                if (property != nullptr && !property->empty())
                                {
                                    pcieFunctionList.push_back(
                                        {{"@odata.id",
                                          "/redfish/v1/Systems/system/PCIeDevices/" +
                                              device + "/PCIeFunctions/" +
                                              std::to_string(functionNum)}});
                                }
                            }
                            asyncResp->res.jsonValue["Members@odata.count"] =
                                pcieFunctionList.size();
                        }
                    }
                };

                std::string escapedPath = std::string(pciePath) + "/" + device;
                dbus::utility::escapePathForDbus(escapedPath);
                sdbusplus::asio::getAllProperties(
                    *crow::connections::systemBus, pcieService, escapedPath,
                    pcieDeviceInterface, std::move(getPCIeDeviceCallback));
            });
}

inline void requestRoutesSystemPCIeFunction(App& app)
{
    BMCWEB_ROUTE(
        app,
        "/redfish/v1/Systems/system/PCIeDevices/<str>/PCIeFunctions/<str>/")
        .privileges(redfish::privileges::getPCIeFunction)
        .methods(
            boost::beast::http::verb::get)([](const crow::Request&,
                                              const std::shared_ptr<
                                                  bmcweb::AsyncResp>& asyncResp,
                                              const std::string& device,
                                              const std::string& function) {
            auto getPCIeDeviceCallback =
                [asyncResp, device, function](
                    const boost::system::error_code ec,
                    const dbus::utility::DBusPropertiesMap& pcieDevProperties) {
                    if (ec)
                    {
                        BMCWEB_LOG_DEBUG
                            << "failed to get PCIe Device properties ec: "
                            << ec.value() << ": " << ec.message();
                        if (ec.value() ==
                            boost::system::linux_error::bad_request_descriptor)
                        {
                            messages::resourceNotFound(asyncResp->res,
                                                       "PCIeDevice", device);
                        }
                        else
                        {
                            messages::internalError(asyncResp->res);
                        }
                        return;
                    }

                    const std::string* deviceId = nullptr;
                    const std::string* vendorId = nullptr;
                    const std::string* functionType = nullptr;
                    const std::string* deviceClass = nullptr;
                    const std::string* classCode = nullptr;
                    const std::string* revisionId = nullptr;
                    const std::string* subsystemId = nullptr;
                    const std::string* subsystemVendorId = nullptr;

                    const bool success = sdbusplus::unpackPropertiesNoThrow(
                        dbus_utils::UnpackErrorPrinter(), pcieDevProperties,
                        "Function" + function + "DeviceId", deviceId,
                        "Function" + function + "VendorId", vendorId,
                        "Function" + function + "FunctionType", functionType,
                        "Function" + function + "DeviceClass", deviceClass,
                        "Function" + function + "ClassCode", classCode,
                        "Function" + function + "RevisionId", revisionId,
                        "Function" + function + "SubsystemId", subsystemId,
                        "Function" + function + "SubsystemVendorId",
                        subsystemVendorId);

                    if (!success)
                    {
                        messages::internalError(asyncResp->res);
                        return;
                    }

                    // Check if this function exists by looking for a device
                    // ID
                    if (!deviceId || deviceId->empty())
                    {
                        messages::resourceNotFound(asyncResp->res,
                                                   "PCIeFunction", function);
                        return;
                    }

                    asyncResp->res.jsonValue = {
                        {"@odata.type", "#PCIeFunction.v1_2_0.PCIeFunction"},
                        {"@odata.id",
                         "/redfish/v1/Systems/system/PCIeDevices/" + device +
                             "/PCIeFunctions/" + function},
                        {"Name", "PCIe Function"},
                        {"Id", function},
                        {"FunctionId", std::stoi(function)},
                        {"Links",
                         {{"PCIeDevice",
                           {{"@odata.id",
                             "/redfish/v1/Systems/system/PCIeDevices/" +
                                 device}}}}}};

                    if (deviceId)
                    {
                        asyncResp->res.jsonValue["DeviceId"] = *deviceId;
                    }

                    if (vendorId)
                    {
                        asyncResp->res.jsonValue["VendorId"] = *vendorId;
                    }

                    if (functionType)
                    {
                        asyncResp->res.jsonValue["FunctionType"] =
                            *functionType;
                    }

                    if (deviceClass)
                    {
                        asyncResp->res.jsonValue["DeviceClass"] = *deviceClass;
                    }

                    if (classCode)
                    {
                        asyncResp->res.jsonValue["ClassCode"] = *classCode;
                    }

                    if (revisionId)
                    {
                        asyncResp->res.jsonValue["RevisionId"] = *revisionId;
                    }

                    if (subsystemId)

                    {
                        asyncResp->res.jsonValue["SubsystemId"] = *subsystemId;
                    }

                    if (subsystemVendorId)
                    {
                        asyncResp->res.jsonValue["SubsystemVendorId"] =
                            *subsystemVendorId;
                    }
                };
            std::string escapedPath = std::string(pciePath) + "/" + device;
            dbus::utility::escapePathForDbus(escapedPath);
            sdbusplus::asio::getAllProperties(
                *crow::connections::systemBus, pcieService, escapedPath,
                pcieDeviceInterface, std::move(getPCIeDeviceCallback));
        });
}

} // namespace redfish
