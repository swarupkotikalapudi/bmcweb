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
#include "utils/time_utils.hpp"

#include <boost/container/flat_map.hpp>
#include <system_error>
#include <variant>

namespace redfish
{

class TelemetryService : public Node
{
  public:
    TelemetryService(CrowApp& app) : Node(app, "/redfish/v1/TelemetryService/")
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
        res.jsonValue["@odata.type"] =
            "#TelemetryService.v1_2_0.TelemetryService";
        res.jsonValue["@odata.id"] = "/redfish/v1/TelemetryService";
        res.jsonValue["Id"] = "TelemetryService";
        res.jsonValue["Name"] = "Telemetry Service";

        res.jsonValue["LogService"]["@odata.id"] =
            "/redfish/v1/Managers/bmc/LogServices/Journal";
        res.jsonValue["MetricReportDefinitions"]["@odata.id"] =
            "/redfish/v1/TelemetryService/MetricReportDefinitions";
        res.jsonValue["MetricReports"]["@odata.id"] =
            "/redfish/v1/TelemetryService/MetricReports";

        getMonitoringServiceProperties(res);
    }

    void getMonitoringServiceProperties(crow::Response& res)
    {
        auto asyncResp = std::make_shared<AsyncResp>(res);
        crow::connections::systemBus->async_method_call(
            [asyncResp](
                const boost::system::error_code ec,
                const boost::container::flat_map<std::string,
                                                 std::variant<uint32_t>>& ret) {
                if (ec)
                {
                    asyncResp->res.jsonValue["ServiceEnabled"] = false;
                    asyncResp->res.jsonValue["Status"]["State"] = "Absent";
                    BMCWEB_LOG_ERROR << "respHandler DBus error " << ec;
                    return;
                }

                asyncResp->res.jsonValue["ServiceEnabled"] = true;
                asyncResp->res.jsonValue["Status"]["State"] = "Enabled";

                json_util::assignIfPresent<uint32_t>(ret, "MaxReports",
                                                     asyncResp->res);
                json_util::assignIfPresent<uint32_t>(
                    ret, "PollRateResolution",
                    asyncResp->res.jsonValue["MinCollectionInterval"],
                    time_utils::toDurationFormat);
            },
            "xyz.openbmc_project.MonitoringService",
            "/xyz/openbmc_project/MonitoringService/Reports",
            "org.freedesktop.DBus.Properties", "GetAll",
            "xyz.openbmc_project.MonitoringService.ReportsManagement");
    }
};

class MetricReportDefinitionCollection : public Node
{
  public:
    MetricReportDefinitionCollection(CrowApp& app) :
        Node(app, "/redfish/v1/TelemetryService/MetricReportDefinitions")
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
        res.jsonValue["@odata.type"] = "#MetricReportDefinitionCollection."
                                       "MetricReportDefinitionCollection";
        res.jsonValue["@odata.id"] =
            "/redfish/v1/TelemetryService/MetricReportDefinitions";
        res.jsonValue["Name"] = "Metric Definition Collection";

        const std::array<const char*, 1> interfaces = {
            "xyz.openbmc_project.MonitoringService.Report"};

        auto asyncResp = std::make_shared<AsyncResp>(res);
        crow::connections::systemBus->async_method_call(
            [asyncResp](const boost::system::error_code ec,
                        const std::vector<std::string>& reports) {
                if (ec == boost::system::errc::no_such_file_or_directory)
                {
                    asyncResp->res.jsonValue["Members"] =
                        nlohmann::json::array();
                    asyncResp->res.jsonValue["Members@odata.count"] = 0;
                    return;
                }

                if (ec)
                {
                    messages::internalError(asyncResp->res);
                    BMCWEB_LOG_ERROR << "respHandler DBus error " << ec;
                    return;
                }

                json_util::dbusPathsToMembersArray(
                    asyncResp->res, reports,
                    "/redfish/v1/TelemetryService/MetricReportDefinitions/");
            },
            "xyz.openbmc_project.ObjectMapper",
            "/xyz/openbmc_project/object_mapper",
            "xyz.openbmc_project.ObjectMapper", "GetSubTreePaths",
            "/xyz/openbmc_project/MonitoringService/Reports/TelemetryService",
            1, interfaces);
    }
};

class MetricReportDefinition : public Node
{
  public:
    MetricReportDefinition(CrowApp& app) :
        Node(app, "/redfish/v1/TelemetryService/MetricReportDefinitions/<str>",
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
    void doGet(crow::Response& res, const crow::Request& req,
               const std::vector<std::string>& params) override
    {
        auto asyncResp = std::make_shared<AsyncResp>(res);

        if (params.size() != 1)
        {
            messages::internalError(asyncResp->res);
            return;
        }

        const std::string& id = params[0];
        const std::array<const char*, 1> interfaces = {
            "xyz.openbmc_project.MonitoringService.Report"};

        crow::connections::systemBus->async_method_call(
            [asyncResp, id](const boost::system::error_code ec,
                            const std::vector<std::string>& reports) {
                if (ec == boost::system::errc::no_such_file_or_directory)
                {
                    messages::resourceNotFound(
                        asyncResp->res,
                        "#MetricReportDefinition.v1_3_0.MetricReportDefinition",
                        id);
                    return;
                }

                if (ec)
                {
                    messages::internalError(asyncResp->res);
                    BMCWEB_LOG_ERROR << "respHandler DBus error " << ec;
                    return;
                }

                const std::string target = "/xyz/openbmc_project/"
                                           "MonitoringService/Reports/"
                                           "TelemetryService/" +
                                           id;
                auto path = std::find(reports.begin(), reports.end(), target);
                if (path != std::end(reports))
                {
                    getReportDefinitonProperties(asyncResp, *path, id);
                    return;
                }

                messages::resourceNotFound(
                    asyncResp->res,
                    "#MetricReportDefinition.v1_3_0.MetricReportDefinition",
                    id);
            },
            "xyz.openbmc_project.ObjectMapper",
            "/xyz/openbmc_project/object_mapper",
            "xyz.openbmc_project.ObjectMapper", "GetSubTreePaths",
            "/xyz/openbmc_project/MonitoringService/Reports/TelemetryService",
            0, interfaces);
    }

    static std::vector<std::string>
        toReportActions(const std::vector<std::string>& actions)
    {
        const boost::container::flat_map<std::string, std::string>
            reportActions = {
                {"Event", "RedfishEvent"},
                {"Log", "LogToMetricReportsCollection"},
            };

        std::vector<std::string> out;
        for (auto& action : actions)
        {
            auto found = reportActions.find(action);
            if (found != reportActions.end())
            {
                out.emplace_back(found->second);
            }
        }
        return out;
    }

    using ReadingParameters = std::vector<
        std::tuple<std::string, std::vector<sdbusplus::message::object_path>,
                   std::string, std::string>>;
    using Metrics = std::vector<std::map<
        std::string, std::variant<std::string, std::vector<std::string>>>>;

    static Metrics toMetrics(const ReadingParameters& params)
    {
        Metrics metrics;

        for (auto& [id, sensorPaths, operationType, metadata] : params)
        {
            metrics.push_back({
                {"MetricId", id},
                {"MetricProperties", std::vector<std::string>() = {metadata}},
            });
        }

        return metrics;
    }

    static void
        getReportDefinitonProperties(const std::shared_ptr<AsyncResp> asyncResp,
                                     const std::string& reportPath,
                                     const std::string& id)
    {
        asyncResp->res.jsonValue["@odata.type"] =
            "#MetricReportDefinition.v1_3_0.MetricReportDefinition";
        asyncResp->res.jsonValue["@odata.id"] =
            "/redfish/v1/TelemetryService/MetricReportDefinitions/" + id;
        asyncResp->res.jsonValue["Id"] = id;
        asyncResp->res.jsonValue["Name"] = id;
        asyncResp->res.jsonValue["MetricReport"]["@odata.id"] =
            "/redfish/v1/TelemetryService/MetricReports/" + id;
        asyncResp->res.jsonValue["Status"]["State"] = "Enabled";

        crow::connections::systemBus->async_method_call(
            [asyncResp](const boost::system::error_code ec,
                        const boost::container::flat_map<
                            std::string,
                            std::variant<std::string, std::vector<std::string>,
                                         uint32_t, ReadingParameters>>& ret) {
                if (ec)
                {
                    messages::internalError(asyncResp->res);
                    BMCWEB_LOG_ERROR << "respHandler DBus error " << ec;
                    return;
                }

                json_util::assignIfPresent<std::vector<std::string>>(
                    ret, "ReportAction",
                    asyncResp->res.jsonValue["ReportActions"], toReportActions);
                auto assigned = json_util::assignIfPresent<std::string>(
                    ret, "ReportingType",
                    asyncResp->res.jsonValue["MetricReportDefinitionType"]);
                if (assigned &&
                    asyncResp->res.jsonValue["MetricReportDefinitionType"] ==
                        "Periodic")
                {
                    json_util::assignIfPresent<uint32_t>(
                        ret, "ScanPeriod",
                        asyncResp->res
                            .jsonValue["Schedule"]["RecurrenceInterval"],
                        time_utils::toDurationFormat);
                }
                json_util::assignIfPresent<ReadingParameters>(
                    ret, "ReadingParameters",
                    asyncResp->res.jsonValue["Metrics"], toMetrics);
            },
            "xyz.openbmc_project.MonitoringService", reportPath,
            "org.freedesktop.DBus.Properties", "GetAll",
            "xyz.openbmc_project.MonitoringService.Report");
    }
};

class MetricReportCollection : public Node
{
  public:
    MetricReportCollection(CrowApp& app) :
        Node(app, "/redfish/v1/TelemetryService/MetricReports")
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
        res.jsonValue["@odata.type"] =
            "#MetricReportCollection.MetricReportCollection";
        res.jsonValue["@odata.id"] =
            "/redfish/v1/TelemetryService/MetricReports";
        res.jsonValue["Name"] = "Metric Report Collection";

        const std::array<const char*, 1> interfaces = {
            "xyz.openbmc_project.MonitoringService.Report"};

        auto asyncResp = std::make_shared<AsyncResp>(res);
        crow::connections::systemBus->async_method_call(
            [asyncResp](const boost::system::error_code ec,
                        const std::vector<std::string>& reports) {
                if (ec == boost::system::errc::no_such_file_or_directory)
                {
                    asyncResp->res.jsonValue["Members"] =
                        nlohmann::json::array();
                    asyncResp->res.jsonValue["Members@odata.count"] = 0;
                    return;
                }

                if (ec)
                {
                    messages::internalError(asyncResp->res);
                    BMCWEB_LOG_ERROR << "respHandler DBus error " << ec;
                    return;
                }

                json_util::dbusPathsToMembersArray(
                    asyncResp->res, reports,
                    "/redfish/v1/TelemetryService/MetricReports/");
            },
            "xyz.openbmc_project.ObjectMapper",
            "/xyz/openbmc_project/object_mapper",
            "xyz.openbmc_project.ObjectMapper", "GetSubTreePaths",
            "/xyz/openbmc_project/MonitoringService/Reports/TelemetryService",
            1, interfaces);
    }
};

class MetricReport : public Node
{
  public:
    MetricReport(CrowApp& app) :
        Node(app, "/redfish/v1/TelemetryService/MetricReports/<str>",
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
    void doGet(crow::Response& res, const crow::Request& req,
               const std::vector<std::string>& params) override
    {
        auto asyncResp = std::make_shared<AsyncResp>(res);

        if (params.size() != 1)
        {
            messages::internalError(asyncResp->res);
            return;
        }

        const std::string& id = params[0];
        const std::array<const char*, 1> interfaces = {
            "xyz.openbmc_project.MonitoringService.Report"};

        crow::connections::systemBus->async_method_call(
            [asyncResp, id](const boost::system::error_code ec,
                            const std::vector<std::string>& reports) {
                if (ec == boost::system::errc::no_such_file_or_directory)
                {
                    messages::resourceNotFound(
                        asyncResp->res, "#MetricReport.v1_3_0.MetricReport",
                        id);
                    return;
                }

                if (ec)
                {
                    messages::internalError(asyncResp->res);
                    BMCWEB_LOG_ERROR << "respHandler DBus error " << ec;
                    return;
                }

                const std::string target = "/xyz/openbmc_project/"
                                           "MonitoringService/Reports/"
                                           "TelemetryService/" +
                                           id;
                auto objpath =
                    std::find(reports.begin(), reports.end(), target);
                if (objpath != std::end(reports))
                {
                    getReportProperties(asyncResp, *objpath, id);
                    return;
                }

                messages::resourceNotFound(
                    asyncResp->res, "#MetricReport.v1_3_0.MetricReport", id);
            },
            "xyz.openbmc_project.ObjectMapper",
            "/xyz/openbmc_project/object_mapper",
            "xyz.openbmc_project.ObjectMapper", "GetSubTreePaths",
            "/xyz/openbmc_project/MonitoringService/Reports/TelemetryService",
            0, interfaces);
    }

    using Readings =
        std::vector<std::tuple<std::string, std::string, double, std::string>>;
    using MetricValues = std::vector<std::map<std::string, std::string>>;

    static MetricValues toMetricValues(const Readings& readings)
    {
        MetricValues metricValues;

        for (auto& [id, metadata, sensorValue, timestamp] : readings)
        {
            metricValues.push_back({
                {"MetricId", id},
                {"MetricProperty", metadata},
                {"MetricValue", std::to_string(sensorValue)},
                {"Timestamp", timestamp},
            });
        }

        return metricValues;
    }

    static void getReportProperties(const std::shared_ptr<AsyncResp> asyncResp,
                                    const std::string& reportPath,
                                    const std::string& id)
    {
        asyncResp->res.jsonValue["@odata.type"] =
            "#MetricReport.v1_3_0.MetricReport";
        asyncResp->res.jsonValue["@odata.id"] =
            "/redfish/v1/TelemetryService/MetricReports/" + id;
        asyncResp->res.jsonValue["Id"] = id;
        asyncResp->res.jsonValue["Name"] = id;
        asyncResp->res.jsonValue["MetricReportDefinition"]["@odata.id"] =
            "/redfish/v1/TelemetryService/MetricReportDefinitions/" + id;

        crow::connections::systemBus->async_method_call(
            [asyncResp](
                const boost::system::error_code ec,
                const boost::container::flat_map<
                    std::string, std::variant<Readings, std::string>>& ret) {
                if (ec)
                {
                    messages::internalError(asyncResp->res);
                    BMCWEB_LOG_ERROR << "respHandler DBus error " << ec;
                    return;
                }

                json_util::assignIfPresent<std::string>(ret, "Timestamp",
                                                        asyncResp->res);
                json_util::assignIfPresent<Readings>(
                    ret, "Readings", asyncResp->res.jsonValue["MetricValues"],
                    toMetricValues);
            },
            "xyz.openbmc_project.MonitoringService", reportPath,
            "org.freedesktop.DBus.Properties", "GetAll",
            "xyz.openbmc_project.MonitoringService.Report");
    }
};

} // namespace redfish
