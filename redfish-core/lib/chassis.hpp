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

#include "node.hpp"

#include <boost/container/flat_map.hpp>
#include <variant>

namespace redfish
{

/**
 * @brief Retrieves chassis state properties over dbus
 *
 * @param[in] aResp - Shared pointer for completing asynchronous calls.
 *
 * @return None.
 */
void getChassisState(std::shared_ptr<AsyncResp> aResp)
{
    crow::connections::systemBus->async_method_call(
        [aResp{std::move(aResp)}](
            const boost::system::error_code ec,
            const std::variant<std::string> &chassisState) {
            if (ec)
            {
                BMCWEB_LOG_DEBUG << "DBUS response error " << ec;
                messages::internalError(aResp->res);
                return;
            }

            const std::string *s = std::get_if<std::string>(&chassisState);
            BMCWEB_LOG_DEBUG << "Chassis state: " << *s;
            if (s != nullptr)
            {
                // Verify Chassis State
                if (*s == "xyz.openbmc_project.State.Chassis.PowerState.On")
                {
                    aResp->res.jsonValue["PowerState"] = "On";
                    aResp->res.jsonValue["Status"]["State"] = "Enabled";
                }
                else if (*s ==
                         "xyz.openbmc_project.State.Chassis.PowerState.Off")
                {
                    aResp->res.jsonValue["PowerState"] = "Off";
                    aResp->res.jsonValue["Status"]["State"] = "StandbyOffline";
                }
            }
        },
        "xyz.openbmc_project.State.Chassis",
        "/xyz/openbmc_project/state/chassis0",
        "org.freedesktop.DBus.Properties", "Get",
        "xyz.openbmc_project.State.Chassis", "CurrentPowerState");
}

/**
 * DBus types primitives for several generic DBus interfaces
 * TODO(Pawel) consider move this to separate file into boost::dbus
 */
// Note, this is not a very useful Variant, but because it isn't used to get
// values, it should be as simple as possible
// TODO(ed) invent a nullvariant type
using VariantType = std::variant<bool, std::string, uint64_t>;
using ManagedObjectsType = std::vector<std::pair<
    sdbusplus::message::object_path,
    std::vector<std::pair<std::string,
                          std::vector<std::pair<std::string, VariantType>>>>>>;

using PropertiesType = boost::container::flat_map<std::string, VariantType>;

void getIntrusionByService(std::shared_ptr<AsyncResp> aResp,
                           const std::string &service,
                           const std::string &objPath)
{
    BMCWEB_LOG_DEBUG << "Get intrusion status by service \n";

    crow::connections::systemBus->async_method_call(
        [aResp{std::move(aResp)}](const boost::system::error_code ec,
                                  const std::variant<std::string> &value) {
            if (ec)
            {
                // do not add err msg in redfish response, becaues this is not
                //     mandatory property
                BMCWEB_LOG_ERROR << "DBUS response error " << ec << "\n";
                return;
            }

            const std::string *status = std::get_if<std::string>(&value);

            if (status == nullptr)
            {
                BMCWEB_LOG_ERROR << "intrusion status read error \n";
                return;
            }

            aResp->res.jsonValue["PhysicalSecurity"] = {
                {"IntrusionSensorNumber", 1}, {"IntrusionSensor", *status}};
        },
        service, objPath, "org.freedesktop.DBus.Properties", "Get",
        "xyz.openbmc_project.Chassis.Intrusion", "Status");
}

/**
 * Retrieves physical security properties over dbus
 */
void getPhysicalSecurityData(std::shared_ptr<AsyncResp> aResp)
{
    crow::connections::systemBus->async_method_call(
        [aResp{std::move(aResp)}](
            const boost::system::error_code ec,
            const std::vector<std::pair<
                std::string,
                std::vector<std::pair<std::string, std::vector<std::string>>>>>
                &subtree) {
            if (ec)
            {
                // do not add err msg in redfish response, becaues this is not
                //     mandatory property
                BMCWEB_LOG_ERROR << "DBUS error: no matched iface " << ec
                                 << "\n";
                return;
            }
            // Iterate over all retrieved ObjectPaths.
            for (const auto &object : subtree)
            {
                for (const auto &service : object.second)
                {
                    getIntrusionByService(aResp, service.first, object.first);
                    return;
                }
            }
        },
        "xyz.openbmc_project.ObjectMapper",
        "/xyz/openbmc_project/object_mapper",
        "xyz.openbmc_project.ObjectMapper", "GetSubTree",
        "/xyz/openbmc_project/Intrusion", int32_t(1),
        std::array<const char *, 1>{"xyz.openbmc_project.Chassis.Intrusion"});
}

void getChassisElements(const std::vector<std::string> &resourcesList,
                        std::pair<std::string, std::string> &chassisNode,
                        std::map<std::string, std::string> &subAssyList)
{
    std::string chassisType, subassembly;
    for (const std::string &objpath : resourcesList)
    {
        if ((boost::starts_with(objpath,
                                "/xyz/openbmc_project/inventory/system/")) &&
            (dbus::utility::getNthStringFromPath(objpath, 4, chassisType)) &&
            (dbus::utility::getNthStringFromPath(objpath, 5, subassembly)))
        {
            if ((chassisType == "board") || (chassisType == "powersupply"))
            {
                subAssyList.emplace(std::make_pair(subassembly, objpath));
            }
            else
            {
                chassisNode = std::make_pair(subassembly, objpath);
            }
        }
    }
}

/**
 * ChassisCollection derived class for delivering Chassis Collection Schema
 */
class ChassisCollection : public Node
{
  public:
    ChassisCollection(CrowApp &app) : Node(app, "/redfish/v1/Chassis/")
    {
        entityPrivileges = {
            {boost::beast::http::verb::get, {{"Login"}}},
            {boost::beast::http::verb::head, {{"Login"}}},
            {boost::beast::http::verb::patch, {{"ConfigureComponents"}}},
            {boost::beast::http::verb::put, {{"ConfigureComponents"}}},
            {boost::beast::http::verb::delete_, {{"ConfigureComponents"}}},
            {boost::beast::http::verb::post, {{"ConfigureComponents"}}}};
    }

  private:
    /**
     * Functions triggers appropriate requests on DBus
     */
    void doGet(crow::Response &res, const crow::Request &req,
               const std::vector<std::string> &params) override
    {
        res.jsonValue["@odata.type"] = "#ChassisCollection.ChassisCollection";
        res.jsonValue["@odata.id"] = "/redfish/v1/Chassis";
        res.jsonValue["@odata.context"] =
            "/redfish/v1/$metadata#ChassisCollection.ChassisCollection";
        res.jsonValue["Name"] = "Chassis Collection";

#ifdef BMCWEB_ENABLE_REDFISH_ONE_CHASSIS
        // Assume one Chassis named "chassis"
        res.jsonValue["Members@odata.count"] = 1;
        res.jsonValue["Members"] = {
            {{"@odata.id", "/redfish/v1/Chassis/chassis"}}};
        res.end();
        return;
#endif
        const std::array<const char *, 3> interfaces = {
            "xyz.openbmc_project.Inventory.Item.Board",
            "xyz.openbmc_project.Inventory.Item.Chassis",
            "xyz.openbmc_project.Inventory.Item.PowerSupply"};

        auto asyncResp = std::make_shared<AsyncResp>(res);
        crow::connections::systemBus->async_method_call(
            [asyncResp](const boost::system::error_code ec,
                        const std::vector<std::string> &resourcesList) {
                if (ec)
                {
                    messages::internalError(asyncResp->res);
                    return;
                }
                std::map<std::string, std::string> subAssyList;
                std::pair<std::string, std::string> chassisNode;
                nlohmann::json &chassisArray =
                    asyncResp->res.jsonValue["Members"];
                chassisArray = nlohmann::json::array();

                getChassisElements(resourcesList, chassisNode, subAssyList);
                chassisArray.push_back({{"@odata.id", "/redfish/v1/Chassis/" +
                                                          chassisNode.first}});

                for (auto entry : subAssyList)
                {
                    chassisArray.push_back(
                        {{"@odata.id", "/redfish/v1/Chassis/" +
                                           chassisNode.first + "/" +
                                           entry.first}});
                }

                asyncResp->res.jsonValue["Members@odata.count"] =
                    chassisArray.size();
            },
            "xyz.openbmc_project.ObjectMapper",
            "/xyz/openbmc_project/object_mapper",
            "xyz.openbmc_project.ObjectMapper", "GetSubTreePaths",
            "/xyz/openbmc_project/inventory", int32_t(0), interfaces);
    }
}; // namespace redfish

/**
 * Chassis override class for delivering Chassis Schema
 */
class Chassis : public Node
{
  public:
    Chassis(CrowApp &app) :
        Node(app, "/redfish/v1/Chassis/<str>/", std::string())
    {
        entityPrivileges = {
            {boost::beast::http::verb::get, {{"Login"}}},
            {boost::beast::http::verb::head, {{"Login"}}},
            {boost::beast::http::verb::patch, {{"ConfigureComponents"}}},
            {boost::beast::http::verb::put, {{"ConfigureComponents"}}},
            {boost::beast::http::verb::delete_, {{"ConfigureComponents"}}},
            {boost::beast::http::verb::post, {{"ConfigureComponents"}}}};
    }

  private:
    /**
     * Functions triggers appropriate requests on DBus
     */
    void doGet(crow::Response &res, const crow::Request &req,
               const std::vector<std::string> &params) override
    {
        // Check if there is required param, truly entering this shall be
        // impossible.
        if (params.size() != 1)
        {
            messages::internalError(res);
            res.end();
            return;
        }
        const std::string &chassisId = params[0];
        const std::array<const char *, 1> interfaces = {
            "xyz.openbmc_project.Inventory.Item.Chassis"};

#ifdef BMCWEB_ENABLE_REDFISH_ONE_CHASSIS
        // In a one chassis system the only supported name is "chassis"
        if (chassisId != "chassis")
        {
            messages::resourceNotFound(res, "#Chassis.v1_4_0.Chassis",
                                       chassisId);
            res.end();
            return;
        }
#endif

        res.jsonValue["@odata.type"] = "#Chassis.v1_4_0.Chassis";
        res.jsonValue["@odata.id"] = "/redfish/v1/Chassis/" + chassisId;
        res.jsonValue["@odata.context"] =
            "/redfish/v1/$metadata#Chassis.Chassis";
        res.jsonValue["Name"] = "Chassis Collection";
        res.jsonValue["ChassisType"] = "RackMount";

        auto asyncResp = std::make_shared<AsyncResp>(res);
        crow::connections::systemBus->async_method_call(
            [asyncResp, chassisId(std::string(chassisId))](
                const boost::system::error_code ec,
                const std::vector<std::pair<
                    std::string, std::vector<std::pair<
                                     std::string, std::vector<std::string>>>>>
                    &subtree) {
                if (ec)
                {
                    messages::internalError(asyncResp->res);
                    return;
                }
                // Iterate over all retrieved ObjectPaths.
                for (const std::pair<
                         std::string,
                         std::vector<
                             std::pair<std::string, std::vector<std::string>>>>
                         &object : subtree)
                {
                    const std::string &path = object.first;
                    const std::vector<
                        std::pair<std::string, std::vector<std::string>>>
                        &connectionNames = object.second;

// If only one chassis, just select the first one
#ifndef BMCWEB_ENABLE_REDFISH_ONE_CHASSIS
                    if (!boost::ends_with(path, chassisId))
                    {
                        continue;
                    }
#endif
                    if (connectionNames.size() < 1)
                    {
                        BMCWEB_LOG_ERROR << "Only got "
                                         << connectionNames.size()
                                         << " Connection names";
                        continue;
                    }

                    const std::string connectionName = connectionNames[0].first;
                    crow::connections::systemBus->async_method_call(
                        [asyncResp, chassisId(std::string(chassisId))](
                            const boost::system::error_code ec,
                            const std::vector<std::pair<
                                std::string, VariantType>> &propertiesList) {
                            for (const std::pair<std::string, VariantType>
                                     &property : propertiesList)
                            {
                                // Store DBus properties that are also
                                // Redfish properties with same name and a
                                // string value
                                const std::string &propertyName =
                                    property.first;
                                if ((propertyName == "PartNumber") ||
                                    (propertyName == "SerialNumber") ||
                                    (propertyName == "Manufacturer") ||
                                    (propertyName == "Model"))
                                {
                                    const std::string *value =
                                        std::get_if<std::string>(
                                            &property.second);
                                    if (value != nullptr)
                                    {
                                        asyncResp->res.jsonValue[propertyName] =
                                            *value;
                                    }
                                }
                            }
                            asyncResp->res.jsonValue["Name"] = chassisId;
                            asyncResp->res.jsonValue["Id"] = chassisId;
                            asyncResp->res.jsonValue["Thermal"] = {
                                {"@odata.id", "/redfish/v1/Chassis/" +
                                                  chassisId + "/Thermal"}};
                            // Power object
                            asyncResp->res.jsonValue["Power"] = {
                                {"@odata.id", "/redfish/v1/Chassis/" +
                                                  chassisId + "/Power"}};
                            asyncResp->res.jsonValue["Status"] = {
                                {"Health", "OK"},
                                {"State", "Enabled"},
                            };

                            asyncResp->res
                                .jsonValue["Links"]["ComputerSystems"] = {
                                {{"@odata.id", "/redfish/v1/Systems/system"}}};
                            asyncResp->res.jsonValue["Links"]["ManagedBy"] = {
                                {{"@odata.id", "/redfish/v1/Managers/bmc"}}};
                            getChassisState(asyncResp);
                        },
                        connectionName, path, "org.freedesktop.DBus.Properties",
                        "GetAll",
                        "xyz.openbmc_project.Inventory.Decorator.Asset");
                    return;
                }

                // Couldn't find an object with that name.  return an error
                messages::resourceNotFound(
                    asyncResp->res, "#Chassis.v1_4_0.Chassis", chassisId);
            },
            "xyz.openbmc_project.ObjectMapper",
            "/xyz/openbmc_project/object_mapper",
            "xyz.openbmc_project.ObjectMapper", "GetSubTree",
            "/xyz/openbmc_project/inventory", int32_t(0), interfaces);

        getPhysicalSecurityData(asyncResp);
    }
};

class ChassisSubAssy : public Node
{
  public:
    ChassisSubAssy(CrowApp &app) :
        Node(app, "/redfish/v1/Chassis/<str>/<str>", std::string(),
             std::string())
    {
        entityPrivileges = {
            {boost::beast::http::verb::get, {{"Login"}}},
            {boost::beast::http::verb::head, {{"Login"}}},
            {boost::beast::http::verb::patch, {{"ConfigureComponents"}}},
            {boost::beast::http::verb::put, {{"ConfigureComponents"}}},
            {boost::beast::http::verb::delete_, {{"ConfigureComponents"}}},
            {boost::beast::http::verb::post, {{"ConfigureComponents"}}}};
    }

  private:
    bool findSubAssyElement(const std::string &path, const std::string &nodeId)
    {
        std::string chassisType, subassembly;
        if ((boost::starts_with(path,
                                "/xyz/openbmc_project/inventory/system/")) &&
            (dbus::utility::getNthStringFromPath(path, 4, chassisType)) &&
            (dbus::utility::getNthStringFromPath(path, 5, subassembly)))
        {
            return (
                ((chassisType == "board") || (chassisType == "powersupply")) &&
                (subassembly == nodeId.c_str()));
        }
        return false;
    }

    void doGetSubAssembly(const std::shared_ptr<AsyncResp> asyncResp,
                          const std::string &path,
                          const std::string &connectionName,
                          const std::string &chassisId,
                          const std::string &nodeId)
    {
        crow::connections::systemBus->async_method_call(
            [asyncResp, chassisId(std::string(chassisId)),
             nodeId(std::string(nodeId))](
                const boost::system::error_code ec,
                const std::vector<std::pair<std::string, VariantType>>
                    &propertiesList) {
                asyncResp->res.jsonValue["@odata.type"] =
                    "#Chassis.v1_4_0.Chassis";
                asyncResp->res.jsonValue["@odata.id"] =
                    "/redfish/v1/Chassis/" + chassisId + "/" + nodeId;
                asyncResp->res.jsonValue["@odata.context"] =
                    "/redfish/v1/$metadata#Chassis.Chassis";
                asyncResp->res.jsonValue["ChassisType"] = "Card";

                for (const std::pair<std::string, VariantType> &property :
                     propertiesList)
                {
                    // Store DBus properties that are also
                    // Redfish properties with same name and a
                    // string value
                    const std::string &propertyName = property.first;
                    if ((propertyName == "PartNumber") ||
                        (propertyName == "SerialNumber") ||
                        (propertyName == "Manufacturer") ||
                        (propertyName == "Model"))
                    {
                        const std::string *value =
                            std::get_if<std::string>(&property.second);
                        if (value != nullptr)
                        {
                            asyncResp->res.jsonValue[propertyName] = *value;
                        }
                    }
                }
                asyncResp->res.jsonValue["Name"] = nodeId;
                asyncResp->res.jsonValue["Id"] = nodeId;
                asyncResp->res.jsonValue["Thermal"] = {
                    {"@odata.id", "/redfish/v1/Chassis/" + chassisId + "/" +
                                      nodeId + "/Thermal"}};
                // Power object
                asyncResp->res.jsonValue["Power"] = {
                    {"@odata.id", "/redfish/v1/Chassis/" + chassisId + "/" +
                                      nodeId + "/Power"}};
                asyncResp->res.jsonValue["Status"] = {
                    {"Health", "OK"},
                    {"State", "Enabled"},
                };

                asyncResp->res.jsonValue["Links"]["ComputerSystems"] = {
                    {{"@odata.id", "/redfish/v1/Systems/system"}}};
                asyncResp->res.jsonValue["Links"]["ManagedBy"] = {
                    {{"@odata.id", "/redfish/v1/Managers/bmc"}}};
                asyncResp->res.jsonValue["Links"]["ContainedBy"] = {
                    {{"@odata.id", "/redfish/v1/Chassis/" + chassisId}}};
            },
            connectionName, path, "org.freedesktop.DBus.Properties", "GetAll",
            "xyz.openbmc_project.Inventory.Decorator.Asset");
    }

    /**
     * Functions triggers appropriate requests on DBus
     */
    void doGet(crow::Response &res, const crow::Request &req,
               const std::vector<std::string> &params) override
    {
        if (params.size() != 2)
        {
            messages::internalError(res);
            res.end();
            return;
        }
        const std::array<const char *, 2> interfaces = {
            "xyz.openbmc_project.Inventory.Item.Board",
            "xyz.openbmc_project.Inventory.Item.PowerSupply"};
        const std::string &chassisId = params[0];
        const std::string &nodeId = params[1];

        auto asyncResp = std::make_shared<AsyncResp>(res);
        crow::connections::systemBus->async_method_call(
            [asyncResp, chassisId(std::string(chassisId)),
             nodeId(std::string(nodeId)), this](
                const boost::system::error_code ec,
                const std::vector<std::pair<
                    std::string, std::vector<std::pair<
                                     std::string, std::vector<std::string>>>>>
                    &subtree) {
                if (ec)
                {
                    messages::internalError(asyncResp->res);
                    return;
                }
                // Iterate over all retrieved ObjectPaths.
                for (const std::pair<
                         std::string,
                         std::vector<
                             std::pair<std::string, std::vector<std::string>>>>
                         &object : subtree)
                {
                    const std::string &path = object.first;
                    const std::vector<
                        std::pair<std::string, std::vector<std::string>>>
                        &connectionNames = object.second;
                    const std::string connectionName = connectionNames[0].first;
                    if (findSubAssyElement(path, nodeId))
                    {
                        doGetSubAssembly(asyncResp, path, connectionName,
                                         chassisId, nodeId);
                        return;
                    }
                }
                // Couldn't find an object with that name.  return an error
                messages::resourceNotFound(asyncResp->res,
                                           "#Chassis.v1_4_0.Chassis", nodeId);
            },
            "xyz.openbmc_project.ObjectMapper",
            "/xyz/openbmc_project/object_mapper",
            "xyz.openbmc_project.ObjectMapper", "GetSubTree",
            "/xyz/openbmc_project/inventory", int32_t(0), interfaces);

        getPhysicalSecurityData(asyncResp);
    }
};
} // namespace redfish
