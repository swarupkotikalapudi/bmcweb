/*
// Copyright (c) 2019 Intel Corporation
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
#include <error_messages.hpp>
#include <fstream>
#include <utils/json_utils.hpp>
#include <variant>

namespace redfish
{
using Json = nlohmann::json;

static constexpr const std::array<const char*, 2> supportedEvtTypes = {
    "Event", "MetricReport"};
static constexpr const std::array<const char*, 2> supportedRegPrefixes = {
    "Base", "OpenBMC"};
static constexpr const std::array<const char*, 3> supportedRetryPolicies = {
    "TerminateAfterRetries", "SuspendRetries", "RetryForever"};

static constexpr const uint8_t maxNoOfSubscriptions = 20;

static constexpr const char* eventServiceDefaultFile =
    "/var/lib/bmcweb/eventservice_config.json";

struct EventSrvConfig
{
    bool enabled;
    uint32_t retryAttempts;
    uint32_t retryTimeoutInterval;
};

struct EventSrvSubscription
{
    std::string destinationUrl;
    std::string protocol;
    std::string retryPolicy;
    std::string customText;
    std::string eventFormatType;
    std::string subscriptionType;
    std::vector<std::string> registryMsgIds;
    std::vector<std::string> registryPrefixes;
    std::vector<nlohmann::json> httpHeaders; // key-value pair
    std::vector<nlohmann::json> metricReportDefinitions;
};

EventSrvConfig configData;
boost::container::flat_map<std::string, EventSrvSubscription> subscriptionsMap;

inline void initEventSrvStore()
{
    /*Read the persistent data from store and populate config & subscription
      info with default*/
    std::ifstream eventConfigFile(eventServiceDefaultFile);
    if (!eventConfigFile.good())
    {
        BMCWEB_LOG_ERROR << "Cannot find event service config file";
        return;
    }
    try
    {
        auto data = nlohmann::json::parse(eventConfigFile, nullptr);
        const auto& eventConfig = data["configurations"];

        configData.enabled = eventConfig["ServiceEnabled"];
        configData.retryAttempts = eventConfig["DeliveryRetryAttempts"];
        configData.retryTimeoutInterval =
            eventConfig["DeliveryRetryIntervalSeconds"];

        for (const auto& subscriptionObj : data["subscriptions"])
        {
            BMCWEB_LOG_INFO << "Inside subscriptions for loop: ";
            EventSrvSubscription subValue;

            subValue.destinationUrl = subscriptionObj["Destination"];
            subValue.protocol = subscriptionObj["Protocol"];
            subValue.retryPolicy = subscriptionObj["DeliveryRetryPolicy"];
            subValue.customText = subscriptionObj["Context"];
            subValue.eventFormatType = subscriptionObj["EventFormatType"];
            subValue.subscriptionType = subscriptionObj["SubscriptionType"];
            subValue.registryMsgIds =
                subscriptionObj["MessageIds"].get<std::vector<std::string>>();
            subValue.registryPrefixes = subscriptionObj["RegistryPrefixes"]
                                            .get<std::vector<std::string>>();
            subValue.httpHeaders = subscriptionObj["HttpHeaders"]
                                       .get<std::vector<nlohmann::json>>();
            subValue.metricReportDefinitions =
                subscriptionObj["MetricReportDefinitions"]
                    .get<std::vector<nlohmann::json>>();

            std::string id =
                boost::uuids::to_string(boost::uuids::random_generator()());
            subscriptionsMap.insert(std::pair(id, subValue));
        }
    }
    catch (nlohmann::json::exception& e)
    {
        BMCWEB_LOG_ERROR << "Error parsing event service config file";
        return;
    }
    return;
}

inline void updateSubscriptionData()
{
    /* Persist the config and subscription data.
       subscriptionsMap & configData need to be
       written to Persist store. */

    nlohmann::json jsonData;

    nlohmann::json& configObj = jsonData["configurations"];
    configObj["ServiceEnabled"] = configData.enabled;
    configObj["DeliveryRetryAttempts"] = configData.retryAttempts;
    configObj["DeliveryRetryIntervalSeconds"] = configData.retryTimeoutInterval;

    nlohmann::json& subListArray = jsonData["subscriptions"];
    subListArray = nlohmann::json::array();

    EventSrvSubscription subValue;
    for (auto& it : subscriptionsMap)
    {
        nlohmann::json entry;
        subValue = it.second;

        entry["Context"] = subValue.customText;
        entry["DeliveryRetryPolicy"] = subValue.retryPolicy;
        entry["Destination"] = subValue.destinationUrl;
        entry["EventFormatType"] = subValue.eventFormatType;
        entry["HttpHeaders"] = subValue.httpHeaders;
        entry["MessageIds"] = subValue.registryMsgIds;
        entry["MetricReportDefinitions"] = subValue.metricReportDefinitions;
        entry["Protocol"] = subValue.protocol;
        entry["RegistryPrefixes"] = subValue.registryPrefixes;
        entry["SubscriptionType"] = subValue.subscriptionType;

        subListArray.push_back(entry);
    }

    static std::string tmpFile{std::string(eventServiceDefaultFile) + "_tmp"};

    int fd = open(tmpFile.c_str(), O_CREAT | O_WRONLY | O_TRUNC | O_SYNC,
                  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd < 0)
    {
        BMCWEB_LOG_ERROR << "Error in creating json file" << tmpFile.c_str();
        return;
    }
    const auto& writeData = jsonData.dump();
    if (write(fd, writeData.c_str(), writeData.size()) !=
        static_cast<ssize_t>(writeData.size()))
    {
        close(fd);

        BMCWEB_LOG_ERROR << "Error in writing configuration file"
                         << tmpFile.c_str();
        return;
    }
    close(fd);

    if (std::rename(tmpFile.c_str(), eventServiceDefaultFile) != 0)
    {
        BMCWEB_LOG_ERROR << "Error in renaming temporary data file"
                         << tmpFile.c_str();
        return;
    }
    return;
}
class EventService : public Node
{
  public:
    EventService(CrowApp& app) : Node(app, "/redfish/v1/EventService/")
    {
        initEventSrvStore();

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
        res.jsonValue = {
            {"@odata.type", "#EventService.v1_3_0.EventService"},
            {"@odata.context",
             "/redfish/v1/$metadata#EventService.EventService"},
            {"Id", "EventService"},
            {"Name", "Event Service"},
            {"ServerSentEventUri",
             "/redfish/v1/EventService/Subscriptions/SSE"},
            {"Subscriptions",
             {{"@odata.id", "/redfish/v1/EventService/Subscriptions"}}},
            {"Actions",
             {{"#EventService.SubmitTestEvent",
               {{"target", "/redfish/v1/EventService/Actions/"
                           "EventService.SubmitTestEvent"}}}}},
            {"@odata.id", "/redfish/v1/EventService"}};

        asyncResp->res.jsonValue["ServiceEnabled"] = configData.enabled;
        asyncResp->res.jsonValue["DeliveryRetryAttempts"] =
            configData.retryAttempts;
        asyncResp->res.jsonValue["DeliveryRetryIntervalSeconds"] =
            configData.retryTimeoutInterval;
        asyncResp->res.jsonValue["EventFormatTypes"] = supportedEvtTypes;
        asyncResp->res.jsonValue["RegistryPrefixes"] = supportedRegPrefixes;
        asyncResp->res.jsonValue["ResourceTypes"] = nlohmann::json::array();

        res.end();
    }

    void doPatch(crow::Response& res, const crow::Request& req,
                 const std::vector<std::string>& params) override
    {
        auto asyncResp = std::make_shared<AsyncResp>(res);

        std::optional<bool> serviceEnabled;
        std::optional<uint32_t> retryAttemps;
        std::optional<uint32_t> retryInterval;

        if (!json_util::readJson(req, res, "ServiceEnabled", serviceEnabled,
                                 "DeliveryRetryAttempts", retryAttemps,
                                 "DeliveryRetryIntervalSeconds", retryInterval))
        {
            return;
        }

        if (serviceEnabled)
        {
            configData.enabled = *serviceEnabled;
        }

        if (retryAttemps)
        {
            // Supported range [1-3]
            if ((*retryAttemps < 1) || (*retryAttemps > 3))
            {
                messages::queryParameterOutOfRange(
                    asyncResp->res, std::to_string(*retryAttemps),
                    "DeliveryRetryAttempts", "[1-3]");
            }
            else
            {
                configData.retryAttempts = *retryAttemps;
            }
        }

        if (retryInterval)
        {
            // Supported range [30 - 180]
            if ((*retryInterval < 30) || (*retryInterval > 180))
            {
                messages::queryParameterOutOfRange(
                    asyncResp->res, std::to_string(*retryInterval),
                    "DeliveryRetryIntervalSeconds", "[30-180]");
            }
            else
            {
                configData.retryTimeoutInterval = *retryInterval;
            }
        }
    }
};

class EventDestinationCollection : public Node
{
  public:
    EventDestinationCollection(CrowApp& app) :
        Node(app, "/redfish/v1/EventService/Subscriptions/")
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

        res.jsonValue = {
            {"@odata.type",
             "#EventDestinationCollection.v1_0_0.EventDestinationCollection"},
            {"@odata.context",
             "/redfish/v1/"
             "$metadata#EventDestinationCollection.EventDestinationCollection"},
            {"@odata.id", "/redfish/v1/EventService/Subscriptions"}};

        nlohmann::json& memberArray = asyncResp->res.jsonValue["Members"];
        memberArray = nlohmann::json::array();
        asyncResp->res.jsonValue["Members@odata.count"] =
            subscriptionsMap.size();

        for (auto& it : subscriptionsMap)
        {
            memberArray.push_back(
                {{"@odata.id",
                  "/redfish/v1/EventService/Subscriptions/" + it.first}});
        }
    }

    void doPost(crow::Response& res, const crow::Request& req,
                const std::vector<std::string>& params) override
    {
        auto asyncResp = std::make_shared<AsyncResp>(res);

        if (subscriptionsMap.size() >= maxNoOfSubscriptions)
        {
            messages::eventSubscriptionLimitExceeded(asyncResp->res);
            return;
        }
        std::string destUrl;
        std::string protocol;
        std::optional<std::string> context;
        std::optional<std::string> eventType;
        std::optional<std::string> retryPolicy;
        std::optional<std::vector<std::string>> msgIds;
        std::optional<std::vector<std::string>> regPrefixes;
        std::optional<std::vector<nlohmann::json>> headers;
        std::optional<std::vector<nlohmann::json>> metricReportDefinitions;

        if (!json_util::readJson(
                req, res, "Destination", destUrl, "Context", context,
                "Protocol", protocol, "EventFormatType", eventType,
                "HttpHeaders", headers, "RegistryPrefixes", regPrefixes,
                "MessageIds", msgIds, "DeliveryRetryPolicy", retryPolicy,
                "MetricReportDefinitions", metricReportDefinitions))
        {
            return;
        }

        EventSrvSubscription subValue;

        const std::regex urlRegex("(http|https)://([^/ :]+):?([^/ ]*)(/?[^ "
                                  "#?]*)\\x3f?([^ #]*)#?([^ ]*)");
        if (!std::regex_match(destUrl, urlRegex))
        {
            messages::propertyValueNotInList(asyncResp->res, destUrl,
                                             "Destination");
            return;
        }
        subValue.destinationUrl = destUrl;
        subValue.subscriptionType = "Event";
        if (protocol != "Redfish")
        {
            messages::propertyValueNotInList(asyncResp->res, protocol,
                                             "Protocol");
            return;
        }
        subValue.protocol = protocol;

        if (eventType)
        {
            if (std::find(supportedEvtTypes.begin(), supportedEvtTypes.end(),
                          *eventType) == supportedEvtTypes.end())
            {
                messages::propertyValueNotInList(asyncResp->res, *eventType,
                                                 "EventFormatType");
                return;
            }
            subValue.eventFormatType = *eventType;
        }
        else
        {
            // If not specified, use default "Event"
            subValue.eventFormatType.assign({"Event"});
        }

        if (context)
        {
            subValue.customText = *context;
        }

        if (headers)
        {
            subValue.httpHeaders = *headers;
        }

        if (regPrefixes)
        {
            for (const std::string& it : *regPrefixes)
            {
                if (std::find(supportedRegPrefixes.begin(),
                              supportedRegPrefixes.end(),
                              it) == supportedRegPrefixes.end())
                {
                    messages::propertyValueNotInList(asyncResp->res, it,
                                                     "RegistryPrefixes");
                    return;
                }
            }
            subValue.registryPrefixes = *regPrefixes;
        }

        if (msgIds)
        {
            // Do we need to loop-up MessageRegistry and validate
            // data for authenticity??? Not mandate, i believe.
            subValue.registryMsgIds = *msgIds;
        }

        if (retryPolicy)
        {
            if (std::find(supportedRetryPolicies.begin(),
                          supportedRetryPolicies.end(),
                          *retryPolicy) == supportedRetryPolicies.end())
            {
                messages::propertyValueNotInList(asyncResp->res, *retryPolicy,
                                                 "DeliveryRetryPolicy");
                return;
            }
            subValue.retryPolicy = *retryPolicy;
        }
        else
        {
            // Default "TerminateAfterRetries"
            subValue.retryPolicy = "TerminateAfterRetries";
        }

        if (metricReportDefinitions)
        {
            // TODO Validate
            subValue.metricReportDefinitions = *metricReportDefinitions;
        }

        std::string id =
            boost::uuids::to_string(boost::uuids::random_generator()());
        subscriptionsMap.insert(std::pair(id, subValue));

        messages::created(asyncResp->res);
        asyncResp->res.addHeader(
            "Location", "/redfish/v1/EventService/Subscriptions/" + id);

        updateSubscriptionData();
    }
};

class EventDestination : public Node
{
  public:
    EventDestination(CrowApp& app) :
        Node(app, "/redfish/v1/EventService/Subscriptions/<str>/",
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
        auto obj = subscriptionsMap.find(id);
        if (obj == subscriptionsMap.end())
        {
            res.result(boost::beast::http::status::not_found);
            res.end();
            return;
        }

        EventSrvSubscription& subValue = obj->second;

        res.jsonValue = {
            {"@odata.type", "#EventDestination.v1_6_0.EventDestination"},
            {"@odata.context",
             "/redfish/v1/$metadata#EventDestination.EventDestination"},
            {"Protocol", "Redfish"}};
        asyncResp->res.jsonValue["@odata.id"] =
            "/redfish/v1/EventService/Subscriptions/" + id;
        asyncResp->res.jsonValue["Id"] = id;
        asyncResp->res.jsonValue["Name"] = "Event Subscription " + id;
        asyncResp->res.jsonValue["Destination"] = subValue.destinationUrl;
        asyncResp->res.jsonValue["Context"] = subValue.customText;
        asyncResp->res.jsonValue["SubscriptionType"] =
            subValue.subscriptionType;
        asyncResp->res.jsonValue["HttpHeaders"] = subValue.httpHeaders;
        asyncResp->res.jsonValue["EventFormatType"] = subValue.eventFormatType;
        asyncResp->res.jsonValue["RegistryPrefixes"] =
            subValue.registryPrefixes;
        asyncResp->res.jsonValue["OriginResources"] = nlohmann::json::array();
        asyncResp->res.jsonValue["ResourceTypes"] = nlohmann::json::array();
        asyncResp->res.jsonValue["MessageIds"] = subValue.registryMsgIds;
        asyncResp->res.jsonValue["MetricReportDefinitions"] =
            subValue.metricReportDefinitions;
        asyncResp->res.jsonValue["DeliveryRetryPolicy"] = subValue.retryPolicy;
    }

    void doPatch(crow::Response& res, const crow::Request& req,
                 const std::vector<std::string>& params) override
    {
        auto asyncResp = std::make_shared<AsyncResp>(res);
        if (params.size() != 1)
        {
            messages::internalError(asyncResp->res);
            return;
        }

        const std::string& id = params[0];
        auto obj = subscriptionsMap.find(id);
        if (obj == subscriptionsMap.end())
        {
            res.result(boost::beast::http::status::not_found);
            res.end();
            return;
        }

        std::optional<std::string> context;
        std::optional<std::string> retryPolicy;
        std::optional<std::vector<nlohmann::json>> headers;

        if (!json_util::readJson(req, res, "Context", context,
                                 "DeliveryRetryPolicy", retryPolicy,
                                 "HttpHeaders", headers))
        {
            return;
        }

        EventSrvSubscription& subValue = obj->second;

        if (context)
        {
            subValue.customText = *context;
        }

        if (headers)
        {
            subValue.httpHeaders = *headers;
        }

        if (retryPolicy)
        {
            if (std::find(supportedRetryPolicies.begin(),
                          supportedRetryPolicies.end(),
                          *retryPolicy) == supportedRetryPolicies.end())
            {
                messages::propertyValueNotInList(asyncResp->res, *retryPolicy,
                                                 "DeliveryRetryPolicy");
                return;
            }
            subValue.retryPolicy = *retryPolicy;
        }

        updateSubscriptionData();
    }

    void doDelete(crow::Response& res, const crow::Request& req,
                  const std::vector<std::string>& params) override
    {
        auto asyncResp = std::make_shared<AsyncResp>(res);

        if (params.size() != 1)
        {
            messages::internalError(asyncResp->res);
            return;
        }

        const std::string& id = params[0];
        auto obj = subscriptionsMap.find(id);
        if (obj == subscriptionsMap.end())
        {
            res.result(boost::beast::http::status::not_found);
            res.end();
            return;
        }

        subscriptionsMap.erase(obj);

        updateSubscriptionData();
    }
};

} // namespace redfish
