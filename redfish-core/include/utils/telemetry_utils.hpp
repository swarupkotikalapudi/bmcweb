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

namespace redfish
{

namespace telemetry
{

constexpr const char* service = "xyz.openbmc_project.MonitoringService";
constexpr const char* reportInterface =
    "xyz.openbmc_project.MonitoringService.Report";
constexpr const char* metricReportDefinitionUri =
    "/redfish/v1/TelemetryService/MetricReportDefinitions/";
constexpr const char* metricReportUri =
    "/redfish/v1/TelemetryService/MetricReports/";
constexpr const char* reportDir =
    "/xyz/openbmc_project/MonitoringService/Reports/TelemetryService/";

void getReportCollection(const std::shared_ptr<AsyncResp>& asyncResp,
                         const std::string& uri)
{
    const std::array<const char*, 1> interfaces = {reportInterface};

    dbus::utility::getSubTreePaths(
        [asyncResp, uri](const boost::system::error_code ec,
                         const std::vector<std::string>& reports) {
            if (ec == boost::system::errc::no_such_file_or_directory)
            {
                asyncResp->res.jsonValue["Members"] = nlohmann::json::array();
                asyncResp->res.jsonValue["Members@odata.count"] = 0;
                return;
            }

            if (ec)
            {
                messages::internalError(asyncResp->res);
                BMCWEB_LOG_ERROR << "respHandler DBus error " << ec;
                return;
            }

            json_util::dbusPathsToMembersArray(asyncResp->res, reports, uri);
        },
        "/xyz/openbmc_project/MonitoringService/Reports/TelemetryService", 1,
        interfaces);
}

/* Types of properties that are present on MonitoringService.Report interface */
using Timestamp = int32_t;
using ReadingParameters =
    std::vector<std::tuple<std::vector<sdbusplus::message::object_path>,
                           std::string, std::string, std::string>>;
using Readings =
    std::vector<std::tuple<std::string, std::string, double, Timestamp>>;
using ReportAction = std::vector<std::string>;
using ReportingType = std::string;
using ScanPeriod = uint32_t;
using ReportProp = std::variant<ReadingParameters, Readings, ReportAction,
                                ReportingType, ScanPeriod, Timestamp>;

std::string getDbusReportPath(const std::string& id)
{
    std::string path = reportDir + id;
    dbus::utility::escapePathForDbus(path);
    return path;
}

void getReport(
    const std::shared_ptr<AsyncResp>& asyncResp, const std::string& id,
    const std::string& schemaType,
    const std::function<
        void(const std::shared_ptr<AsyncResp>&, const std::string&,
             const std::vector<std::pair<std::string, ReportProp>>&)>&& func)
{
    const std::string reportPath = getDbusReportPath(id);

    dbus::utility::getAllProperties(
        [asyncResp, id, schemaType, func = std::move(func)](
            const boost::system::error_code ec,
            const std::vector<std::pair<std::string, ReportProp>>& ret) {
            if (ec == boost::system::errc::no_such_file_or_directory)
            {
                messages::resourceNotFound(asyncResp->res, schemaType, id);
                return;
            }

            if (ec || !func)
            {
                messages::internalError(asyncResp->res);
                BMCWEB_LOG_ERROR << "respHandler DBus error " << ec;
                return;
            }

            func(asyncResp, id, ret);
        },
        service, reportPath, reportInterface);
}
} // namespace telemetry
} // namespace redfish
