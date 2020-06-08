#pragma once

#include "node.hpp"
#include "sensors.hpp"
#include "utils/telemetry_utils.hpp"
#include "utils/time_utils.hpp"

#include <boost/container/flat_map.hpp>

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

    using ChassisSensorNode = std::pair<std::string, std::string>;
    using DbusSensors = std::vector<sdbusplus::message::object_path>;
    using ReadingParameters =
        std::vector<std::tuple<std::vector<sdbusplus::message::object_path>,
                               std::string, std::string, std::string>>;
    struct AddReportArgs
    {
        std::string name;
        std::string reportingType;
        bool emitsReadingsUpdate = false;
        bool logToMetricReportsCollection = false;
        uint64_t interval = 0;
        ReadingParameters readingParams;
    };

    void doPost(crow::Response& res, const crow::Request& req,
                const std::vector<std::string>&) override
    {
        auto asyncResp = std::make_shared<AsyncResp>(res);
        AddReportArgs addReportArgs;
        if (!getUserParameters(res, req, addReportArgs))
        {
            return;
        }

        boost::container::flat_set<ChassisSensorNode> chassisSensorSet;
        if (!getChassisSensorNode(asyncResp, addReportArgs.readingParams,
                                  chassisSensorSet))
        {
            return;
        }

        auto addReportReq =
            std::make_shared<AddReport>(addReportArgs, asyncResp);
        for (const auto& [chassis, sensorType] : chassisSensorSet)
        {
            retrieveUriToDbusMap(
                chassis, sensorType,
                [asyncResp, addReportReq](
                    const boost::beast::http::status status,
                    const boost::container::flat_map<std::string, std::string>&
                        uriToDbus) {
                    if (status != boost::beast::http::status::ok)
                    {
                        BMCWEB_LOG_ERROR << "Failed to retrieve URI to dbus "
                                            "sensors map with err "
                                         << static_cast<unsigned>(status);
                        messages::internalError(asyncResp->res);
                        return;
                    }
                    addReportReq->insert(uriToDbus);
                });
        }
    }

    static bool toDbusReportActions(crow::Response& res,
                                    std::vector<std::string>& actions,
                                    AddReportArgs& params)
    {
        size_t index = 0;
        for (auto& action : actions)
        {
            if (action == "RedfishEvent")
            {
                params.emitsReadingsUpdate = true;
            }
            else if (action == "LogToMetricReportsCollection")
            {
                params.logToMetricReportsCollection = true;
            }
            else
            {
                messages::propertyValueNotInList(
                    res, action, "ReportActions/" + std::to_string(index));
                return false;
            }
            index++;
        }
        return true;
    }

    static bool getUserParameters(crow::Response& res, const crow::Request& req,
                                  AddReportArgs& args)
    {
        std::vector<nlohmann::json> metrics;
        std::vector<std::string> reportActions;
        std::optional<nlohmann::json> schedule;
        if (!json_util::readJson(req, res, "Id", args.name, "Metrics", metrics,
                                 "MetricReportDefinitionType",
                                 args.reportingType, "ReportActions",
                                 reportActions, "Schedule", schedule))
        {
            return false;
        }

        constexpr const char* allowedCharactersInName =
            "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";
        if (args.name.empty() ||
            args.name.find_first_not_of(allowedCharactersInName) !=
                std::string::npos)
        {
            BMCWEB_LOG_ERROR << "Failed to match " << args.name
                             << " with allowed character "
                             << allowedCharactersInName;
            messages::propertyValueIncorrect(res, "Id", args.name);
            return false;
        }

        if (args.reportingType != "Periodic" &&
            args.reportingType != "OnRequest")
        {
            messages::propertyValueNotInList(res, args.reportingType,
                                             "MetricReportDefinitionType");
            return false;
        }

        if (!toDbusReportActions(res, reportActions, args))
        {
            return false;
        }

        if (args.reportingType == "Periodic")
        {
            if (!schedule)
            {
                messages::createFailedMissingReqProperties(res, "Schedule");
                return false;
            }

            std::string durationStr;
            if (!json_util::readJson(*schedule, res, "RecurrenceInterval",
                                     durationStr))
            {
                return false;
            }

            std::optional<std::chrono::milliseconds> durationNum =
                time_utils::fromDurationString(durationStr);
            if (!durationNum)
            {
                messages::propertyValueIncorrect(res, "RecurrenceInterval",
                                                 durationStr);
                return false;
            }
            args.interval = static_cast<uint64_t>(durationNum->count());
        }

        return fillReadingParameters(res, metrics, args.readingParams);
    }

    static bool fillReadingParameters(crow::Response& res,
                                      std::vector<nlohmann::json>& metrics,
                                      ReadingParameters& readingParams)
    {
        readingParams.reserve(metrics.size());
        for (auto& m : metrics)
        {
            std::string metricId;
            std::vector<std::string> metricProperties;
            if (!json_util::readJson(m, res, "MetricId", metricId,
                                     "MetricProperties", metricProperties))
            {
                return false;
            }

            DbusSensors dbusSensors;
            dbusSensors.reserve(metricProperties.size());
            for (const auto& prop : metricProperties)
            {
                dbusSensors.emplace_back(prop);
            }
            nlohmann::json metadata;
            metadata["MetricProperties"] = metricProperties;
            if (metricProperties.size() == 1)
            {
                std::string sensorType;
                metadata["MetricProperty"] = metricProperties[0];
                if (dbus::utility::getNthStringFromPath(metricProperties[0], 3,
                                                        sensorType))
                {
                    metadata["MetricDefinition"]["@odata.id"] =
                        telemetry::metricDefinitionUri + sensorType;
                }
            }

            readingParams.emplace_back(dbusSensors, "SINGLE", metricId,
                                       metadata.dump());
        }
        return true;
    }

    static bool getChassisSensorNode(
        const std::shared_ptr<AsyncResp>& asyncResp,
        const ReadingParameters& readingParams,
        boost::container::flat_set<ChassisSensorNode>& matched)
    {
        for (const auto& readingParam : readingParams)
        {
            size_t index = 0;
            const DbusSensors& sensors = std::get<DbusSensors>(readingParam);
            for (const auto& sensor : sensors)
            {
                std::string chassis;
                std::string node;

                if (!boost::starts_with(sensor.str, "/redfish/v1/Chassis/") ||
                    !dbus::utility::getNthStringFromPath(sensor, 3, chassis) ||
                    !dbus::utility::getNthStringFromPath(sensor, 4, node))
                {
                    BMCWEB_LOG_ERROR << "Failed to get chassis and sensor Node "
                                        "from "
                                     << sensor.str;
                    messages::propertyValueIncorrect(asyncResp->res, sensor.str,
                                                     "MetricProperties/" +
                                                         std::to_string(index));
                    return false;
                }

                if (boost::ends_with(node, "#"))
                {
                    node.pop_back();
                }

                matched.emplace(chassis, node);
                index++;
            }
        }
        return true;
    }

    class AddReport
    {
      public:
        AddReport(AddReportArgs& args, std::shared_ptr<AsyncResp>& asyncResp) :
            asyncResp{asyncResp}, addReportArgs{args}
        {}
        ~AddReport()
        {
            if (asyncResp->res.result() != boost::beast::http::status::ok)
            {
                return;
            }

            for (auto& readingParam : addReportArgs.readingParams)
            {
                size_t index = 0;
                auto& dbusSensors = std::get<DbusSensors>(readingParam);
                for (auto& uri : dbusSensors)
                {
                    auto dbus = uriToDbus.find(uri);
                    if (dbus == uriToDbus.end())
                    {
                        BMCWEB_LOG_ERROR << "Failed to find DBus sensor "
                                            "corresponding to URI "
                                         << uri.str;
                        messages::propertyValueNotInList(
                            asyncResp->res,
                            "MetricProperties/" + std::to_string(index),
                            uri.str);
                        return;
                    }
                    uri = dbus->second;
                    index++;
                }
            }

            crow::connections::systemBus->async_method_call(
                [asyncResp = asyncResp, name = addReportArgs.name](
                    const boost::system::error_code ec, const std::string&) {
                    if (ec == boost::system::errc::file_exists)
                    {
                        messages::resourceAlreadyExists(
                            asyncResp->res, "MetricReportDefinition", "Id",
                            name);
                        return;
                    }
                    if (ec == boost::system::errc::too_many_files_open)
                    {
                        messages::createLimitReachedForResource(asyncResp->res);
                        return;
                    }
                    if (ec)
                    {
                        messages::internalError(asyncResp->res);
                        BMCWEB_LOG_ERROR << "respHandler DBus error " << ec;
                        return;
                    }

                    messages::created(asyncResp->res);
                },
                telemetry::service, "/xyz/openbmc_project/Telemetry/Reports",
                "xyz.openbmc_project.Telemetry.ReportManager", "AddReport",
                "TelemetryService/" + addReportArgs.name,
                addReportArgs.reportingType, addReportArgs.emitsReadingsUpdate,
                addReportArgs.logToMetricReportsCollection,
                addReportArgs.interval, addReportArgs.readingParams);
        }

        void insert(
            const boost::container::flat_map<std::string, std::string>& el)
        {
            uriToDbus.insert(el.begin(), el.end());
        }

      private:
        std::shared_ptr<AsyncResp> asyncResp;
        AddReportArgs addReportArgs;
        boost::container::flat_map<std::string, std::string> uriToDbus{};
    };
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
            redfishReportActions.emplace_back("RedfishEvent");
        }
        if (*logToMetricReportsCollection)
        {
            redfishReportActions.emplace_back("LogToMetricReportsCollection");
        }

        nlohmann::json metrics = nlohmann::json::array();
        for (auto& [sensorPaths, operationType, id, metadata] : *readingParams)
        {
            nlohmann::json metadataJson = nlohmann::json::parse(metadata);
            metrics.push_back({
                {"MetricId", id},
                {"MetricProperties", metadataJson.contains("MetricProperties")
                                         ? metadataJson["MetricProperties"]
                                         : nlohmann::json()},
            });
        }
        asyncResp->res.jsonValue["Metrics"] = metrics;
        asyncResp->res.jsonValue["MetricReportDefinitionType"] = *reportingType;
        asyncResp->res.jsonValue["ReportActions"] = redfishReportActions;
        asyncResp->res.jsonValue["Schedule"]["RecurrenceInterval"] =
            time_utils::toDurationString(std::chrono::milliseconds(*interval));
    }

    void doDelete(crow::Response& res, const crow::Request&,
                  const std::vector<std::string>& params) override
    {
        auto asyncResp = std::make_shared<AsyncResp>(res);
        if (params.size() != 1)
        {
            messages::internalError(asyncResp->res);
            return;
        }

        const std::string& id = params[0];
        const std::string reportPath = telemetry::getDbusReportPath(id);

        crow::connections::systemBus->async_method_call(
            [asyncResp, id](const boost::system::error_code ec) {
                /*
                 * boost::system::errc and std::errc are missing value for
                 * EBADR error that is defined in Linux.
                 */
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

                asyncResp->res.result(boost::beast::http::status::no_content);
            },
            telemetry::service, reportPath, "xyz.openbmc_project.Object.Delete",
            "Delete");
    }

    static constexpr const char* schemaType =
        "#MetricReportDefinition.v1_3_0.MetricReportDefinition";
};
} // namespace redfish
