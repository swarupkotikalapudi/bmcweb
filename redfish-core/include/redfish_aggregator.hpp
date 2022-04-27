#pragma once

#include <boost/asio/deadline_timer.hpp>
#include <dbus_utility.hpp>
#include <error_messages.hpp>
#include <http_client.hpp>
#include <http_connection.hpp>

namespace redfish
{

class RedfishAggregator
{
  private:
    const std::string retryPolicyName = "RedfishAggregation";
    const uint32_t retryAttempts = 5;
    const uint32_t retryTimeoutInterval = 0;
    const std::string id = "Aggregator";

    RedfishAggregator()
    {
        getSatelliteConfigs(constructorCallback);

        // Setup the retry policy to be used by Redfish Aggregation
        crow::HttpClient::getInstance().setRetryConfig(
            retryAttempts, retryTimeoutInterval, aggregationRetryHandler,
            retryPolicyName);
    }

    static inline boost::system::error_code
        aggregationRetryHandler(unsigned int respCode)
    {
        // As a default, assume 200X is alright.
        // We don't need to retry on a 404
        if ((respCode < 200) || ((respCode >= 300) && (respCode != 404)))
        {
            return boost::system::errc::make_error_code(
                boost::system::errc::result_out_of_range);
        }

        // Return 0 if the response code is valid
        return boost::system::errc::make_error_code(
            boost::system::errc::success);
    };

    // Dummy callback used by the Constructor so that it can report the number
    // of satellite configs when the class is first created
    static void constructorCallback(
        const std::unordered_map<std::string, boost::urls::url>& satelliteInfo)
    {
        BMCWEB_LOG_DEBUG << "There were "
                         << std::to_string(satelliteInfo.size())
                         << " satellite configs found at startup";
    }

    // Polls D-Bus to get all available satellite config information
    // Expects a handler which interacts with the returned configs
    static void getSatelliteConfigs(
        const std::function<void(
            const std::unordered_map<std::string, boost::urls::url>&)>& handler)
    {
        BMCWEB_LOG_DEBUG << "Gathering satellite configs";
        crow::connections::systemBus->async_method_call(
            [handler](const boost::system::error_code ec,
                      const dbus::utility::ManagedObjectType& objects) {
            if (ec)
            {
                BMCWEB_LOG_ERROR << "DBUS response error " << ec.value() << ", "
                                 << ec.message();
                return;
            }

            // Maps a chosen alias representing a satellite BMC to a url
            // containing the information required to create a http
            // connection to the satellite
            std::unordered_map<std::string, boost::urls::url> satelliteInfo;

            findSatelliteConfigs(objects, satelliteInfo);

            if (!satelliteInfo.empty())
            {
                BMCWEB_LOG_DEBUG << "Redfish Aggregation enabled with "
                                 << std::to_string(satelliteInfo.size())
                                 << " satellite BMCs";
            }
            else
            {
                BMCWEB_LOG_DEBUG
                    << "No satellite BMCs detected.  Redfish Aggregation not enabled";
            }
            handler(satelliteInfo);
            },
            "xyz.openbmc_project.EntityManager", "/",
            "org.freedesktop.DBus.ObjectManager", "GetManagedObjects");
    }

    // Search D-Bus objects for satellite config objects and add their
    // information if valid
    static void findSatelliteConfigs(
        const dbus::utility::ManagedObjectType& objects,
        std::unordered_map<std::string, boost::urls::url>& satelliteInfo)
    {
        for (const auto& objectPath : objects)
        {
            for (const auto& interface : objectPath.second)
            {
                if (interface.first ==
                    "xyz.openbmc_project.Configuration.SatelliteController")
                {
                    BMCWEB_LOG_DEBUG << "Found Satellite Controller at "
                                     << objectPath.first.str;

                    addSatelliteConfig(interface.second, satelliteInfo);
                }
            }
        }
    }

    // Parse the properties of a satellite config object and add the
    // configuration if the properties are valid
    static void addSatelliteConfig(
        const dbus::utility::DBusPropertiesMap& properties,
        std::unordered_map<std::string, boost::urls::url>& satelliteInfo)
    {
        boost::urls::url url;
        std::string name;

        for (const auto& prop : properties)
        {
            if (prop.first == "Name")
            {
                const std::string* propVal =
                    std::get_if<std::string>(&prop.second);
                if (propVal == nullptr)
                {
                    BMCWEB_LOG_ERROR << "Invalid Name value";
                    return;
                }

                // The IDs will become <Name>_<ID> so the name should not
                // contain a '_'
                if (propVal->find('_') != std::string::npos)
                {
                    BMCWEB_LOG_ERROR << "Name cannot contain a \"_\"";
                    return;
                }
                name = *propVal;
            }

            else if (prop.first == "Hostname")
            {
                const std::string* propVal =
                    std::get_if<std::string>(&prop.second);
                if (propVal == nullptr)
                {
                    BMCWEB_LOG_ERROR << "Invalid Hostname value";
                    return;
                }
                url.set_host(*propVal);
            }

            else if (prop.first == "Port")
            {
                const uint64_t* propVal = std::get_if<uint64_t>(&prop.second);
                if (propVal == nullptr)
                {
                    BMCWEB_LOG_ERROR << "Invalid Port value";
                    return;
                }

                if (*propVal > std::numeric_limits<uint16_t>::max())
                {
                    BMCWEB_LOG_ERROR << "Port value out of range";
                    return;
                }
                url.set_port(static_cast<uint16_t>(*propVal));
            }

            else if (prop.first == "AuthType")
            {
                const std::string* propVal =
                    std::get_if<std::string>(&prop.second);
                if (propVal == nullptr)
                {
                    BMCWEB_LOG_ERROR << "Invalid AuthType value";
                    return;
                }

                // For now assume authentication not required to communicate
                // with the satellite BMC
                if (*propVal != "None")
                {
                    BMCWEB_LOG_ERROR
                        << "Unsupported AuthType value: " << *propVal
                        << ", only \"none\" is supported";
                    return;
                }
                url.set_scheme("http");
            }
        } // Finished reading properties

        // Make sure all required config information was made available
        if (name.empty())
        {
            BMCWEB_LOG_ERROR << "Satellite config missing Name";
            return;
        }

        if (url.host().empty())
        {
            BMCWEB_LOG_ERROR << "Satellite config " << name << " missing Host";
            return;
        }

        if (!url.has_port())
        {
            BMCWEB_LOG_ERROR << "Satellite config " << name << " missing Port";
            return;
        }

        if (!url.has_scheme())
        {
            BMCWEB_LOG_ERROR << "Satellite config " << name
                             << " missing AuthType";
            return;
        }

        std::string resultString;
        auto result = satelliteInfo.insert_or_assign(name, std::move(url));
        if (result.second)
        {
            resultString = "Added new satellite config ";
        }
        else
        {
            resultString = "Updated existing satellite config ";
        }

        BMCWEB_LOG_DEBUG << resultString << name << " at "
                         << result.first->second.scheme() << "://"
                         << result.first->second.encoded_host_and_port();
    }

    // Intended to handle an incoming request based on if Redfish Aggregation
    // is enabled.  Forwards request to satellite BMC if it exists.
    void aggregateAndHandle(
        crow::Request& thisReq,
        const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
        const std::unordered_map<std::string, boost::urls::url>& satelliteInfo)
    {
        BMCWEB_LOG_DEBUG << "MYDEBUG: Start of aggregateAndHandle()";
        BMCWEB_LOG_DEBUG << "MYDEBUG: size is "
                         << std::to_string(satelliteInfo.size());

        // satelliteInfo will contain all satellite config information
        // If it is empty then that means we don't need to perform any
        // aggregation operations
        if (!satelliteInfo.empty())
        {
            BMCWEB_LOG_DEBUG << "Aggregation is enabled, begin processing of "
                             << thisReq.target();

            std::vector<std::string> prefixes;
            getPrefixes(prefixes, satelliteInfo);
            boost::urls::string_view origTarget(thisReq.target().data(),
                                                thisReq.target().size());

            // We need to forward the request if its path begins with form of
            // /redfish/v1/<valid_resource>/<known_prefix>_<resource_id>
            auto parsed = boost::urls::parse_relative_ref(origTarget);

            if (!parsed)
            {
                BMCWEB_LOG_ERROR << "Unable to parse path";
                return;
            }

            boost::urls::url thisURL(*parsed);

            auto segments = thisURL.segments();
            std::string knownPrefix;

            if (segments.size() >= 4)
            {
                if ((segments.size() == 4) && (segments[3].empty()))
                {
                    // The request was for a collection and ended with a "/"
                    // Don't actually do anything
                    // TODO: This should be handled so that we can aggregate
                    // the satellite resource collections
                    return;
                }

                for (auto prefix : prefixes)
                {
                    std::string targetURI(segments[3]);
                    size_t pos = targetURI.find(prefix + "_");
                    if (pos != std::string::npos)
                    {
                        BMCWEB_LOG_DEBUG << "MYDEBUG: We have a match";
                        knownPrefix = prefix;
                    }
                }
            }

            // Make sure the URI is for a Chassis, Managers, Systems, Fabrics,
            // or ComponentIntegrity resource
            // Its form should be /redfish/v1/<valid_resource>/<prefix>_<str>
            // const std::regex urlRegex(
            //    "/redfish/v1/(Chassis|Managers|Systems|Fabrics|ComponentIntegrity)/("
            //    + prefixes + ")_.+");

            // std::string targetURI(thisReq.target());

            // Is the target URI related to an aggregated resource?
            // if (std::regex_match(targetURI, urlRegex))
            if (!knownPrefix.empty())
            {
                /*

                  // The URI is for an aggregated resource and includes a prefix
                  // We need to either remove the prefix so the request can be
                  // locally handled, or forward the request to a satellite BMC
                  std::vector<std::string> fields;
                  boost::split(fields, targetURI, boost::is_any_of("/"));

                  // fields[0] will be empty because of the leading "/" in the
                  URI
                  // request.  The prefix will therefore be part of fields[4]
                  std::vector<std::string> ids;
                  boost::split(ids, fields[4], boost::is_any_of("_"));
                  BMCWEB_LOG_DEBUG << "Extracted prefix is \"" << ids[0] <<
                  "\"";

                  // We need to search the satellite prefixes";
                  BMCWEB_LOG_DEBUG << "Searching satellite prefixes for "
                                   << ids[0];

                  // If the prefix is recognized, this will forward the request
                  // to the associated satellite BMC
                  forwardRequest(thisReq, asyncResp, ids[0], satelliteInfo);

                  */
                // Remove the known prefix and then forward to the associated
                // satellite BMC
                forwardRequest(thisReq, asyncResp, knownPrefix, satelliteInfo);
                return;
            } // end handling of aggregated resource

            // TODO: We need to perform some special handling for aggregated
            // collections.  We'll want to make sure the request is for a
            // collection so we don't accidentally forward a request
            // meant for the local bmc
            //
            // Might want to create a forwardCollectionRequest() method
            // for this since we'll need to handle the response differently

            // The request should be locally handled so we don't need to do
            // anything else
            BMCWEB_LOG_DEBUG << "Aggregation not required";
        } // End Redfish Aggregation initial processing
    }     // End aggregateAndHAndle()

    // Returns vector of all aggregation prefixes
    static void getPrefixes(
        std::vector<std::string>& prefixes,
        const std::unordered_map<std::string, boost::urls::url>& satelliteInfo)
    {
        prefixes.clear();
        if (satelliteInfo.empty())
        {
            return;
        }

        for (const auto& sat : satelliteInfo)
        {
            BMCWEB_LOG_DEBUG << "Found aggregation prefix \"" << sat.first
                             << "\"";
            prefixes.push_back(sat.first);
        }

        BMCWEB_LOG_DEBUG << "Total aggregation prefixes = " << prefixes.size();
    }

    // Attempt to forward a request to the satellite BMC associated with the
    // prefix.
    void forwardRequest(
        crow::Request& thisReq,
        const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
        const std::string& prefix,
        const std::unordered_map<std::string, boost::urls::url>& satelliteInfo)
    {
        const auto& sat = satelliteInfo.find(prefix);
        if (sat == satelliteInfo.end())
        {
            // Realistically this shouldn't get called since we perform an
            // earlier check to make sure the prefix exists
            BMCWEB_LOG_ERROR << "Unrecognized satellite prefix " << prefix;
            return;
        }

        // We need to strip the prefix from the request's path
        std::string targetURI(thisReq.target());
        size_t pos = targetURI.find(prefix + "_");
        if (pos == std::string::npos)
        {
            // If this fails then something went wrong
            BMCWEB_LOG_ERROR << "Error removing prefix \"" << prefix
                             << "_\" from request URI";
            messages::internalError(asyncResp->res);
            return;
        }

        targetURI.erase(pos, prefix.size() + 1);

        std::function<void(crow::Response&)> cb =
            std::bind_front(processResponse, asyncResp);

        std::string data = boost::lexical_cast<std::string>(thisReq.req.body());
        crow::HttpClient::getInstance().sendDataWithCallback(
            data, id, std::string(sat->second.host()),
            sat->second.port_number(), targetURI, thisReq.fields,
            thisReq.method(), retryPolicyName, cb);
    }

    // Processes the response returned by a satellite BMC and loads its
    // contents into asyncResp
    static void
        processResponse(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                        crow::Response& resp)
    {
        // No processing needed if the request wasn't successful
        if (resp.resultInt() != 200)
        {
            BMCWEB_LOG_DEBUG << "No need to parse satellite response";
            asyncResp->res.stringResponse = std::move(resp.stringResponse);
            return;
        }

        // The resp will not have a json component
        // We need to create a json from resp's stringResponse
        if ((resp.stringResponse->base()["Content-Type"] ==
             "application/json") ||
            (nlohmann::json::accept(resp.body())))
        {
            nlohmann::json jsonVal =
                nlohmann::json::parse(resp.body(), nullptr, false);
            if (jsonVal.is_discarded())
            {
                BMCWEB_LOG_ERROR << "Error parsing satellite response as JSON";
                messages::internalError(asyncResp->res);
                return;
            }

            BMCWEB_LOG_DEBUG << "Successfully parsed satellite response";

            // TODO: For collections (including $expand) we  want to add the
            // satellite responses to our response rather than just straight
            // overwriting them if our local handling was successful (i.e.
            // would return a 200).

            // The aggregating BMC should have also handled this request and
            // returned a 404.  We need to overwrite that.
            asyncResp->res.stringResponse.emplace(
                boost::beast::http::response<
                    boost::beast::http::string_body>{});
            asyncResp->res.result(resp.result());
            asyncResp->res.jsonValue = std::move(jsonVal);

            BMCWEB_LOG_DEBUG << "MYDEBUG: Finished overwriting asyncResp";
            // TODO: Need to fix the URIs in the response so that they include
            // the prefix
        }
        else
        {
            // We received a 200 response without a parsable payload so just
            // copy it as is
            asyncResp->res.stringResponse = std::move(resp.stringResponse);
        }
    }

  public:
    RedfishAggregator(const RedfishAggregator&) = delete;
    RedfishAggregator& operator=(const RedfishAggregator&) = delete;
    RedfishAggregator(RedfishAggregator&&) = delete;
    RedfishAggregator& operator=(RedfishAggregator&&) = delete;
    ~RedfishAggregator() = default;

    static RedfishAggregator& getInstance()
    {
        static RedfishAggregator handler;
        return handler;
    }

    // Entry point to Redfish Aggregation
    static void
        beginAggregation(const crow::Request& thisReq,
                         const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
    {
        // Create a copy of thisReq so we we can still locally process the req
        boost::beast::http::request<boost::beast::http::string_body> req =
            thisReq.req;
        std::error_code ec;
        auto localReq = std::make_shared<crow::Request>(req, ec);
        if (ec)
        {
            BMCWEB_LOG_ERROR << "Failed to create copy of request";
            return;
        }

        //        std::weak_ptr<crow::Request> weakReq = localReq;
        //        auto cb = [&copyReq, &asyncResp](
        //        auto cb = [&weakReq, &asyncResp](
        /*
                auto cb = [localReq, &asyncResp](
                              const std::unordered_map<std::string,
           boost::urls::url>& satelliteInfo) {
                    // std::shared_ptr<crow::Request> cbReq = weakReq.lock();
                    // if (!cbReq)
                    //{
                    //    BMCWEB_LOG_CRITICAL << cbReq << " Failed to capture
           connection";
                     //   return;
                    //}
                    //getInstance().aggregateAndHandle(copyReq, asyncResp,
           satelliteInfo);
                    //getInstance().aggregateAndHandle(*cbReq, asyncResp,
           satelliteInfo); getInstance().aggregateAndHandle(*localReq,
           asyncResp, satelliteInfo);
                };

                getSatelliteConfigs(std::move(cb));
                */

        auto cb = std::bind_front(
            [localReq](const std::shared_ptr<bmcweb::AsyncResp> aResp,
                       const std::unordered_map<std::string, boost::urls::url>&
                           satelliteInfo) {
            getInstance().aggregateAndHandle(*localReq, aResp, satelliteInfo);
            },
            asyncResp);

/*
        // Maybe capture a weak_ptr to asyncResp and then turn it into a
        // shared_ptr inside the callback and pass that to aggregateAndHandle?
        auto cb =
            [localReq, &asyncResp](
                       const std::unordered_map<std::string, boost::urls::url>&
                           satelliteInfo) {
            getInstance().aggregateAndHandle(*localReq, asyncResp, satelliteInfo);
            };
*/
        getSatelliteConfigs(std::move(cb));
        //getSatelliteConfigs(cb);
    }
};

} // namespace redfish
