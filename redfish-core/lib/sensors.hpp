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

#include <math.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/asio.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/range/algorithm/replace_copy_if.hpp>
#include <dbus_singleton.hpp>
#include <utils/json_utils.hpp>
#include <variant>

namespace redfish
{

using GetSubTreeType = std::vector<
    std::pair<std::string,
              std::vector<std::pair<std::string, std::vector<std::string>>>>>;

using SensorVariant = std::variant<int64_t, double, uint32_t, bool>;

using ManagedObjectsVectorType = std::vector<std::pair<
    sdbusplus::message::object_path,
    boost::container::flat_map<
        std::string, boost::container::flat_map<std::string, SensorVariant>>>>;

using GetObjectType =
    std::vector<std::pair<std::string, std::vector<std::string>>>;

/**
 * SensorsAsyncResp
 * Gathers data needed for response processing after async calls are done
 */
class SensorsAsyncResp
{
  public:
    SensorsAsyncResp(crow::Response& response, const std::string& chassisId,
                     const std::initializer_list<const char*> types,
                     const std::string& subNode) :
        res(response),
        chassisId(chassisId), types(types), chassisSubNode(subNode)
    {
    }

    SensorsAsyncResp(crow::Response& response, const std::string& chassisId) :
        res(response), chassisId(chassisId)
    {
    }

    ~SensorsAsyncResp()
    {
        if (res.result() == boost::beast::http::status::internal_server_error)
        {
            // Reset the json object to clear out any data that made it in
            // before the error happened todo(ed) handle error condition with
            // proper code
            res.jsonValue = nlohmann::json::object();
        }
        res.end();
    }

    crow::Response& res;
    std::string chassisId{};
    const std::vector<const char*> types;
    std::string chassisSubNode{};
};

class SensorCollection : public Node
{
  public:
    SensorCollection(CrowApp& app) :
        Node(app, "/redfish/v1/Chassis/<str>/Sensors", std::string())
    {
        entityPrivileges = {
            {boost::beast::http::verb::get, {{"Login"}}},
            {boost::beast::http::verb::head, {{"Login"}}},
            {boost::beast::http::verb::patch, {{"ConfigureManager"}}},
            {boost::beast::http::verb::put, {{"ConfigureManager"}}},
            {boost::beast::http::verb::delete_, {{"ConfigureManager"}}},
            {boost::beast::http::verb::post, {{"ConfigureManager"}}}};
    }

  private:
    void doGet(crow::Response &res, const crow::Request &req,
               const std::vector<std::string> &params) override
    {
        BMCWEB_LOG_DEBUG << "SensorCollection doGet enter";
        if (params.size() != 1)
        {
            BMCWEB_LOG_DEBUG << "SensorCollection doGet param size < 1";
            messages::internalError(res);
            res.end();
            return;
        }
        const std::string &chassisId = params[0];
        std::shared_ptr<SensorsAsyncResp> asyncResp =
            std::make_shared<SensorsAsyncResp>(res, chassisId);

        asyncResp->res.jsonValue["@odata.type"] =
            "#SensorCollection.SensorCollection";
        asyncResp->res.jsonValue["@odata.id"] = "/redfish/v1/Chassis/" +
                                                chassisId + "/Sensors";
        asyncResp->res.jsonValue["@odata.context"] =
            "/redfish/v1/$metadata#SensorCollection.SensorCollection";
        asyncResp->res.jsonValue["Name"] = "Sensor Collection";
        asyncResp->res.jsonValue["Description"] =
            "Collection of Sensors for this Chassis";
        asyncResp->res.jsonValue["Members"] = nlohmann::json::array();
        asyncResp->res.jsonValue["Members@odata.count"] = 0;

        const std::array<const char*, 1> interfaces = {
            "xyz.openbmc_project.Sensor.Value"};

        const std::vector<std::string> excludeSensors = {
            "/xyz/openbmc_project/sensors/temperature/",
            "/xyz/openbmc_project/sensors/voltage/",
            "/xyz/openbmc_project/sensors/fan_tach/"};

        // Get list of all sensors and filter out the types we don't want
        crow::connections::systemBus->async_method_call(
            [asyncResp,
             excludeSensors](const boost::system::error_code ec,
                             const GetSubTreeType& subtree)
            {
                BMCWEB_LOG_DEBUG << "respHandler1 enter";
                if (ec)
                {
                    messages::internalError(asyncResp->res);
                    BMCWEB_LOG_ERROR
                        << "SensorCollection getSensorList resp_handler: "
                        << "Dbus error " << ec;
                    return;
                }

                std::vector<std::string> nodeSensorList;
                for (const std::pair<
                        std::string,
                        std::vector<std::pair<std::string,
                                    std::vector<std::string>>>>&
                                    object : subtree)
                {
                    BMCWEB_LOG_DEBUG << "Checking if sensor excluded: "
                                     << object.first;
                    if (std::find_if(excludeSensors.begin(),
                                     excludeSensors.end(),
                                     [object](const std::string& str)
                                     {
                                         BMCWEB_LOG_DEBUG <<
                                            "In find_if: object=" <<
                                            object.first;

                                         return object.first.find(str) !=
                                                std::string::npos;
                                     }) != excludeSensors.end())
                    {
                        BMCWEB_LOG_DEBUG << "Sensor exlcuded: "
                                         << object.first;
                        continue;
                    }
                    BMCWEB_LOG_DEBUG << "Sensor kept: " << object.first;
                    nodeSensorList.push_back(object.first);
                }

                for(auto & sensor : nodeSensorList)
                {
                    BMCWEB_LOG_DEBUG << "Validating sensor " << sensor;
                    // Get the endpoints of the sensor
                    crow::connections::systemBus->async_method_call(
                        [sensor, asyncResp](
                            const boost::system::error_code ec,
                            std::variant<std::vector<std::string>>
                                variantEndpoints)
                        {
                            BMCWEB_LOG_DEBUG << "respHandler2 enter: sensor="
                                             << sensor;
                            if (ec)
                            {
                                return; // if they don't have an association we
                                        // just skip it
                            }

                            auto endpoints = std::get_if<std::vector<
                                                    std::string>>(
                                                        &variantEndpoints);

                            if (endpoints == nullptr)
                            {
                                BMCWEB_LOG_ERROR <<
                                    "Invalid association interface";
                                messages::internalError(asyncResp->res);
                                return;
                            }

                            // Assume there is only one endpoint
                            std::string endpoint = *(endpoints->begin());
                            BMCWEB_LOG_DEBUG << "endpoint=" << endpoint;
                            const std::array<const char*, 1> excludeInventory =
                                {"xyz.openbmc_project.Inventory.Item"
                                    ".PowerSupply"};

                            // Check the interfaces associated with the sensor
                            // and make sure it doesn't contain any in the
                            // excludeInventory list
                            crow::connections::systemBus->async_method_call(
                                [sensor, endpoint, excludeInventory, asyncResp](
                                    const boost::system::error_code ec,
                                    const GetObjectType &objects)
                                {
                                    BMCWEB_LOG_DEBUG << "respHandler3 enter";
                                    if (ec)
                                    {
                                        BMCWEB_LOG_DEBUG <<
                                            "GetObject for path " <<
                                            endpoint <<
                                            " has error code " <<
                                            ec;
                                    }

                                    bool include = true;
                                    for(auto object : objects)
                                    {
                                        for(auto inventory : object.second)
                                        {
                                            if (std::find_if(
                                                    excludeInventory.begin(),
                                                    excludeInventory.end(),
                                                [inventory]
                                                    (const std::string& str)
                                                {
                                                    BMCWEB_LOG_DEBUG <<
                                                        "In find_if: inventory="
                                                        << inventory << "  STR="
                                                        << str;
                                                    return inventory ==
                                                            str;
                                                }) != excludeInventory.end())
                                            {
                                                include = false;
                                            }
                                        }
                                    }

                                    if(include)
                                    {
                                        BMCWEB_LOG_DEBUG <<
                                            "respHandler3 USE sensor: " <<
                                            sensor;
                                        std::size_t lastPos = sensor.rfind("/");
                                        if (lastPos == std::string::npos)
                                        {
                                            BMCWEB_LOG_ERROR <<
                                                "Failed to find '/' in " <<
                                                sensor;
                                            return;
                                        }
                                        std::string sensorName =
                                            sensor.substr(lastPos + 1);
                                        nlohmann::json &entriesArray =
                                            asyncResp->res.jsonValue["Members"];
                                        entriesArray.push_back({});
                                        nlohmann::json &thisEntry =
                                            entriesArray.back();
                                        thisEntry = {{"@odata.id",
                                            "/redfish/v1/Chassis/" +
                                            asyncResp->chassisId +
                                            "/Sensors/" + sensorName}};

                                        nlohmann::json::number_integer_t*
                                        val = asyncResp->res.jsonValue[
                                                "Members@odata.count"
                                              ].get_ptr<nlohmann::json::
                                                  number_integer_t*>();
                                        if (val == nullptr)
                                        {
                                            // handle error
                                            BMCWEB_LOG_DEBUG <<
                                                "Unexpected error updating"
                                                "Members@odata.count";
                                            return;
                                        }
                                        (*val)++;
                                        return;
                                    }
                                    BMCWEB_LOG_DEBUG << "respHandler3 exit";
                                },
                                "xyz.openbmc_project.ObjectMapper",
                                "/xyz/openbmc_project/object_mapper",
                                "xyz.openbmc_project.ObjectMapper",
                                "GetObject", endpoint,
                                std::array<std::string, 0>());

                            BMCWEB_LOG_DEBUG << "respHandler2 exit";

                        },
                        "xyz.openbmc_project.ObjectMapper",
                        sensor + "/inventory",
                        "org.freedesktop.DBus.Properties", "Get",
                        "xyz.openbmc_project.Association", "endpoints");
                }

                BMCWEB_LOG_DEBUG << "respHandler1 exit";
            },
            "xyz.openbmc_project.ObjectMapper",
            "/xyz/openbmc_project/object_mapper",
            "xyz.openbmc_project.ObjectMapper", "GetSubTree",
            "/xyz/openbmc_project/sensors", 2, interfaces);

        BMCWEB_LOG_DEBUG << "SensorCollection doGet exit";
    }
};

class Sensor : public Node
{
  public:
    Sensor(CrowApp &app) :
        Node(app,
             "/redfish/v1/Chassis/<str>/Sensors/<str>/",
             std::string(), std::string())
    {
        entityPrivileges = {
            {boost::beast::http::verb::get, {{"Login"}}},
            {boost::beast::http::verb::head, {{"Login"}}},
            {boost::beast::http::verb::patch, {{"ConfigureManager"}}},
            {boost::beast::http::verb::put, {{"ConfigureManager"}}},
            {boost::beast::http::verb::delete_, {{"ConfigureManager"}}},
            {boost::beast::http::verb::post, {{"ConfigureManager"}}}};
    }

  private:
    void doGet(crow::Response &res, const crow::Request &req,
               const std::vector<std::string> &params) override
    {
        BMCWEB_LOG_DEBUG << "Sensor doGet enter";
        if (params.size() != 2)
        {
            BMCWEB_LOG_DEBUG << "Sensor doGet param size < 2";
            messages::internalError(res);
            res.end();
            return;
        }
        const std::string &chassisId = params[0];
        std::shared_ptr<SensorsAsyncResp> asyncResp =
            std::make_shared<SensorsAsyncResp>(res, chassisId);

        const std::string &sensorName = params[1];
        const std::array<const char*, 1> interfaces = {
            "xyz.openbmc_project.Sensor.Value"};

        // Get a list of all of the sensors that implement Sensor.Value
        // and get the path and service name associated with the sensor
        crow::connections::systemBus->async_method_call(
            [asyncResp, sensorName](const boost::system::error_code ec,
                                    const GetSubTreeType& subtree)
            {
                BMCWEB_LOG_DEBUG << "respHandler1 enter";
                if (ec)
                {
                    messages::internalError(asyncResp->res);
                    BMCWEB_LOG_ERROR
                        << "Sensor getSensorPaths resp_handler: "
                        << "Dbus error " << ec;
                    return;
                }

                auto it = std::find_if(subtree.begin(), subtree.end(),
                               [sensorName](const std::pair<std::string,
                                                  std::vector<std::pair<
                                                  std::string, std::vector<
                                                  std::string>>>> &object)
                               {
                                   return object.first.find(sensorName) !=
                                          std::string::npos;
                               });

                if(it == subtree.end())
                {
                    BMCWEB_LOG_ERROR
                        << "Could not find path for sensor: " << sensorName;
                    return;
                }
                std::string sensorPath = (*it).first;
                BMCWEB_LOG_DEBUG << "Found sensor path for sensor '" <<
                                    sensorName << "': " << sensorPath;

                // This should always be correct if we found the sensorPath
                std::string owner = (*it).second.begin()->first;
                BMCWEB_LOG_DEBUG << "Found service for sensor '" <<
                                    sensorName << "': " << owner;

                // Get the sensor properties
                crow::connections::systemBus->async_method_call(
                    [sensorName, sensorPath, asyncResp](
                        const boost::system::error_code ec,
                        const boost::container::flat_map<std::string,
                                  std::variant<std::string,
                                               int64_t>>& ret)
                    {
                        if (ec)
                        {
                            messages::internalError(asyncResp->res);
                            BMCWEB_LOG_ERROR
                                << "Sensor getSensorValues resp_handler: "
                                << "Dbus error " << ec;
                            return;
                        }

                        asyncResp->res.jsonValue["@odata.type"] =
                            "#Sensor.v1_0_0.Sensor";
                        asyncResp->res.jsonValue["@odata.context"] =
                            "/redfish/v1/$metadata#Sensor.Sensor";
                        asyncResp->res.jsonValue["@odata.id"] =
                            "/redfish/v1/Chassis/" + asyncResp->chassisId +
                            "/Sensors/" + sensorName;
                        asyncResp->res.jsonValue["Name"] = "Sensor";
                        asyncResp->res.jsonValue["Id"] = sensorName;

                        const int64_t *value;
                        const int64_t *max;
                        const int64_t *min;
                        const std::string *unit;
                        const int64_t *scale;
                        for (auto &obj : ret)
                        {
                            BMCWEB_LOG_DEBUG << "Found data for: " << obj.first;
                            if(obj.first == "Value")
                            {
                                value = sdbusplus::message::variant_ns::get_if<
                                    int64_t>(&obj.second);
                            }
                            else if(obj.first == "MaxValue")
                            {
                                max = sdbusplus::message::variant_ns::get_if<
                                    int64_t>(&obj.second);
                            }
                            else if(obj.first == "MinValue")
                            {
                                min = sdbusplus::message::variant_ns::get_if<
                                    int64_t>(&obj.second);
                            }
                            else if(obj.first == "Unit")
                            {
                                unit = sdbusplus::message::variant_ns::get_if<
                                    std::string>(&obj.second);
                            }
                            else if(obj.first == "Scale")
                            {
                                scale = sdbusplus::message::variant_ns::get_if<
                                    int64_t>(&obj.second);
                            }
                            else
                            {
                                BMCWEB_LOG_DEBUG << "Unexpected sensor data: "
                                                 << obj.first;
                            }
                        }

                        if(value == nullptr)
                        {
                            messages::propertyMissing(asyncResp->res,
                                                      "Value");
                        }
                        if(max == nullptr)
                        {
                            messages::propertyMissing(asyncResp->res,
                                                      "MaxValue");
                        }
                        if(min == nullptr)
                        {
                            messages::propertyMissing(asyncResp->res,
                                                      "MinValue");
                        }
                        if(max == nullptr)
                        {
                            messages::propertyMissing(asyncResp->res,
                                                      "Unit");
                        }
                        if(scale == nullptr)
                        {
                            messages::propertyMissing(asyncResp->res,
                                                      "Scale");
                        }

                        asyncResp->res.jsonValue["Reading"] =
                            *value * std::pow(10, *scale);
                        asyncResp->res.jsonValue["ReadingRangeMax"] =
                            *max;
                        asyncResp->res.jsonValue["ReadingRangeMin"] =
                            *min;
                        asyncResp->res.jsonValue["ReadingUnits"] =
                            *unit;
                        // Not sure if this property is necessary
                        // or if Scale even maps to Precesion.
                        // Tried mapping it to PowerFactor, and was getting
                        // strange failures from RedfishServiceValidator
                        asyncResp->res.jsonValue["Precision"] =
                            *scale;
                    },
                    owner, sensorPath,
                    "org.freedesktop.DBus.Properties",
                    "GetAll", "xyz.openbmc_project.Sensor.Value");

                BMCWEB_LOG_DEBUG << "respHandler1 exit";
            },
            "xyz.openbmc_project.ObjectMapper",
            "/xyz/openbmc_project/object_mapper",
            "xyz.openbmc_project.ObjectMapper", "GetSubTree",
            "/xyz/openbmc_project/sensors", 2, interfaces);
    }
};

/**
 * @brief Get objects with connection necessary for sensors
 * @param SensorsAsyncResp Pointer to object holding response data
 * @param sensorNames Sensors retrieved from chassis
 * @param callback Callback for processing gathered connections
 */
template <typename Callback>
void getObjectsWithConnection(
    std::shared_ptr<SensorsAsyncResp> SensorsAsyncResp,
    const std::shared_ptr<boost::container::flat_set<std::string>> sensorNames,
    Callback&& callback)
{
    BMCWEB_LOG_DEBUG << "getObjectsWithConnection enter";
    const std::string path = "/xyz/openbmc_project/sensors";
    const std::array<std::string, 1> interfaces = {
        "xyz.openbmc_project.Sensor.Value"};

    // Response handler for parsing objects subtree
    auto respHandler = [callback{std::move(callback)}, SensorsAsyncResp,
                        sensorNames](const boost::system::error_code ec,
                                     const GetSubTreeType& subtree) {
        BMCWEB_LOG_DEBUG << "getObjectsWithConnection resp_handler enter";
        if (ec)
        {
            messages::internalError(SensorsAsyncResp->res);
            BMCWEB_LOG_ERROR
                << "getObjectsWithConnection resp_handler: Dbus error " << ec;
            return;
        }

        BMCWEB_LOG_DEBUG << "Found " << subtree.size() << " subtrees";

        // Make unique list of connections only for requested sensor types and
        // found in the chassis
        boost::container::flat_set<std::string> connections;
        std::set<std::pair<std::string, std::string>> objectsWithConnection;
        // Intrinsic to avoid malloc.  Most systems will have < 8 sensor
        // producers
        connections.reserve(8);

        BMCWEB_LOG_DEBUG << "sensorNames list count: " << sensorNames->size();
        for (const std::string& tsensor : *sensorNames)
        {
            BMCWEB_LOG_DEBUG << "Sensor to find: " << tsensor;
        }

        for (const std::pair<
                 std::string,
                 std::vector<std::pair<std::string, std::vector<std::string>>>>&
                 object : subtree)
        {
            if (sensorNames->find(object.first) != sensorNames->end())
            {
                for (const std::pair<std::string, std::vector<std::string>>&
                         objData : object.second)
                {
                    BMCWEB_LOG_DEBUG << "Adding connection: " << objData.first;
                    connections.insert(objData.first);
                    objectsWithConnection.insert(
                        std::make_pair(object.first, objData.first));
                }
            }
        }
        BMCWEB_LOG_DEBUG << "Found " << connections.size() << " connections";
        callback(std::move(connections), std::move(objectsWithConnection));
        BMCWEB_LOG_DEBUG << "getObjectsWithConnection resp_handler exit";
    };
    // Make call to ObjectMapper to find all sensors objects
    crow::connections::systemBus->async_method_call(
        std::move(respHandler), "xyz.openbmc_project.ObjectMapper",
        "/xyz/openbmc_project/object_mapper",
        "xyz.openbmc_project.ObjectMapper", "GetSubTree", path, 2, interfaces);
    BMCWEB_LOG_DEBUG << "getObjectsWithConnection exit";
}

/**
 * @brief Create connections necessary for sensors
 * @param SensorsAsyncResp Pointer to object holding response data
 * @param sensorNames Sensors retrieved from chassis
 * @param callback Callback for processing gathered connections
 */
template <typename Callback>
void getConnections(
    std::shared_ptr<SensorsAsyncResp> SensorsAsyncResp,
    const std::shared_ptr<boost::container::flat_set<std::string>> sensorNames,
    Callback&& callback)
{
    auto objectsWithConnectionCb =
        [callback](const boost::container::flat_set<std::string>& connections,
                   const std::set<std::pair<std::string, std::string>>&
                       objectsWithConnection) {
            callback(std::move(connections));
        };
    getObjectsWithConnection(SensorsAsyncResp, sensorNames,
                             std::move(objectsWithConnectionCb));
}

/**
 * @brief Shrinks the list of sensors for processing
 * @param SensorsAysncResp  The class holding the Redfish response
 * @param allSensors  A list of all the sensors associated to the
 * chassis element (i.e. baseboard, front panel, etc...)
 * @param activeSensors A list that is a reduction of the incoming
 * allSensors list.  Eliminate Thermal sensors when a Power request is
 * made, and eliminate Power sensors when a Thermal request is made.
 */
void reduceSensorList(
    std::shared_ptr<SensorsAsyncResp> SensorsAsyncResp,
    const std::vector<std::string>* allSensors,
    std::shared_ptr<boost::container::flat_set<std::string>> activeSensors)
{
    if (SensorsAsyncResp == nullptr)
    {
        return;
    }
    if ((allSensors == nullptr) || (activeSensors == nullptr))
    {
        messages::resourceNotFound(
            SensorsAsyncResp->res, SensorsAsyncResp->chassisSubNode,
            SensorsAsyncResp->chassisSubNode == "Thermal" ? "Temperatures"
                                                          : "Voltages");

        return;
    }
    if (allSensors->empty())
    {
        // Nothing to do, the activeSensors object is also empty
        return;
    }

    for (const char* type : SensorsAsyncResp->types)
    {
        for (const std::string& sensor : *allSensors)
        {
            if (boost::starts_with(sensor, type))
            {
                activeSensors->emplace(sensor);
            }
        }
    }
}

/**
 * @brief Retrieves requested chassis sensors and redundancy data from DBus .
 * @param SensorsAsyncResp   Pointer to object holding response data
 * @param callback  Callback for next step in gathered sensor processing
 */
template <typename Callback>
void getChassis(std::shared_ptr<SensorsAsyncResp> sensorsAsyncResp,
                Callback&& callback)
{
    BMCWEB_LOG_DEBUG << "getChassis enter";
    const std::array<const char*, 3> interfaces = {
        "xyz.openbmc_project.Inventory.Item.Board",
        "xyz.openbmc_project.Inventory.Item.Chassis",
        "xyz.openbmc_project.Inventory.Item.PowerSupply"};
    auto respHandler = [callback{std::move(callback)}, sensorsAsyncResp](
                           const boost::system::error_code ec,
                           const std::vector<std::string>& chassisPaths) {
        BMCWEB_LOG_DEBUG << "getChassis respHandler enter";
        if (ec)
        {
            BMCWEB_LOG_ERROR << "getChassis respHandler DBUS error: " << ec;
            messages::internalError(sensorsAsyncResp->res);
            return;
        }

        const std::string* chassisPath = nullptr;
        std::string chassisName;
        for (const std::string& chassis : chassisPaths)
        {
            std::size_t lastPos = chassis.rfind("/");
            if (lastPos == std::string::npos)
            {
                BMCWEB_LOG_ERROR << "Failed to find '/' in " << chassis;
                continue;
            }
            chassisName = chassis.substr(lastPos + 1);
            if (chassisName == sensorsAsyncResp->chassisId)
            {
                chassisPath = &chassis;
                break;
            }
        }
        if (chassisPath == nullptr)
        {
            messages::resourceNotFound(sensorsAsyncResp->res, "Chassis",
                                       sensorsAsyncResp->chassisId);
            return;
        }

        const std::string& chassisSubNode = sensorsAsyncResp->chassisSubNode;
        if (chassisSubNode == "Power")
        {
            sensorsAsyncResp->res.jsonValue["@odata.type"] =
                "#Power.v1_5_2.Power";
        }
        else if (chassisSubNode == "Thermal")
        {
            sensorsAsyncResp->res.jsonValue["@odata.type"] =
                "#Thermal.v1_4_0.Thermal";
            sensorsAsyncResp->res.jsonValue["Fans"] = nlohmann::json::array();
            sensorsAsyncResp->res.jsonValue["Temperatures"] =
                nlohmann::json::array();
        }
        sensorsAsyncResp->res.jsonValue["@odata.id"] =
            "/redfish/v1/Chassis/" + sensorsAsyncResp->chassisId + "/" +
            chassisSubNode;

        sensorsAsyncResp->res.jsonValue["@odata.context"] =
            "/redfish/v1/$metadata#" + chassisSubNode + "." + chassisSubNode;
        sensorsAsyncResp->res.jsonValue["Id"] = chassisSubNode;
        sensorsAsyncResp->res.jsonValue["Name"] = chassisSubNode;

        // Get the list of sensors for this Chassis element
        std::string sensorPath = *chassisPath + "/sensors";
        crow::connections::systemBus->async_method_call(
            [sensorsAsyncResp, callback{std::move(callback)}](
                const boost::system::error_code ec,
                const std::variant<std::vector<std::string>>&
                    variantEndpoints) {
                if (ec)
                {
                    if (ec.value() != EBADR)
                    {
                        messages::internalError(sensorsAsyncResp->res);
                        return;
                    }
                }
                const std::vector<std::string>* nodeSensorList =
                    std::get_if<std::vector<std::string>>(&(variantEndpoints));
                if (nodeSensorList == nullptr)
                {
                    messages::resourceNotFound(
                        sensorsAsyncResp->res, sensorsAsyncResp->chassisSubNode,
                        sensorsAsyncResp->chassisSubNode == "Thermal"
                            ? "Temperatures"
                            : "Voltages");
                    return;
                }
                const std::shared_ptr<boost::container::flat_set<std::string>>
                    culledSensorList = std::make_shared<
                        boost::container::flat_set<std::string>>();
                reduceSensorList(sensorsAsyncResp, nodeSensorList,
                                 culledSensorList);
                callback(culledSensorList);
            },
            "xyz.openbmc_project.ObjectMapper", sensorPath,
            "org.freedesktop.DBus.Properties", "Get",
            "xyz.openbmc_project.Association", "endpoints");
    };

    // Get the Chassis Collection
    crow::connections::systemBus->async_method_call(
        respHandler, "xyz.openbmc_project.ObjectMapper",
        "/xyz/openbmc_project/object_mapper",
        "xyz.openbmc_project.ObjectMapper", "GetSubTreePaths",
        "/xyz/openbmc_project/inventory", int32_t(0), interfaces);
    BMCWEB_LOG_DEBUG << "getChassis exit";
}

/**
 * @brief Finds all DBus object paths that implement ObjectManager.
 *
 * Creates a mapping from the associated connection name to the object path.
 *
 * Finds the object paths asynchronously.  Invokes callback when information has
 * been obtained.
 *
 * The callback must have the following signature:
 *   @code
 *   callback(const boost::container::flat_map<std::string,
 *            std::string>& objectMgrPaths)
 *   @endcode
 *
 * @param sensorsAsyncResp Pointer to object holding response data.
 * @param callback Callback to invoke when object paths obtained.
 */
template <typename Callback>
void getObjectManagerPaths(std::shared_ptr<SensorsAsyncResp> SensorsAsyncResp,
                           Callback&& callback)
{
    BMCWEB_LOG_DEBUG << "getObjectManagerPaths enter";
    const std::array<std::string, 1> interfaces = {
        "org.freedesktop.DBus.ObjectManager"};

    // Response handler for GetSubTree DBus method
    auto respHandler = [callback{std::move(callback)},
                        SensorsAsyncResp](const boost::system::error_code ec,
                                          const GetSubTreeType& subtree) {
        BMCWEB_LOG_DEBUG << "getObjectManagerPaths respHandler enter";
        if (ec)
        {
            messages::internalError(SensorsAsyncResp->res);
            BMCWEB_LOG_ERROR << "getObjectManagerPaths respHandler: DBus error "
                             << ec;
            return;
        }

        // Loop over returned object paths
        boost::container::flat_map<std::string, std::string> objectMgrPaths;
        for (const std::pair<
                 std::string,
                 std::vector<std::pair<std::string, std::vector<std::string>>>>&
                 object : subtree)
        {
            // Loop over connections for current object path
            const std::string& objectPath = object.first;
            for (const std::pair<std::string, std::vector<std::string>>&
                     objData : object.second)
            {
                // Add mapping from connection to object path
                const std::string& connection = objData.first;
                objectMgrPaths[connection] = objectPath;
                BMCWEB_LOG_DEBUG << "Added mapping " << connection << " -> "
                                 << objectPath;
            }
        }
        callback(std::move(objectMgrPaths));
        BMCWEB_LOG_DEBUG << "getObjectManagerPaths respHandler exit";
    };

    // Query mapper for all DBus object paths that implement ObjectManager
    crow::connections::systemBus->async_method_call(
        std::move(respHandler), "xyz.openbmc_project.ObjectMapper",
        "/xyz/openbmc_project/object_mapper",
        "xyz.openbmc_project.ObjectMapper", "GetSubTree", "/", int32_t(0),
        interfaces);
    BMCWEB_LOG_DEBUG << "getObjectManagerPaths exit";
}

/**
 * @brief Retrieves the health from a sensor .
 * @param interfacesDict   Map of all sensor interfaces
 */

static std::string getHealth(
    const boost::container::flat_map<
        std::string, boost::container::flat_map<std::string, SensorVariant>>&
        interfacesDict)
{
    auto criticalThresholdIt =
        interfacesDict.find("xyz.openbmc_project.Sensor.Threshold.Critical");
    if (criticalThresholdIt != interfacesDict.end())
    {
        auto thresholdHighIt =
            criticalThresholdIt->second.find("CriticalAlarmHigh");
        auto thresholdLowIt =
            criticalThresholdIt->second.find("CriticalAlarmLow");
        if (thresholdHighIt != criticalThresholdIt->second.end())
        {
            const bool* asserted = std::get_if<bool>(&thresholdHighIt->second);
            if (asserted == nullptr)
            {
                BMCWEB_LOG_ERROR << "Illegal sensor threshold";
            }
            else if (*asserted)
            {
                return "Critical";
            }
        }
        if (thresholdLowIt != criticalThresholdIt->second.end())
        {
            const bool* asserted = std::get_if<bool>(&thresholdLowIt->second);
            if (asserted == nullptr)
            {
                BMCWEB_LOG_ERROR << "Illegal sensor threshold";
            }
            else if (*asserted)
            {
                return "Critical";
            }
        }
    }

    auto warningThresholdIt =
        interfacesDict.find("xyz.openbmc_project.Sensor.Threshold.Warning");
    if (warningThresholdIt != interfacesDict.end())
    {
        auto thresholdHighIt =
            warningThresholdIt->second.find("WarningAlarmHigh");
        auto thresholdLowIt =
            warningThresholdIt->second.find("WarningAlarmLow");
        if (thresholdHighIt != warningThresholdIt->second.end())
        {
            const bool* asserted = std::get_if<bool>(&thresholdHighIt->second);
            if (asserted == nullptr)
            {
                BMCWEB_LOG_ERROR << "Illegal sensor threshold";
            }
            else if (*asserted)
            {
                return "Warning";
            }
        }
        if (thresholdLowIt != warningThresholdIt->second.end())
        {
            const bool* asserted = std::get_if<bool>(&thresholdLowIt->second);
            if (asserted == nullptr)
            {
                BMCWEB_LOG_ERROR << "Illegal sensor threshold";
            }
            else if (*asserted)
            {
                return "Warning";
            }
        }
    }
    return "OK";
}

/**
 * @brief Builds a json sensor representation of a sensor.
 * @param sensorName  The name of the sensor to be built
 * @param sensorType  The type (temperature, fan_tach, etc) of the sensor to
 * build
 * @param interfacesDict  A dictionary of the interfaces and properties of said
 * interfaces to be built from
 * @param sensor_json  The json object to fill
 */
void objectInterfacesToJson(
    const std::string& sensorName, const std::string& sensorType,
    const boost::container::flat_map<
        std::string, boost::container::flat_map<std::string, SensorVariant>>&
        interfacesDict,
    nlohmann::json& sensor_json)
{
    // We need a value interface before we can do anything with it
    auto valueIt = interfacesDict.find("xyz.openbmc_project.Sensor.Value");
    if (valueIt == interfacesDict.end())
    {
        BMCWEB_LOG_ERROR << "Sensor doesn't have a value interface";
        return;
    }

    // Assume values exist as is (10^0 == 1) if no scale exists
    int64_t scaleMultiplier = 0;

    auto scaleIt = valueIt->second.find("Scale");
    // If a scale exists, pull value as int64, and use the scaling.
    if (scaleIt != valueIt->second.end())
    {
        const int64_t* int64Value = std::get_if<int64_t>(&scaleIt->second);
        if (int64Value != nullptr)
        {
            scaleMultiplier = *int64Value;
        }
    }

    sensor_json["MemberId"] = sensorName;
    sensor_json["Name"] = boost::replace_all_copy(sensorName, "_", " ");

    sensor_json["Status"]["State"] = "Enabled";
    sensor_json["Status"]["Health"] = getHealth(interfacesDict);

    // Parameter to set to override the type we get from dbus, and force it to
    // int, regardless of what is available.  This is used for schemas like fan,
    // that require integers, not floats.
    bool forceToInt = false;

    const char* unit = "Reading";
    if (sensorType == "temperature")
    {
        unit = "ReadingCelsius";
        sensor_json["@odata.type"] = "#Thermal.v1_3_0.Temperature";
        // TODO(ed) Documentation says that path should be type fan_tach,
        // implementation seems to implement fan
    }
    else if (sensorType == "fan" || sensorType == "fan_tach")
    {
        unit = "Reading";
        sensor_json["ReadingUnits"] = "RPM";
        sensor_json["@odata.type"] = "#Thermal.v1_3_0.Fan";
        forceToInt = true;
    }
    else if (sensorType == "fan_pwm")
    {
        unit = "Reading";
        sensor_json["ReadingUnits"] = "Percent";
        sensor_json["@odata.type"] = "#Thermal.v1_3_0.Fan";
        forceToInt = true;
    }
    else if (sensorType == "voltage")
    {
        unit = "ReadingVolts";
        sensor_json["@odata.type"] = "#Power.v1_0_0.Voltage";
    }
    else if (sensorType == "power")
    {
        std::string sensorNameLower =
            boost::algorithm::to_lower_copy(sensorName);

        if (!sensorName.compare("total_power"))
        {
            unit = "PowerConsumedWatts";
        }
        else if (sensorNameLower.find("input") != std::string::npos)
        {
            unit = "PowerInputWatts";
        }
        else
        {
            unit = "PowerOutputWatts";
        }
    }
    else
    {
        BMCWEB_LOG_ERROR << "Redfish cannot map object type for " << sensorName;
        return;
    }
    // Map of dbus interface name, dbus property name and redfish property_name
    std::vector<std::tuple<const char*, const char*, const char*>> properties;
    properties.reserve(7);

    properties.emplace_back("xyz.openbmc_project.Sensor.Value", "Value", unit);

    // If sensor type doesn't map to Redfish PowerSupply, add threshold props
    if ((sensorType != "current") && (sensorType != "power"))
    {
        properties.emplace_back("xyz.openbmc_project.Sensor.Threshold.Warning",
                                "WarningHigh", "UpperThresholdNonCritical");
        properties.emplace_back("xyz.openbmc_project.Sensor.Threshold.Warning",
                                "WarningLow", "LowerThresholdNonCritical");
        properties.emplace_back("xyz.openbmc_project.Sensor.Threshold.Critical",
                                "CriticalHigh", "UpperThresholdCritical");
        properties.emplace_back("xyz.openbmc_project.Sensor.Threshold.Critical",
                                "CriticalLow", "LowerThresholdCritical");
    }

    // TODO Need to get UpperThresholdFatal and LowerThresholdFatal

    if (sensorType == "temperature")
    {
        properties.emplace_back("xyz.openbmc_project.Sensor.Value", "MinValue",
                                "MinReadingRangeTemp");
        properties.emplace_back("xyz.openbmc_project.Sensor.Value", "MaxValue",
                                "MaxReadingRangeTemp");
    }
    else if ((sensorType != "current") && (sensorType != "power"))
    {
        // Sensor type doesn't map to Redfish PowerSupply; add min/max props
        properties.emplace_back("xyz.openbmc_project.Sensor.Value", "MinValue",
                                "MinReadingRange");
        properties.emplace_back("xyz.openbmc_project.Sensor.Value", "MaxValue",
                                "MaxReadingRange");
    }

    for (const std::tuple<const char*, const char*, const char*>& p :
         properties)
    {
        auto interfaceProperties = interfacesDict.find(std::get<0>(p));
        if (interfaceProperties != interfacesDict.end())
        {
            auto valueIt = interfaceProperties->second.find(std::get<1>(p));
            if (valueIt != interfaceProperties->second.end())
            {
                const SensorVariant& valueVariant = valueIt->second;
                nlohmann::json& valueIt = sensor_json[std::get<2>(p)];
                // Attempt to pull the int64 directly
                const int64_t* int64Value = std::get_if<int64_t>(&valueVariant);

                const double* doubleValue = std::get_if<double>(&valueVariant);
                const uint32_t* uValue = std::get_if<uint32_t>(&valueVariant);
                double temp = 0.0;
                if (int64Value != nullptr)
                {
                    temp = *int64Value;
                }
                else if (doubleValue != nullptr)
                {
                    temp = *doubleValue;
                }
                else if (uValue != nullptr)
                {
                    temp = *uValue;
                }
                else
                {
                    BMCWEB_LOG_ERROR
                        << "Got value interface that wasn't int or double";
                    continue;
                }
                temp = temp * std::pow(10, scaleMultiplier);
                if (forceToInt)
                {
                    valueIt = static_cast<int64_t>(temp);
                }
                else
                {
                    valueIt = temp;
                }
            }
        }
    }
    BMCWEB_LOG_DEBUG << "Added sensor " << sensorName;
}

static void
    populateFanRedundancy(std::shared_ptr<SensorsAsyncResp> sensorsAsyncResp)
{
    crow::connections::systemBus->async_method_call(
        [sensorsAsyncResp](const boost::system::error_code ec,
                           const GetSubTreeType& resp) {
            if (ec)
            {
                return; // don't have to have this interface
            }
            for (const std::pair<std::string,
                                 std::vector<std::pair<
                                     std::string, std::vector<std::string>>>>&
                     pathPair : resp)
            {
                const std::string& path = pathPair.first;
                const std::vector<
                    std::pair<std::string, std::vector<std::string>>>& objDict =
                    pathPair.second;
                if (objDict.empty())
                {
                    continue; // this should be impossible
                }

                const std::string& owner = objDict.begin()->first;
                crow::connections::systemBus->async_method_call(
                    [path, owner,
                     sensorsAsyncResp](const boost::system::error_code ec,
                                       std::variant<std::vector<std::string>>
                                           variantEndpoints) {
                        if (ec)
                        {
                            return; // if they don't have an association we
                                    // can't tell what chassis is
                        }
                        // verify part of the right chassis
                        auto endpoints = std::get_if<std::vector<std::string>>(
                            &variantEndpoints);

                        if (endpoints == nullptr)
                        {
                            BMCWEB_LOG_ERROR << "Invalid association interface";
                            messages::internalError(sensorsAsyncResp->res);
                            return;
                        }

                        auto found = std::find_if(
                            endpoints->begin(), endpoints->end(),
                            [sensorsAsyncResp](const std::string& entry) {
                                return entry.find(
                                           sensorsAsyncResp->chassisId) !=
                                       std::string::npos;
                            });

                        if (found == endpoints->end())
                        {
                            return;
                        }
                        crow::connections::systemBus->async_method_call(
                            [path, sensorsAsyncResp](
                                const boost::system::error_code ec,
                                const boost::container::flat_map<
                                    std::string,
                                    std::variant<uint8_t,
                                                 std::vector<std::string>,
                                                 std::string>>& ret) {
                                if (ec)
                                {
                                    return; // don't have to have this
                                            // interface
                                }
                                auto findFailures = ret.find("AllowedFailures");
                                auto findCollection = ret.find("Collection");
                                auto findStatus = ret.find("Status");

                                if (findFailures == ret.end() ||
                                    findCollection == ret.end() ||
                                    findStatus == ret.end())
                                {
                                    BMCWEB_LOG_ERROR
                                        << "Invalid redundancy interface";
                                    messages::internalError(
                                        sensorsAsyncResp->res);
                                    return;
                                }

                                auto allowedFailures = std::get_if<uint8_t>(
                                    &(findFailures->second));
                                auto collection =
                                    std::get_if<std::vector<std::string>>(
                                        &(findCollection->second));
                                auto status = std::get_if<std::string>(
                                    &(findStatus->second));

                                if (allowedFailures == nullptr ||
                                    collection == nullptr || status == nullptr)
                                {

                                    BMCWEB_LOG_ERROR
                                        << "Invalid redundancy interface "
                                           "types";
                                    messages::internalError(
                                        sensorsAsyncResp->res);
                                    return;
                                }
                                size_t lastSlash = path.rfind("/");
                                if (lastSlash == std::string::npos)
                                {
                                    // this should be impossible
                                    messages::internalError(
                                        sensorsAsyncResp->res);
                                    return;
                                }
                                std::string name = path.substr(lastSlash + 1);
                                std::replace(name.begin(), name.end(), '_',
                                             ' ');

                                std::string health;

                                if (boost::ends_with(*status, "Full"))
                                {
                                    health = "OK";
                                }
                                else if (boost::ends_with(*status, "Degraded"))
                                {
                                    health = "Warning";
                                }
                                else
                                {
                                    health = "Critical";
                                }
                                std::vector<nlohmann::json> redfishCollection;
                                const auto& fanRedfish =
                                    sensorsAsyncResp->res.jsonValue["Fans"];
                                for (const std::string& item : *collection)
                                {
                                    lastSlash = item.rfind("/");
                                    // make a copy as collection is const
                                    std::string itemName =
                                        item.substr(lastSlash + 1);
                                    /*
                                    todo(ed): merge patch that fixes the names
                                    std::replace(itemName.begin(),
                                                 itemName.end(), '_', ' ');*/
                                    auto schemaItem = std::find_if(
                                        fanRedfish.begin(), fanRedfish.end(),
                                        [itemName](const nlohmann::json& fan) {
                                            return fan["MemberId"] == itemName;
                                        });
                                    if (schemaItem != fanRedfish.end())
                                    {
                                        redfishCollection.push_back(
                                            {{"@odata.id",
                                              (*schemaItem)["@odata.id"]}});
                                    }
                                    else
                                    {
                                        BMCWEB_LOG_ERROR
                                            << "failed to find fan in schema";
                                        messages::internalError(
                                            sensorsAsyncResp->res);
                                        return;
                                    }
                                }

                                auto& resp = sensorsAsyncResp->res
                                                 .jsonValue["Redundancy"];
                                resp.push_back(
                                    {{"@odata.id",
                                      "/refish/v1/Chassis/" +
                                          sensorsAsyncResp->chassisId + "/" +
                                          sensorsAsyncResp->chassisSubNode +
                                          "#/Redundancy/" +
                                          std::to_string(resp.size())},
                                     {"@odata.type",
                                      "#Redundancy.v1_3_2.Redundancy"},
                                     {"MinNumNeeded",
                                      collection->size() - *allowedFailures},
                                     {"MemberId", name},
                                     {"Mode", "N+m"},
                                     {"Name", name},
                                     {"RedundancySet", redfishCollection},
                                     {"Status",
                                      {{"Health", health},
                                       {"State", "Enabled"}}}});
                            },
                            owner, path, "org.freedesktop.DBus.Properties",
                            "GetAll",
                            "xyz.openbmc_project.Control.FanRedundancy");
                    },
                    "xyz.openbmc_project.ObjectMapper", path + "/inventory",
                    "org.freedesktop.DBus.Properties", "Get",
                    "xyz.openbmc_project.Association", "endpoints");
            }
        },
        "xyz.openbmc_project.ObjectMapper",
        "/xyz/openbmc_project/object_mapper",
        "xyz.openbmc_project.ObjectMapper", "GetSubTree",
        "/xyz/openbmc_project/control", 2,
        std::array<const char*, 1>{
            "xyz.openbmc_project.Control.FanRedundancy"});
}

void sortJSONResponse(std::shared_ptr<SensorsAsyncResp> SensorsAsyncResp)
{
    nlohmann::json& response = SensorsAsyncResp->res.jsonValue;
    std::array<std::string, 2> sensorHeaders{"Temperatures", "Fans"};
    if (SensorsAsyncResp->chassisSubNode == "Power")
    {
        sensorHeaders = {"Voltages", "PowerSupplies"};
    }
    for (const std::string& sensorGroup : sensorHeaders)
    {
        nlohmann::json::iterator entry = response.find(sensorGroup);
        if (entry != response.end())
        {
            std::sort(entry->begin(), entry->end(),
                      [](nlohmann::json& c1, nlohmann::json& c2) {
                          return c1["Name"] < c2["Name"];
                      });

            // add the index counts to the end of each entry
            size_t count = 0;
            for (nlohmann::json& sensorJson : *entry)
            {
                nlohmann::json::iterator odata = sensorJson.find("@odata.id");
                if (odata == sensorJson.end())
                {
                    continue;
                }
                std::string* value = odata->get_ptr<std::string*>();
                if (value != nullptr)
                {
                    *value += std::to_string(count);
                    count++;
                }
            }
        }
    }
}

/**
 * @brief Gets the values of the specified sensors.
 *
 * Stores the results as JSON in the SensorsAsyncResp.
 *
 * Gets the sensor values asynchronously.  Stores the results later when the
 * information has been obtained.
 *
 * The sensorNames set contains all sensors for the current chassis.
 * SensorsAsyncResp contains the requested sensor types.  Only sensors of a
 * requested type are included in the JSON output.
 *
 * To minimize the number of DBus calls, the DBus method
 * org.freedesktop.DBus.ObjectManager.GetManagedObjects() is used to get the
 * values of all sensors provided by a connection (service).
 *
 * The connections set contains all the connections that provide sensor values.
 *
 * The objectMgrPaths map contains mappings from a connection name to the
 * corresponding DBus object path that implements ObjectManager.
 *
 * @param SensorsAsyncResp Pointer to object holding response data.
 * @param sensorNames All sensors within the current chassis.
 * @param connections Connections that provide sensor values.
 * @param objectMgrPaths Mappings from connection name to DBus object path that
 * implements ObjectManager.
 */
void getSensorData(
    std::shared_ptr<SensorsAsyncResp> SensorsAsyncResp,
    const std::shared_ptr<boost::container::flat_set<std::string>> sensorNames,
    const boost::container::flat_set<std::string>& connections,
    const boost::container::flat_map<std::string, std::string>& objectMgrPaths)
{
    BMCWEB_LOG_DEBUG << "getSensorData enter";
    // Get managed objects from all services exposing sensors
    for (const std::string& connection : connections)
    {
        // Response handler to process managed objects
        auto getManagedObjectsCb = [SensorsAsyncResp, sensorNames](
                                       const boost::system::error_code ec,
                                       ManagedObjectsVectorType& resp) {
            BMCWEB_LOG_DEBUG << "getManagedObjectsCb enter";
            if (ec)
            {
                BMCWEB_LOG_ERROR << "getManagedObjectsCb DBUS error: " << ec;
                messages::internalError(SensorsAsyncResp->res);
                return;
            }
            // Go through all objects and update response with sensor data
            for (const auto& objDictEntry : resp)
            {
                const std::string& objPath =
                    static_cast<const std::string&>(objDictEntry.first);
                BMCWEB_LOG_DEBUG << "getManagedObjectsCb parsing object "
                                 << objPath;

                std::vector<std::string> split;
                // Reserve space for
                // /xyz/openbmc_project/sensors/<name>/<subname>
                split.reserve(6);
                boost::algorithm::split(split, objPath, boost::is_any_of("/"));
                if (split.size() < 6)
                {
                    BMCWEB_LOG_ERROR << "Got path that isn't long enough "
                                     << objPath;
                    continue;
                }
                // These indexes aren't intuitive, as boost::split puts an empty
                // string at the beginning
                const std::string& sensorType = split[4];
                const std::string& sensorName = split[5];
                BMCWEB_LOG_DEBUG << "sensorName " << sensorName
                                 << " sensorType " << sensorType;
                if (sensorNames->find(objPath) == sensorNames->end())
                {
                    BMCWEB_LOG_ERROR << sensorName << " not in sensor list ";
                    continue;
                }

                const char* fieldName = nullptr;
                if (sensorType == "temperature")
                {
                    fieldName = "Temperatures";
                }
                else if (sensorType == "fan" || sensorType == "fan_tach" ||
                         sensorType == "fan_pwm")
                {
                    fieldName = "Fans";
                }
                else if (sensorType == "voltage")
                {
                    fieldName = "Voltages";
                }
                else if (sensorType == "current")
                {
                    fieldName = "PowerSupplies";
                }
                else if (sensorType == "power")
                {
                    if (!sensorName.compare("total_power"))
                    {
                        fieldName = "PowerControl";
                    }
                    else
                    {
                        fieldName = "PowerSupplies";
                    }
                }
                else
                {
                    BMCWEB_LOG_ERROR << "Unsure how to handle sensorType "
                                     << sensorType;
                    continue;
                }

                nlohmann::json& tempArray =
                    SensorsAsyncResp->res.jsonValue[fieldName];

                if (fieldName == "PowerSupplies" && !tempArray.empty())
                {
                    // Power supplies put multiple "sensors" into a single power
                    // supply entry, so only create the first one
                }
                else
                {
                    tempArray.push_back(
                        {{"@odata.id", "/redfish/v1/Chassis/" +
                                           SensorsAsyncResp->chassisId + "/" +
                                           SensorsAsyncResp->chassisSubNode +
                                           "#/" + fieldName + "/"}});
                }
                nlohmann::json& sensorJson = tempArray.back();

                objectInterfacesToJson(sensorName, sensorType,
                                       objDictEntry.second, sensorJson);
            }
            if (SensorsAsyncResp.use_count() == 1)
            {
                sortJSONResponse(SensorsAsyncResp);
                if (SensorsAsyncResp->chassisSubNode == "Thermal")
                {
                    populateFanRedundancy(SensorsAsyncResp);
                }
            }
            BMCWEB_LOG_DEBUG << "getManagedObjectsCb exit";
        };

        // Find DBus object path that implements ObjectManager for the current
        // connection.  If no mapping found, default to "/".
        auto iter = objectMgrPaths.find(connection);
        const std::string& objectMgrPath =
            (iter != objectMgrPaths.end()) ? iter->second : "/";
        BMCWEB_LOG_DEBUG << "ObjectManager path for " << connection << " is "
                         << objectMgrPath;

        crow::connections::systemBus->async_method_call(
            getManagedObjectsCb, connection, objectMgrPath,
            "org.freedesktop.DBus.ObjectManager", "GetManagedObjects");
    };
    BMCWEB_LOG_DEBUG << "getSensorData exit";
}

/**
 * @brief Entry point for retrieving sensors data related to requested
 *        chassis.
 * @param SensorsAsyncResp   Pointer to object holding response data
 */
void getChassisData(std::shared_ptr<SensorsAsyncResp> SensorsAsyncResp)
{
    BMCWEB_LOG_DEBUG << "getChassisData enter";
    auto getChassisCb =
        [SensorsAsyncResp](
            std::shared_ptr<boost::container::flat_set<std::string>>
                sensorNames) {
            BMCWEB_LOG_DEBUG << "getChassisCb enter";
            auto getConnectionCb =
                [SensorsAsyncResp,
                 sensorNames](const boost::container::flat_set<std::string>&
                                  connections) {
                    BMCWEB_LOG_DEBUG << "getConnectionCb enter";
                    auto getObjectManagerPathsCb =
                        [SensorsAsyncResp, sensorNames, connections](
                            const boost::container::flat_map<
                                std::string, std::string>& objectMgrPaths) {
                            BMCWEB_LOG_DEBUG << "getObjectManagerPathsCb enter";
                            // Get sensor data and store results in JSON
                            // response
                            getSensorData(SensorsAsyncResp, sensorNames,
                                          connections, objectMgrPaths);
                            BMCWEB_LOG_DEBUG << "getObjectManagerPathsCb exit";
                        };

                    // Get mapping from connection names to the DBus object
                    // paths that implement the ObjectManager interface
                    getObjectManagerPaths(SensorsAsyncResp,
                                          std::move(getObjectManagerPathsCb));
                    BMCWEB_LOG_DEBUG << "getConnectionCb exit";
                };

            // Get set of connections that provide sensor values
            getConnections(SensorsAsyncResp, sensorNames,
                           std::move(getConnectionCb));
            BMCWEB_LOG_DEBUG << "getChassisCb exit";
        };
    SensorsAsyncResp->res.jsonValue["Redundancy"] = nlohmann::json::array();

    // Get set of sensors in chassis
    getChassis(SensorsAsyncResp, std::move(getChassisCb));
    BMCWEB_LOG_DEBUG << "getChassisData exit";
};

/**
 * @brief Find the requested sensorName in the list of all sensors supplied by
 * the chassis node
 *
 * @param sensorName   The sensor name supplied in the PATCH request
 * @param sensorsList  The list of sensors managed by the chassis node
 * @param sensorsModified  The list of sensors that were found as a result of
 *                         repeated calls to this function
 */
bool findSensorNameUsingSensorPath(
    std::string_view sensorName,
    boost::container::flat_set<std::string>& sensorsList,
    boost::container::flat_set<std::string>& sensorsModified)
{
    for (std::string_view chassisSensor : sensorsList)
    {
        std::size_t pos = chassisSensor.rfind("/");
        if (pos >= (chassisSensor.size() - 1))
        {
            continue;
        }
        std::string_view thisSensorName = chassisSensor.substr(pos + 1);
        if (thisSensorName == sensorName)
        {
            sensorsModified.emplace(chassisSensor);
            return true;
        }
    }
    return false;
}

/**
 * @brief Entry point for overriding sensor values of given sensor
 *
 * @param res   response object
 * @param req   request object
 * @param params   parameter passed for CRUD
 * @param typeList   TypeList of sensors for the resource queried
 * @param chassisSubNode   Chassis Node for which the query has to happen
 */
void setSensorOverride(crow::Response& res, const crow::Request& req,
                       const std::vector<std::string>& params,
                       const std::initializer_list<const char*> typeList,
                       const std::string& chassisSubNode)
{

    // TODO: Need to figure out dynamic way to restrict patch (Set Sensor
    // override) based on another d-bus announcement to be more generic.
    if (params.size() != 1)
    {
        messages::internalError(res);
        res.end();
        return;
    }

    std::unordered_map<std::string, std::vector<nlohmann::json>> allCollections;
    std::optional<std::vector<nlohmann::json>> temperatureCollections;
    std::optional<std::vector<nlohmann::json>> fanCollections;
    std::vector<nlohmann::json> voltageCollections;
    BMCWEB_LOG_INFO << "setSensorOverride for subNode" << chassisSubNode
                    << "\n";

    if (chassisSubNode == "Thermal")
    {
        if (!json_util::readJson(req, res, "Temperatures",
                                 temperatureCollections, "Fans",
                                 fanCollections))
        {
            return;
        }
        if (!temperatureCollections && !fanCollections)
        {
            messages::resourceNotFound(res, "Thermal",
                                       "Temperatures / Voltages");
            res.end();
            return;
        }
        if (temperatureCollections)
        {
            allCollections.emplace("Temperatures",
                                   *std::move(temperatureCollections));
        }
        if (fanCollections)
        {
            allCollections.emplace("Fans", *std::move(fanCollections));
        }
    }
    else if (chassisSubNode == "Power")
    {
        if (!json_util::readJson(req, res, "Voltages", voltageCollections))
        {
            return;
        }
        allCollections.emplace("Voltages", std::move(voltageCollections));
    }
    else
    {
        res.result(boost::beast::http::status::not_found);
        res.end();
        return;
    }

    const char* propertyValueName;
    std::unordered_map<std::string, std::pair<double, std::string>> overrideMap;
    std::string memberId;
    double value;
    for (auto& collectionItems : allCollections)
    {
        if (collectionItems.first == "Temperatures")
        {
            propertyValueName = "ReadingCelsius";
        }
        else if (collectionItems.first == "Fans")
        {
            propertyValueName = "Reading";
        }
        else
        {
            propertyValueName = "ReadingVolts";
        }
        for (auto& item : collectionItems.second)
        {
            if (!json_util::readJson(item, res, "MemberId", memberId,
                                     propertyValueName, value))
            {
                return;
            }
            overrideMap.emplace(memberId,
                                std::make_pair(value, collectionItems.first));
        }
    }
    const std::string& chassisName = params[0];
    auto sensorAsyncResp = std::make_shared<SensorsAsyncResp>(
        res, chassisName, typeList, chassisSubNode);
    auto getChassisSensorListCb = [sensorAsyncResp,
                                   overrideMap](const std::shared_ptr<
                                                boost::container::flat_set<
                                                    std::string>>
                                                    sensorsList) {
        // Match sensor names in the PATCH request to those managed by the
        // chassis node
        const std::shared_ptr<boost::container::flat_set<std::string>>
            sensorNames =
                std::make_shared<boost::container::flat_set<std::string>>();
        for (const auto& item : overrideMap)
        {
            const auto& sensor = item.first;
            if (!findSensorNameUsingSensorPath(sensor, *sensorsList,
                                               *sensorNames))
            {
                BMCWEB_LOG_INFO << "Unable to find memberId " << item.first;
                messages::resourceNotFound(sensorAsyncResp->res,
                                           item.second.second, item.first);
                return;
            }
        }
        // Get the connection to which the memberId belongs
        auto getObjectsWithConnectionCb =
            [sensorAsyncResp, overrideMap](
                const boost::container::flat_set<std::string>& connections,
                const std::set<std::pair<std::string, std::string>>&
                    objectsWithConnection) {
                if (objectsWithConnection.size() != overrideMap.size())
                {
                    BMCWEB_LOG_INFO
                        << "Unable to find all objects with proper connection "
                        << objectsWithConnection.size() << " requested "
                        << overrideMap.size() << "\n";
                    messages::resourceNotFound(
                        sensorAsyncResp->res,
                        sensorAsyncResp->chassisSubNode == "Thermal"
                            ? "Temperatures"
                            : "Voltages",
                        "Count");
                    return;
                }
                for (const auto& item : objectsWithConnection)
                {

                    auto lastPos = item.first.rfind('/');
                    if (lastPos == std::string::npos)
                    {
                        messages::internalError(sensorAsyncResp->res);
                        return;
                    }
                    std::string sensorName = item.first.substr(lastPos + 1);

                    const auto& iterator = overrideMap.find(sensorName);
                    if (iterator == overrideMap.end())
                    {
                        BMCWEB_LOG_INFO << "Unable to find sensor object"
                                        << item.first << "\n";
                        messages::internalError(sensorAsyncResp->res);
                        return;
                    }
                    crow::connections::systemBus->async_method_call(
                        [sensorAsyncResp](const boost::system::error_code ec) {
                            if (ec)
                            {
                                BMCWEB_LOG_DEBUG
                                    << "setOverrideValueStatus DBUS error: "
                                    << ec;
                                messages::internalError(sensorAsyncResp->res);
                                return;
                            }
                        },
                        item.second, item.first,
                        "org.freedesktop.DBus.Properties", "Set",
                        "xyz.openbmc_project.Sensor.Value", "Value",
                        sdbusplus::message::variant<double>(
                            iterator->second.first));
                }
            };
        // Get object with connection for the given sensor name
        getObjectsWithConnection(sensorAsyncResp, sensorNames,
                                 std::move(getObjectsWithConnectionCb));
    };
    // get full sensor list for the given chassisId and cross verify the sensor.
    getChassis(sensorAsyncResp, std::move(getChassisSensorListCb));
}

} // namespace redfish
