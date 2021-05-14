#pragma once

#include "http_response.hpp"

#include <boost/container/flat_map.hpp>
#include <nlohmann/json.hpp>

#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace redfish
{

namespace sensors
{
namespace node
{
static constexpr std::string_view power = "Power";
static constexpr std::string_view sensors = "Sensors";
static constexpr std::string_view thermal = "Thermal";
} // namespace node

namespace dbus
{

static const boost::container::flat_map<std::string_view,
                                        std::vector<const char*>>
    paths = {{node::power,
              {"/xyz/openbmc_project/sensors/voltage",
               "/xyz/openbmc_project/sensors/power"}},
             {node::sensors,
              {"/xyz/openbmc_project/sensors/power",
               "/xyz/openbmc_project/sensors/current",
               "/xyz/openbmc_project/sensors/utilization"}},
             {node::thermal,
              {"/xyz/openbmc_project/sensors/fan_tach",
               "/xyz/openbmc_project/sensors/temperature",
               "/xyz/openbmc_project/sensors/fan_pwm"}}};
} // namespace dbus
} // namespace sensors

/**
 * SensorsAsyncResp
 * Gathers data needed for response processing after async calls are done
 */
class SensorsAsyncResp
{
  public:
    using DataCompleteCb = std::function<void(
        const boost::beast::http::status status,
        const boost::container::flat_map<std::string, std::string>& uriToDbus)>;

    struct SensorData
    {
        const std::string name;
        std::string uri;
        const std::string valueKey;
        const std::string dbusPath;
    };

    SensorsAsyncResp(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                     const std::string& chassisIdIn,
                     const std::vector<const char*>& typesIn,
                     const std::string_view& subNode) :
        asyncResp(asyncResp),
        chassisId(chassisIdIn), types(typesIn), chassisSubNode(subNode)
    {}

    // Store extra data about sensor mapping and return it in callback
    SensorsAsyncResp(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                     const std::string& chassisIdIn,
                     const std::vector<const char*>& typesIn,
                     const std::string_view& subNode,
                     DataCompleteCb&& creationComplete) :
        asyncResp(asyncResp),
        chassisId(chassisIdIn), types(typesIn),
        chassisSubNode(subNode), metadata{std::vector<SensorData>()},
        dataComplete{std::move(creationComplete)}
    {}

    ~SensorsAsyncResp()
    {
        if (asyncResp->res.result() ==
            boost::beast::http::status::internal_server_error)
        {
            // Reset the json object to clear out any data that made it in
            // before the error happened todo(ed) handle error condition with
            // proper code
            asyncResp->res.jsonValue = nlohmann::json::object();
        }

        if (dataComplete && metadata)
        {
            boost::container::flat_map<std::string, std::string> map;
            if (asyncResp->res.result() == boost::beast::http::status::ok)
            {
                for (auto& sensor : *metadata)
                {
                    map.insert(std::make_pair(sensor.uri + sensor.valueKey,
                                              sensor.dbusPath));
                }
            }
            dataComplete(asyncResp->res.result(), map);
        }
    }

    void addMetadata(const nlohmann::json& sensorObject,
                     const std::string& valueKey, const std::string& dbusPath)
    {
        if (metadata)
        {
            metadata->emplace_back(SensorData{sensorObject["Name"],
                                              sensorObject["@odata.id"],
                                              valueKey, dbusPath});
        }
    }

    void updateUri(const std::string& name, const std::string& uri)
    {
        if (metadata)
        {
            for (auto& sensor : *metadata)
            {
                if (sensor.name == name)
                {
                    sensor.uri = uri;
                }
            }
        }
    }

    const std::shared_ptr<bmcweb::AsyncResp> asyncResp;
    const std::string chassisId;
    const std::vector<const char*> types;
    const std::string chassisSubNode;

  private:
    std::optional<std::vector<SensorData>> metadata;
    DataCompleteCb dataComplete;
};

inline void
    getChassisData(const std::shared_ptr<SensorsAsyncResp>& sensorsAsyncResp);

/**
 * @brief Retrieves mapping of Redfish URIs to sensor value property to D-Bus
 * path of the sensor.
 *
 * Function builds valid Redfish response for sensor query of given chassis and
 * node. It then builds metadata about Redfish<->D-Bus correlations and provides
 * it to caller in a callback.
 *
 * @param chassis   Chassis for which retrieval should be performed
 * @param node  Node (group) of sensors. See sensors::node for supported values
 * @param mapComplete   Callback to be called with retrieval result
 */
inline void retrieveUriToDbusMap(const std::string& chassis,
                                 const std::string& node,
                                 SensorsAsyncResp::DataCompleteCb&& mapComplete)
{
    auto pathIt = sensors::dbus::paths.find(node);
    if (pathIt == sensors::dbus::paths.end())
    {
        BMCWEB_LOG_ERROR << "Wrong node provided : " << node;
        mapComplete(boost::beast::http::status::bad_request, {});
        return;
    }

    auto res = std::make_shared<crow::Response>();
    auto asyncResp = std::make_shared<bmcweb::AsyncResp>(*res);
    auto callback =
        [res, asyncResp, mapCompleteCb{std::move(mapComplete)}](
            const boost::beast::http::status status,
            const boost::container::flat_map<std::string, std::string>&
                uriToDbus) { mapCompleteCb(status, uriToDbus); };

    auto resp = std::make_shared<SensorsAsyncResp>(
        asyncResp, chassis, pathIt->second, node, std::move(callback));
    getChassisData(resp);
}

} // namespace redfish