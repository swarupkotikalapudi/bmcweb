#pragma once

#include "node.hpp"
#include "utils/telemetry_utils.hpp"
#include "utils/time_utils.hpp"

#include <tuple>
#include <variant>

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
                     std::string, std::variant<bool, ReadingParameters,
                                               std::string, uint64_t>>>& ret) {
                if (ec.value() == EBADR)
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

    using ReadingParameters =
        std::vector<std::tuple<std::vector<sdbusplus::message::object_path>,
                               std::string, std::string, std::string>>;

    static void fillReportDefinition(
        const std::shared_ptr<AsyncResp>& asyncResp, const std::string& id,
        const std::vector<
            std::pair<std::string, std::variant<bool, ReadingParameters,
                                                std::string, uint64_t>>>& ret)
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

        const bool* emitsReadingsUpdate = nullptr;
        const bool* logToMetricReportsCollection = nullptr;
        const ReadingParameters* readingParams = nullptr;
        const std::string* reportingType = nullptr;
        const uint64_t* interval = nullptr;
        for (const auto& [key, var] : ret)
        {
            if (key == "EmitsReadingsUpdate")
            {
                emitsReadingsUpdate = std::get_if<bool>(&var);
            }
            else if (key == "LogToMetricReportsCollection")
            {
                logToMetricReportsCollection = std::get_if<bool>(&var);
            }
            else if (key == "ReadingParameters")
            {
                readingParams = std::get_if<ReadingParameters>(&var);
            }
            else if (key == "ReportingType")
            {
                reportingType = std::get_if<std::string>(&var);
            }
            else if (key == "Interval")
            {
                interval = std::get_if<uint64_t>(&var);
            }
        }
        if (!emitsReadingsUpdate || !logToMetricReportsCollection ||
            !readingParams || !reportingType || !interval)
        {
            BMCWEB_LOG_ERROR << "Property type mismatch or property is missing";
            messages::internalError(asyncResp->res);
            return;
        }

        std::vector<std::string> redfishReportActions;
        redfishReportActions.reserve(2);
        if (*emitsReadingsUpdate)
        {
            redfishReportActions.push_back("RedfishEvent");
        }
        if (*logToMetricReportsCollection)
        {
            redfishReportActions.push_back("LogToMetricReportsCollection");
        }

        nlohmann::json metrics = nlohmann::json::array();
        for (auto& [sensorPaths, operationType, id, metadata] : *readingParams)
        {
            nlohmann::json metadataJson = nlohmann::json::parse(metadata);
            if (!metadataJson.is_array())
            {
                metadataJson = nlohmann::json::array_t();
            }
            metrics.push_back({
                {"MetricId", id},
                {"MetricProperties", metadataJson},
            });
        }
        asyncResp->res.jsonValue["Metrics"] = metrics;
        asyncResp->res.jsonValue["MetricReportDefinitionType"] = *reportingType;
        asyncResp->res.jsonValue["ReportActions"] = redfishReportActions;
        asyncResp->res.jsonValue["Schedule"]["RecurrenceInterval"] =
            time_utils::toDurationString(std::chrono::milliseconds(*interval));
    }

    static constexpr const char* schemaType =
        "#MetricReportDefinition.v1_3_0.MetricReportDefinition";
};
} // namespace redfish
