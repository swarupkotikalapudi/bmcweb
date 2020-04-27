/*
// Copyright (c) 2018-2020 Intel Corporation
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
#include "utils/telemetry_utils.hpp"
#include "utils/time_utils.hpp"

namespace redfish
{

class MetricReportDefinitionCollection : public Node
{
  public:
    MetricReportDefinitionCollection(App& app) :
        Node(app, "/redfish/v1/TelemetryService/MetricReportDefinitions/")
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
    void doGet(crow::Response& res, const crow::Request&,
               const std::vector<std::string>&) override
    {
        res.jsonValue["@odata.type"] = "#MetricReportDefinitionCollection."
                                       "MetricReportDefinitionCollection";
        res.jsonValue["@odata.id"] =
            "/redfish/v1/TelemetryService/MetricReportDefinitions";
        res.jsonValue["Name"] = "Metric Definition Collection";

        auto asyncResp = std::make_shared<AsyncResp>(res);
        telemetry::getReportCollection(asyncResp,
                                       telemetry::metricReportDefinitionUri);
    }
};

class MetricReportDefinition : public Node
{
  public:
    MetricReportDefinition(App& app) :
        Node(app, "/redfish/v1/TelemetryService/MetricReportDefinitions/<str>/",
             std::string())
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
    void doGet(crow::Response& res, const crow::Request&,
               const std::vector<std::string>& params) override
    {
        auto asyncResp = std::make_shared<AsyncResp>(res);

        if (params.size() != 1)
        {
            messages::internalError(asyncResp->res);
            return;
        }

        const std::string& id = params[0];
        crow::connections::systemBus->async_method_call(
            [asyncResp,
             id](const boost::system::error_code ec,
                 const std::vector<std::pair<
                     std::string,
                     std::variant<std::vector<std::string>, ReadingParameters,
                                  std::string, uint32_t>>>& ret) {
                if (ec == boost::system::errc::no_such_file_or_directory)
                {
                    messages::resourceNotFound(asyncResp->res, schemaType, id);
                    return;
                }
                if (ec)
                {
                    BMCWEB_LOG_ERROR << "respHandler DBus error " << ec;
                    messages::internalError(asyncResp->res);
                    return;
                }

                fillReportDefinition(asyncResp, id, ret);
            },
            telemetry::service, telemetry::getDbusReportPath(id),
            "org.freedesktop.DBus.Properties", "GetAll",
            telemetry::reportInterface);
    }

    static bool toRedfishReportActions(const std::vector<std::string>& actions,
                                       std::vector<std::string>& out)
    {
        if (actions.size() > 2)
        {
            return false;
        }
        for (auto& action : actions)
        {
            if (action == "Event")
            {
                out.push_back("RedfishEvent");
            }
            else if (action == "Log")
            {
                out.push_back("LogToMetricReportsCollection");
            }
            else
            {
                return false;
            }
        }
        return true;
    }

    using ReadingParameters =
        std::vector<std::tuple<std::vector<sdbusplus::message::object_path>,
                               std::string, std::string, std::string>>;

    static nlohmann::json toMetrics(const ReadingParameters& params)
    {
        nlohmann::json metrics;

        for (auto& [sensorPaths, operationType, id, metadata] : params)
        {
            metrics.push_back({
                {"MetricId", id},
                {"MetricProperties", {metadata}},
            });
        }

        return metrics;
    }

    static void fillReportDefinition(
        const std::shared_ptr<AsyncResp>& asyncResp, const std::string& id,
        const std::vector<
            std::pair<std::string,
                      std::variant<std::vector<std::string>, ReadingParameters,
                                   std::string, uint32_t>>>& ret)
    {
        asyncResp->res.jsonValue["@odata.type"] = schemaType;
        asyncResp->res.jsonValue["@odata.id"] =
            telemetry::metricReportDefinitionUri + id;
        asyncResp->res.jsonValue["Id"] = id;
        asyncResp->res.jsonValue["Name"] = id;
        asyncResp->res.jsonValue["MetricReport"]["@odata.id"] =
            telemetry::metricReportUri + id;
        asyncResp->res.jsonValue["Status"]["State"] = "Enabled";
        asyncResp->res.jsonValue["ReportUpdates"] = "Overwrite";

        const std::vector<std::string>* reportActions = nullptr;
        const ReadingParameters* readingParams = nullptr;
        const std::string* reportingType = nullptr;
        const uint32_t* scanPeriod = nullptr;
        for (const auto& [key, var] : ret)
        {
            if (key == "ReportAction")
            {
                reportActions = std::get_if<std::vector<std::string>>(&var);
            }
            else if (key == "ReadingParameters")
            {
                readingParams = std::get_if<ReadingParameters>(&var);
            }
            else if (key == "ReportingType")
            {
                reportingType = std::get_if<std::string>(&var);
            }
            else if (key == "ScanPeriod")
            {
                scanPeriod = std::get_if<uint32_t>(&var);
            }
        }
        if (!reportActions || !readingParams || !reportingType || !scanPeriod)
        {
            BMCWEB_LOG_ERROR << "Property type mismatch or property is missing";
            messages::internalError(asyncResp->res);
            return;
        }

        std::vector<std::string> redfishReportActions;
        if (!toRedfishReportActions(*reportActions, redfishReportActions))
        {
            BMCWEB_LOG_ERROR << "Received invalid ReportActions from backend";
            messages::internalError(asyncResp->res);
            return;
        }

        asyncResp->res.jsonValue["ReportActions"] = redfishReportActions;
        asyncResp->res.jsonValue["Metrics"] = toMetrics(*readingParams);
        asyncResp->res.jsonValue["MetricReportDefinitionType"] = *reportingType;
        asyncResp->res.jsonValue["Schedule"]["RecurrenceInterval"] =
            time_utils::toDurationString(
                std::chrono::milliseconds(*scanPeriod));
    }

    static constexpr const char* schemaType =
        "#MetricReportDefinition.v1_3_0.MetricReportDefinition";
};
} // namespace redfish
