/*
// Copyright (c) 2018 Intel Corporation
// Copyright (c) 2018 Ampere Computing LLC
/
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
#include "sensors.hpp"

namespace redfish
{

using GetSubTreeType = std::vector<
    std::pair<std::string,
              std::vector<std::pair<std::string, std::vector<std::string>>>>>;

class Power : public Node
{
  public:
    Power(CrowApp& app) :
        Node((app),
#ifdef BMCWEB_ENABLE_REDFISH_ONE_CHASSIS
             "/redfish/v1/Chassis/chassis/Power/")
#else
             "/redfish/v1/Chassis/<str>/Power/", std::string())
#endif
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
    void doGet(crow::Response& res, const crow::Request& req,
               const std::vector<std::string>& params) override
    {

#ifdef BMCWEB_ENABLE_REDFISH_ONE_CHASSIS
        const std::string chassis_name = "chassis";
#else
        if (params.size() != 1)
        {
            res.result(boost::beast::http::status::internal_server_error);
            res.end();
            return;
        }
        const std::string& chassis_name = params[0];
#endif

        res.jsonValue["@odata.id"] =
            "/redfish/v1/Chassis/" + chassis_name + "/Power";
        res.jsonValue["@odata.type"] = "#Power.v1_2_1.Power";
        res.jsonValue["@odata.context"] = "/redfish/v1/$metadata#Power.Power";
        res.jsonValue["Id"] = "Power";
        res.jsonValue["Name"] = "Power";
#ifdef BMCWEB_ENABLE_REDFISH_ONE_CHASSIS
        // TODO: Get all sensors
        crow::connections::systemBus->async_method_call(
            [](boost::system::error_code ec, GetSubTreeType& subtree) {
                if (ec)
                {
                    BMCWEB_LOG_ERROR << "error with async_method_call\n";
                    return;
                }
                for (auto& item : subtree)
                {
                    BMCWEB_LOG_DEBUG << item.first << "\n";
                }
             },
             "xyz.openbmc_project.ObjectMapper",
             "/xyz/openbmc_project/object_mapper",
             "xyz.openbmc_project.ObjectMapper", "GetSubTree",
             "/org/openbmc/sensors/", 2, std::vector<std::string>());

        res.end();
#else
        auto sensorAsyncResp = std::make_shared<SensorsAsyncResp>(
            res, chassis_name,
            std::initializer_list<const char*>{
                "/xyz/openbmc_project/sensors/voltage",
                "/xyz/openbmc_project/sensors/power"},
            "Power");
        // TODO Need to retrieve Power Control information.
        getChassisData(sensorAsyncResp);
#endif
    }
};

} // namespace redfish
