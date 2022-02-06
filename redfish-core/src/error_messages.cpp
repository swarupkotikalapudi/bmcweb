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
#include "http_response.hpp"
#include "registries/base_message_registry.hpp"

#include <boost/beast/http/status.hpp>
#include <boost/url/url.hpp>
#include <error_messages.hpp>
#include <logging.hpp>
#include <nlohmann/json.hpp>
namespace redfish
{

namespace messages
{

static void addMessageToErrorJson(nlohmann::json& target,
                                  const nlohmann::json& message)
{
    auto& error = target["error"];

    // If this is the first error message, fill in the information from the
    // first error message to the top level struct
    if (!error.is_object())
    {
        auto messageIdIterator = message.find("MessageId");
        if (messageIdIterator == message.end())
        {
            BMCWEB_LOG_CRITICAL
                << "Attempt to add error message without MessageId";
            return;
        }

        auto messageFieldIterator = message.find("Message");
        if (messageFieldIterator == message.end())
        {
            BMCWEB_LOG_CRITICAL
                << "Attempt to add error message without Message";
            return;
        }
        error = {{"code", *messageIdIterator},
                 {"message", *messageFieldIterator}};
    }
    else
    {
        // More than 1 error occurred, so the message has to be generic
        error["code"] = std::string(messageVersionPrefix) + "GeneralError";
        error["message"] = "A general error has occurred. See Resolution for "
                           "information on how to resolve the error.";
    }

    // This check could technically be done in in the default construction
    // branch above, but because we need the pointer to the extended info field
    // anyway, it's more efficient to do it here.
    auto& extendedInfo = error[messages::messageAnnotation];
    if (!extendedInfo.is_array())
    {
        extendedInfo = nlohmann::json::array();
    }

    extendedInfo.push_back(message);
}

static void addMessageToJsonRoot(nlohmann::json& target,
                                 const nlohmann::json& message)
{
    if (!target[messages::messageAnnotation].is_array())
    {
        // Force object to be an array
        target[messages::messageAnnotation] = nlohmann::json::array();
    }

    target[messages::messageAnnotation].push_back(message);
}

static void addMessageToJson(nlohmann::json& target,
                             const nlohmann::json& message,
                             const std::string& fieldPath)
{
    std::string extendedInfo(fieldPath + messages::messageAnnotation);

    if (!target[extendedInfo].is_array())
    {
        // Force object to be an array
        target[extendedInfo] = nlohmann::json::array();
    }

    // Object exists and it is an array so we can just push in the message
    target[extendedInfo].push_back(message);
}

bool myfunction(const redfish::message_registries::MessageEntry& r,
                const std::string_view l)
{
    return l == r.first;
};

nlohmann::json getLog(const std::string_view name,
                      std::span<const std::string_view> args)
{
    auto it = std::lower_bound(
        redfish::message_registries::base::registry.begin(),
        redfish::message_registries::base::registry.end(), name, myfunction);
    if (it == redfish::message_registries::base::registry.end() ||
        it->first != name)
    {
        return {};
    }
    std::string msg = it->second.message;
    redfish::message_registries::fillMessageArgs(args, msg);

    return {{"@odata.type", "#Message.v1_1_1.Message"},
            {"MessageId", redfish::message_registries::base::header.id +
                              std::string(it->first)},
            {"Message", it->second.message},
            {"MessageArgs", nlohmann::json::array()},
            {"MessageSeverity", it->second.severity},
            {"Resolution", it->second.resolution}};
}

/**
 * @internal
 * @brief Formats ResourceInUse message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json resourceInUse(void)
{
    return getLog("ResourceInUse", {});
}

void resourceInUse(crow::Response& res)
{
    res.result(boost::beast::http::status::service_unavailable);
    addMessageToErrorJson(res.jsonValue, resourceInUse());
}

/**
 * @internal
 * @brief Formats MalformedJSON message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json malformedJSON(void)
{
    return getLog("MalformedJSON", {});
}

void malformedJSON(crow::Response& res)
{
    res.result(boost::beast::http::status::bad_request);
    addMessageToErrorJson(res.jsonValue, malformedJSON());
}

/**
 * @internal
 * @brief Formats ResourceMissingAtURI message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json resourceMissingAtURI(const boost::urls::url_view& arg1)
{
    std::array<std::string_view, 1> args{
        std::string_view{arg1.data(), arg1.size()}};
    return getLog("ResourceMissingAtURI", args);
}

void resourceMissingAtURI(crow::Response& res,
                          const boost::urls::url_view& arg1)
{
    res.result(boost::beast::http::status::bad_request);
    addMessageToErrorJson(res.jsonValue, resourceMissingAtURI(arg1));
}

/**
 * @internal
 * @brief Formats ActionParameterValueFormatError message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json actionParameterValueFormatError(const std::string& arg1,
                                               const std::string& arg2,
                                               const std::string& arg3)
{
    std::array<std::string_view, 3> args{arg1, arg2, arg3};
    return getLog("ActionParameterValueFormatError", args);
}

void actionParameterValueFormatError(crow::Response& res,
                                     const std::string& arg1,
                                     const std::string& arg2,
                                     const std::string& arg3)
{
    res.result(boost::beast::http::status::bad_request);
    addMessageToErrorJson(res.jsonValue,
                          actionParameterValueFormatError(arg1, arg2, arg3));
}

/**
 * @internal
 * @brief Formats InternalError message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json internalError(void)
{
    return getLog("ActionParameterValueFormatError", {});
}

void internalError(crow::Response& res, const bmcweb::source_location location)
{
    BMCWEB_LOG_CRITICAL << "Internal Error " << location.file_name() << "("
                        << location.line() << ":" << location.column() << ") `"
                        << location.function_name() << "`: ";
    res.result(boost::beast::http::status::internal_server_error);
    addMessageToErrorJson(res.jsonValue, internalError());
}

/**
 * @internal
 * @brief Formats UnrecognizedRequestBody message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json unrecognizedRequestBody(void)
{
    return getLog("UnrecognizedRequestBody", {});
}

void unrecognizedRequestBody(crow::Response& res)
{
    res.result(boost::beast::http::status::bad_request);
    addMessageToErrorJson(res.jsonValue, unrecognizedRequestBody());
}

/**
 * @internal
 * @brief Formats ResourceAtUriUnauthorized message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json resourceAtUriUnauthorized(const boost::urls::url_view& arg1,
                                         const std::string& arg2)
{
    std::array<std::string_view, 2> args{
        std::string_view{arg1.data(), arg1.size()}, arg2};
    return getLog("ResourceAtUriUnauthorized", args);
}

void resourceAtUriUnauthorized(crow::Response& res,
                               const boost::urls::url_view& arg1,
                               const std::string& arg2)
{
    res.result(boost::beast::http::status::unauthorized);
    addMessageToErrorJson(res.jsonValue, resourceAtUriUnauthorized(arg1, arg2));
}

/**
 * @internal
 * @brief Formats ActionParameterUnknown message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json actionParameterUnknown(const std::string& arg1,
                                      const std::string& arg2)
{
    std::array<std::string_view, 3> args{arg1, arg2};
    return getLog("ActionParameterUnknown", args);
}

void actionParameterUnknown(crow::Response& res, const std::string& arg1,
                            const std::string& arg2)
{
    res.result(boost::beast::http::status::bad_request);
    addMessageToErrorJson(res.jsonValue, actionParameterUnknown(arg1, arg2));
}

/**
 * @internal
 * @brief Formats ResourceCannotBeDeleted message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json resourceCannotBeDeleted(void)
{
    return getLog("ResourceCannotBeDeleted", {});
}

void resourceCannotBeDeleted(crow::Response& res)
{
    res.result(boost::beast::http::status::forbidden);
    addMessageToErrorJson(res.jsonValue, resourceCannotBeDeleted());
}

/**
 * @internal
 * @brief Formats PropertyDuplicate message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json propertyDuplicate(const std::string& arg1)
{
    std::array<std::string_view, 1> args{arg1};
    return getLog("PropertyDuplicate", args);
}

void propertyDuplicate(crow::Response& res, const std::string& arg1)
{
    res.result(boost::beast::http::status::bad_request);
    addMessageToJson(res.jsonValue, propertyDuplicate(arg1), arg1);
}

/**
 * @internal
 * @brief Formats ServiceTemporarilyUnavailable message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json serviceTemporarilyUnavailable(const std::string& arg1)
{
    std::array<std::string_view, 1> args{arg1};
    return getLog("ServiceTemporarilyUnavailable", args);
}

void serviceTemporarilyUnavailable(crow::Response& res, const std::string& arg1)
{
    res.addHeader("Retry-After", arg1);
    res.result(boost::beast::http::status::service_unavailable);
    addMessageToErrorJson(res.jsonValue, serviceTemporarilyUnavailable(arg1));
}

/**
 * @internal
 * @brief Formats ResourceAlreadyExists message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json resourceAlreadyExists(const std::string& arg1,
                                     const std::string& arg2,
                                     const std::string& arg3)
{
    std::array<std::string_view, 3> args{arg1, arg2, arg3};
    return getLog("ResourceAlreadyExists", args);
}

void resourceAlreadyExists(crow::Response& res, const std::string& arg1,
                           const std::string& arg2, const std::string& arg3)
{
    res.result(boost::beast::http::status::bad_request);
    addMessageToJson(res.jsonValue, resourceAlreadyExists(arg1, arg2, arg3),
                     arg2);
}

/**
 * @internal
 * @brief Formats AccountForSessionNoLongerExists message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json accountForSessionNoLongerExists(void)
{
    return getLog("AccountForSessionNoLongerExists", {});
}

void accountForSessionNoLongerExists(crow::Response& res)
{
    res.result(boost::beast::http::status::forbidden);
    addMessageToErrorJson(res.jsonValue, accountForSessionNoLongerExists());
}

/**
 * @internal
 * @brief Formats CreateFailedMissingReqProperties message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json createFailedMissingReqProperties(const std::string& arg1)
{
    std::array<std::string_view, 2> args{arg1};
    return getLog("CreateFailedMissingReqProperties", args);
}

void createFailedMissingReqProperties(crow::Response& res,
                                      const std::string& arg1)
{
    res.result(boost::beast::http::status::bad_request);
    addMessageToJson(res.jsonValue, createFailedMissingReqProperties(arg1),
                     arg1);
}

/**
 * @internal
 * @brief Formats PropertyValueFormatError message into JSON for the specified
 * property
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json propertyValueFormatError(const std::string& arg1,
                                        const std::string& arg2)
{
    std::array<std::string_view, 2> args{arg1, arg2};
    return getLog("PropertyValueFormatError", args);
}

void propertyValueFormatError(crow::Response& res, const std::string& arg1,
                              const std::string& arg2)
{
    res.result(boost::beast::http::status::bad_request);
    addMessageToJson(res.jsonValue, propertyValueFormatError(arg1, arg2), arg2);
}

/**
 * @internal
 * @brief Formats PropertyValueNotInList message into JSON for the specified
 * property
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json propertyValueNotInList(const std::string& arg1,
                                      const std::string& arg2)
{
    std::array<std::string_view, 2> args{arg1, arg2};
    return getLog("PropertyValueNotInList", args);
}

void propertyValueNotInList(crow::Response& res, const std::string& arg1,
                            const std::string& arg2)
{
    res.result(boost::beast::http::status::bad_request);
    addMessageToJson(res.jsonValue, propertyValueNotInList(arg1, arg2), arg2);
}

/**
 * @internal
 * @brief Formats ResourceAtUriInUnknownFormat message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json resourceAtUriInUnknownFormat(const boost::urls::url_view& arg1)
{
    std::array<std::string_view, 1> args{
        std::string_view{arg1.data(), arg1.size()}};
    return getLog("ResourceAtUriInUnknownFormat", args);
}

void resourceAtUriInUnknownFormat(crow::Response& res,
                                  const boost::urls::url_view& arg1)
{
    res.result(boost::beast::http::status::bad_request);
    addMessageToErrorJson(res.jsonValue, resourceAtUriInUnknownFormat(arg1));
}

/**
 * @internal
 * @brief Formats ServiceDisabled message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json serviceDisabled(const std::string& arg1)
{
    std::array<std::string_view, 1> args{arg1};
    return getLog("ServiceDisabled", args);
}

void serviceDisabled(crow::Response& res, const std::string& arg1)
{
    res.result(boost::beast::http::status::service_unavailable);
    addMessageToErrorJson(res.jsonValue, serviceDisabled(arg1));
}

/**
 * @internal
 * @brief Formats ServiceInUnknownState message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json serviceInUnknownState(void)
{
    return getLog("ServiceInUnknownState", {});
}

void serviceInUnknownState(crow::Response& res)
{
    res.result(boost::beast::http::status::service_unavailable);
    addMessageToErrorJson(res.jsonValue, serviceInUnknownState());
}

/**
 * @internal
 * @brief Formats EventSubscriptionLimitExceeded message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json eventSubscriptionLimitExceeded(void)
{
    return getLog("EventSubscriptionLimitExceeded", {});
}

void eventSubscriptionLimitExceeded(crow::Response& res)
{
    res.result(boost::beast::http::status::service_unavailable);
    addMessageToErrorJson(res.jsonValue, eventSubscriptionLimitExceeded());
}

/**
 * @internal
 * @brief Formats ActionParameterMissing message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json actionParameterMissing(const std::string& arg1,
                                      const std::string& arg2)
{
    std::array<std::string_view, 2> args{arg1, arg2};
    return getLog("ActionParameterMissing", args);
}

void actionParameterMissing(crow::Response& res, const std::string& arg1,
                            const std::string& arg2)
{
    res.result(boost::beast::http::status::bad_request);
    addMessageToErrorJson(res.jsonValue, actionParameterMissing(arg1, arg2));
}

/**
 * @internal
 * @brief Formats StringValueTooLong message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json stringValueTooLong(const std::string& arg1, int arg2)
{
    std::string arg2String = std::to_string(arg2);
    std::array<std::string_view, 2> args{arg1, arg2String};
    return getLog("StringValueTooLong", args);
}

void stringValueTooLong(crow::Response& res, const std::string& arg1, int arg2)
{
    res.result(boost::beast::http::status::bad_request);
    addMessageToErrorJson(res.jsonValue, stringValueTooLong(arg1, arg2));
}

/**
 * @internal
 * @brief Formats SessionTerminated message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json sessionTerminated(void)
{
    return getLog("SessionTerminated", {});
}

void sessionTerminated(crow::Response& res)
{
    res.result(boost::beast::http::status::ok);
    addMessageToJsonRoot(res.jsonValue, sessionTerminated());
}

/**
 * @internal
 * @brief Formats SubscriptionTerminated message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json subscriptionTerminated(void)
{
    return getLog("SubscriptionTerminated", {});
}

void subscriptionTerminated(crow::Response& res)
{
    res.result(boost::beast::http::status::ok);
    addMessageToJsonRoot(res.jsonValue, subscriptionTerminated());
}

/**
 * @internal
 * @brief Formats ResourceTypeIncompatible message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json resourceTypeIncompatible(const std::string& arg1,
                                        const std::string& arg2)
{
    std::array<std::string_view, 2> args{arg1, arg2};
    return getLog("ResourceTypeIncompatible", args);
}

void resourceTypeIncompatible(crow::Response& res, const std::string& arg1,
                              const std::string& arg2)
{
    res.result(boost::beast::http::status::bad_request);
    addMessageToErrorJson(res.jsonValue, resourceTypeIncompatible(arg1, arg2));
}

/**
 * @internal
 * @brief Formats ResetRequired message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json resetRequired(const boost::urls::url_view& arg1,
                             const std::string& arg2)
{
    std::array<std::string_view, 2> args{
        std::string_view(arg1.data(), arg1.size()), arg2};
    return getLog("ResetRequired", args);
}

void resetRequired(crow::Response& res, const boost::urls::url_view& arg1,
                   const std::string& arg2)
{
    res.result(boost::beast::http::status::bad_request);
    addMessageToErrorJson(res.jsonValue, resetRequired(arg1, arg2));
}

/**
 * @internal
 * @brief Formats ChassisPowerStateOnRequired message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json chassisPowerStateOnRequired(const std::string& arg1)
{
    std::array<std::string_view, 1> args{arg1};
    return getLog("ResetRequired", args);
}

void chassisPowerStateOnRequired(crow::Response& res, const std::string& arg1)
{
    res.result(boost::beast::http::status::bad_request);
    addMessageToErrorJson(res.jsonValue, chassisPowerStateOnRequired(arg1));
}

/**
 * @internal
 * @brief Formats ChassisPowerStateOffRequired message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json chassisPowerStateOffRequired(const std::string& arg1)
{
    std::array<std::string_view, 1> args{arg1};
    return getLog("ChassisPowerStateOffRequired", args);
}

void chassisPowerStateOffRequired(crow::Response& res, const std::string& arg1)
{
    res.result(boost::beast::http::status::bad_request);
    addMessageToErrorJson(res.jsonValue, chassisPowerStateOffRequired(arg1));
}

/**
 * @internal
 * @brief Formats PropertyValueConflict message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json propertyValueConflict(const std::string& arg1,
                                     const std::string& arg2)
{
    std::array<std::string_view, 2> args{arg1, arg2};
    return getLog("PropertyValueConflict", args);
}

void propertyValueConflict(crow::Response& res, const std::string& arg1,
                           const std::string& arg2)
{
    res.result(boost::beast::http::status::bad_request);
    addMessageToErrorJson(res.jsonValue, propertyValueConflict(arg1, arg2));
}

/**
 * @internal
 * @brief Formats PropertyValueIncorrect message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json propertyValueIncorrect(const std::string& arg1,
                                      const std::string& arg2)
{
    std::array<std::string_view, 2> args{arg1, arg2};
    return getLog("PropertyValueIncorrect", args);
}

void propertyValueIncorrect(crow::Response& res, const std::string& arg1,
                            const std::string& arg2)
{
    res.result(boost::beast::http::status::bad_request);
    addMessageToErrorJson(res.jsonValue, propertyValueIncorrect(arg1, arg2));
}

/**
 * @internal
 * @brief Formats ResourceCreationConflict message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json resourceCreationConflict(const boost::urls::url_view& arg1)
{

    std::array<std::string_view, 1> args{
        std::string_view(arg1.data(), arg1.size())};
    return getLog("ResourceCreationConflict", args);
}

void resourceCreationConflict(crow::Response& res,
                              const boost::urls::url_view& arg1)
{
    res.result(boost::beast::http::status::bad_request);
    addMessageToErrorJson(res.jsonValue, resourceCreationConflict(arg1));
}

/**
 * @internal
 * @brief Formats MaximumErrorsExceeded message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json maximumErrorsExceeded(void)
{
    return getLog("MaximumErrorsExceeded", {});
}

void maximumErrorsExceeded(crow::Response& res)
{
    res.result(boost::beast::http::status::internal_server_error);
    addMessageToErrorJson(res.jsonValue, maximumErrorsExceeded());
}

/**
 * @internal
 * @brief Formats PreconditionFailed message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json preconditionFailed(void)
{
    return getLog("PreconditionFailed", {});
}

void preconditionFailed(crow::Response& res)
{
    res.result(boost::beast::http::status::precondition_failed);
    addMessageToErrorJson(res.jsonValue, preconditionFailed());
}

/**
 * @internal
 * @brief Formats PreconditionRequired message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json preconditionRequired(void)
{
    return getLog("PreconditionRequired", {});
}

void preconditionRequired(crow::Response& res)
{
    res.result(boost::beast::http::status::bad_request);
    addMessageToErrorJson(res.jsonValue, preconditionRequired());
}

/**
 * @internal
 * @brief Formats OperationFailed message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json operationFailed(void)
{
    return getLog("OperationFailed", {});
}

void operationFailed(crow::Response& res)
{
    res.result(boost::beast::http::status::internal_server_error);
    addMessageToErrorJson(res.jsonValue, operationFailed());
}

/**
 * @internal
 * @brief Formats OperationTimeout message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json operationTimeout(void)
{
    return getLog("OperationTimeout", {});
}

void operationTimeout(crow::Response& res)
{
    res.result(boost::beast::http::status::internal_server_error);
    addMessageToErrorJson(res.jsonValue, operationTimeout());
}

/**
 * @internal
 * @brief Formats PropertyValueTypeError message into JSON for the specified
 * property
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json propertyValueTypeError(const std::string& arg1,
                                      const std::string& arg2)
{
    std::array<std::string_view, 2> args{arg1, arg2};
    return getLog("PropertyValueTypeError", args);
}

void propertyValueTypeError(crow::Response& res, const std::string& arg1,
                            const std::string& arg2)
{
    res.result(boost::beast::http::status::bad_request);
    addMessageToJson(res.jsonValue, propertyValueTypeError(arg1, arg2), arg2);
}

/**
 * @internal
 * @brief Formats ResourceNotFound message into JSONd
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json resourceNotFound(const std::string& arg1,
                                const std::string& arg2)
{
    std::array<std::string_view, 2> args{arg1, arg2};
    return getLog("ResourceNotFound", args);
}

void resourceNotFound(crow::Response& res, const std::string& arg1,
                      const std::string& arg2)
{
    res.result(boost::beast::http::status::not_found);
    addMessageToErrorJson(res.jsonValue, resourceNotFound(arg1, arg2));
}

/**
 * @internal
 * @brief Formats CouldNotEstablishConnection message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json couldNotEstablishConnection(const boost::urls::url_view& arg1)
{

    std::array<std::string_view, 1> args{
        std::string_view(arg1.data(), arg1.size())};
    return getLog("CouldNotEstablishConnection", args);
}

void couldNotEstablishConnection(crow::Response& res,
                                 const boost::urls::url_view& arg1)
{
    res.result(boost::beast::http::status::not_found);
    addMessageToErrorJson(res.jsonValue, couldNotEstablishConnection(arg1));
}

/**
 * @internal
 * @brief Formats PropertyNotWritable message into JSON for the specified
 * property
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json propertyNotWritable(const std::string& arg1)
{
    std::array<std::string_view, 1> args{arg1};
    return getLog("PropertyNotWritable", args);
}

void propertyNotWritable(crow::Response& res, const std::string& arg1)
{
    res.result(boost::beast::http::status::forbidden);
    addMessageToJson(res.jsonValue, propertyNotWritable(arg1), arg1);
}

/**
 * @internal
 * @brief Formats QueryParameterValueTypeError message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json queryParameterValueTypeError(const std::string& arg1,
                                            const std::string& arg2)
{
    std::array<std::string_view, 2> args{arg1, arg2};
    return getLog("QueryParameterValueTypeError", args);
}

void queryParameterValueTypeError(crow::Response& res, const std::string& arg1,
                                  const std::string& arg2)
{
    res.result(boost::beast::http::status::bad_request);
    addMessageToErrorJson(res.jsonValue,
                          queryParameterValueTypeError(arg1, arg2));
}

/**
 * @internal
 * @brief Formats ServiceShuttingDown message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json serviceShuttingDown(void)
{
    return getLog("ServiceShuttingDown", {});
}

void serviceShuttingDown(crow::Response& res)
{
    res.result(boost::beast::http::status::service_unavailable);
    addMessageToErrorJson(res.jsonValue, serviceShuttingDown());
}

/**
 * @internal
 * @brief Formats ActionParameterDuplicate message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json actionParameterDuplicate(const std::string& arg1,
                                        const std::string& arg2)
{
    std::array<std::string_view, 2> args{arg1, arg2};
    return getLog("ActionParameterDuplicate", args);
}

void actionParameterDuplicate(crow::Response& res, const std::string& arg1,
                              const std::string& arg2)
{
    res.result(boost::beast::http::status::bad_request);
    addMessageToErrorJson(res.jsonValue, actionParameterDuplicate(arg1, arg2));
}

/**
 * @internal
 * @brief Formats ActionParameterNotSupported message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json actionParameterNotSupported(const std::string& arg1,
                                           const std::string& arg2)
{
    std::array<std::string_view, 2> args{arg1, arg2};
    return getLog("ActionParameterNotSupported", args);
}

void actionParameterNotSupported(crow::Response& res, const std::string& arg1,
                                 const std::string& arg2)
{
    res.result(boost::beast::http::status::bad_request);
    addMessageToErrorJson(res.jsonValue,
                          actionParameterNotSupported(arg1, arg2));
}

/**
 * @internal
 * @brief Formats SourceDoesNotSupportProtocol message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json sourceDoesNotSupportProtocol(const boost::urls::url_view& arg1,
                                            const std::string& arg2)
{
    std::array<std::string_view, 2> args{
        std::string_view(arg1.data(), arg1.size()), arg2};
    return getLog("SourceDoesNotSupportProtocol", args);
}

void sourceDoesNotSupportProtocol(crow::Response& res,
                                  const boost::urls::url_view& arg1,
                                  const std::string& arg2)
{
    res.result(boost::beast::http::status::bad_request);
    addMessageToErrorJson(res.jsonValue,
                          sourceDoesNotSupportProtocol(arg1, arg2));
}

/**
 * @internal
 * @brief Formats AccountRemoved message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json accountRemoved(void)
{
    return getLog("AccountRemoved", {});
}

void accountRemoved(crow::Response& res)
{
    res.result(boost::beast::http::status::ok);
    addMessageToJsonRoot(res.jsonValue, accountRemoved());
}

/**
 * @internal
 * @brief Formats AccessDenied message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json accessDenied(const boost::urls::url_view& arg1)
{
    std::array<std::string_view, 1> args{
        std::string_view(arg1.data(), arg1.size())};
    return getLog("AccessDenied", args);
}

void accessDenied(crow::Response& res, const boost::urls::url_view& arg1)
{
    res.result(boost::beast::http::status::forbidden);
    addMessageToErrorJson(res.jsonValue, accessDenied(arg1));
}

/**
 * @internal
 * @brief Formats QueryNotSupported message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json queryNotSupported(void)
{
    return getLog("QueryNotSupported", {});
}

void queryNotSupported(crow::Response& res)
{
    res.result(boost::beast::http::status::bad_request);
    addMessageToErrorJson(res.jsonValue, queryNotSupported());
}

/**
 * @internal
 * @brief Formats CreateLimitReachedForResource message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json createLimitReachedForResource(void)
{
    return getLog("CreateLimitReachedForResource", {});
}

void createLimitReachedForResource(crow::Response& res)
{
    res.result(boost::beast::http::status::bad_request);
    addMessageToErrorJson(res.jsonValue, createLimitReachedForResource());
}

/**
 * @internal
 * @brief Formats GeneralError message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json generalError(void)
{
    return getLog("GeneralError", {});
}

void generalError(crow::Response& res)
{
    res.result(boost::beast::http::status::internal_server_error);
    addMessageToErrorJson(res.jsonValue, generalError());
}

/**
 * @internal
 * @brief Formats Success message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json success(void)
{
    return getLog("Success", {});
}

void success(crow::Response& res)
{
    // don't set res.result here because success is the default and any
    // error should overwrite the default
    addMessageToJsonRoot(res.jsonValue, success());
}

/**
 * @internal
 * @brief Formats Created message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json created(void)
{
    return getLog("Created", {});
}

void created(crow::Response& res)
{
    res.result(boost::beast::http::status::created);
    addMessageToJsonRoot(res.jsonValue, created());
}

/**
 * @internal
 * @brief Formats NoOperation message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json noOperation(void)
{
    return getLog("NoOperation", {});
}

void noOperation(crow::Response& res)
{
    res.result(boost::beast::http::status::bad_request);
    addMessageToErrorJson(res.jsonValue, noOperation());
}

/**
 * @internal
 * @brief Formats PropertyUnknown message into JSON for the specified
 * property
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json propertyUnknown(const std::string& arg1)
{
    std::array<std::string_view, 1> args{arg1};
    return getLog("PropertyUnknown", args);
}

void propertyUnknown(crow::Response& res, const std::string& arg1)
{
    res.result(boost::beast::http::status::bad_request);
    addMessageToJson(res.jsonValue, propertyUnknown(arg1), arg1);
}

/**
 * @internal
 * @brief Formats NoValidSession message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json noValidSession(void)
{
    return getLog("NoValidSession", {});
}

void noValidSession(crow::Response& res)
{
    res.result(boost::beast::http::status::forbidden);
    addMessageToErrorJson(res.jsonValue, noValidSession());
}

/**
 * @internal
 * @brief Formats InvalidObject message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json invalidObject(const boost::urls::url_view& arg1)
{
    std::array<std::string_view, 1> args{
        std::string_view(arg1.data(), arg1.size())};
    return getLog("InvalidObject", args);
}

void invalidObject(crow::Response& res, const boost::urls::url_view& arg1)
{
    res.result(boost::beast::http::status::bad_request);
    addMessageToErrorJson(res.jsonValue, invalidObject(arg1));
}

/**
 * @internal
 * @brief Formats ResourceInStandby message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json resourceInStandby(void)
{
    return getLog("ResourceInStandby", {});
}

void resourceInStandby(crow::Response& res)
{
    res.result(boost::beast::http::status::service_unavailable);
    addMessageToErrorJson(res.jsonValue, resourceInStandby());
}

/**
 * @internal
 * @brief Formats ActionParameterValueTypeError message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json actionParameterValueTypeError(const std::string& arg1,
                                             const std::string& arg2,
                                             const std::string& arg3)
{
    std::array<std::string_view, 3> args{arg1, arg2, arg3};
    return getLog("ActionParameterValueTypeError", args);
}

void actionParameterValueTypeError(crow::Response& res, const std::string& arg1,
                                   const std::string& arg2,
                                   const std::string& arg3)
{
    res.result(boost::beast::http::status::bad_request);
    addMessageToErrorJson(res.jsonValue,
                          actionParameterValueTypeError(arg1, arg2, arg3));
}

/**
 * @internal
 * @brief Formats SessionLimitExceeded message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json sessionLimitExceeded(void)
{
    return getLog("SessionLimitExceeded", {});
}

void sessionLimitExceeded(crow::Response& res)
{
    res.result(boost::beast::http::status::service_unavailable);
    addMessageToErrorJson(res.jsonValue, sessionLimitExceeded());
}

/**
 * @internal
 * @brief Formats ActionNotSupported message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json actionNotSupported(const std::string& arg1)
{
    std::array<std::string_view, 1> args{arg1};
    return getLog("ActionNotSupported", args);
}

void actionNotSupported(crow::Response& res, const std::string& arg1)
{
    res.result(boost::beast::http::status::bad_request);
    addMessageToErrorJson(res.jsonValue, actionNotSupported(arg1));
}

/**
 * @internal
 * @brief Formats InvalidIndex message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json invalidIndex(int64_t arg1)
{
    std::string arg1Str = std::to_string(arg1);
    std::array<std::string_view, 1> args{arg1Str};
    return getLog("InvalidIndex", args);
}

void invalidIndex(crow::Response& res, int64_t arg1)
{
    res.result(boost::beast::http::status::bad_request);
    addMessageToErrorJson(res.jsonValue, invalidIndex(arg1));
}

/**
 * @internal
 * @brief Formats EmptyJSON message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json emptyJSON(void)
{
    return getLog("EmptyJSON", {});
}

void emptyJSON(crow::Response& res)
{
    res.result(boost::beast::http::status::bad_request);
    addMessageToErrorJson(res.jsonValue, emptyJSON());
}

/**
 * @internal
 * @brief Formats QueryNotSupportedOnResource message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json queryNotSupportedOnResource(void)
{
    return getLog("QueryNotSupportedOnResource", {});
}

void queryNotSupportedOnResource(crow::Response& res)
{
    res.result(boost::beast::http::status::forbidden);
    addMessageToErrorJson(res.jsonValue, queryNotSupportedOnResource());
}

/**
 * @internal
 * @brief Formats QueryNotSupportedOnOperation message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json queryNotSupportedOnOperation(void)
{
    return getLog("QueryNotSupportedOnOperation", {});
}

void queryNotSupportedOnOperation(crow::Response& res)
{
    res.result(boost::beast::http::status::forbidden);
    addMessageToErrorJson(res.jsonValue, queryNotSupportedOnOperation());
}

/**
 * @internal
 * @brief Formats QueryCombinationInvalid message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json queryCombinationInvalid(void)
{
    return getLog("QueryCombinationInvalid", {});
}

void queryCombinationInvalid(crow::Response& res)
{
    res.result(boost::beast::http::status::bad_request);
    addMessageToErrorJson(res.jsonValue, queryCombinationInvalid());
}

/**
 * @internal
 * @brief Formats InsufficientPrivilege message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json insufficientPrivilege(void)
{
    return getLog("InsufficientPrivilege", {});
}

void insufficientPrivilege(crow::Response& res)
{
    res.result(boost::beast::http::status::forbidden);
    addMessageToErrorJson(res.jsonValue, insufficientPrivilege());
}

/**
 * @internal
 * @brief Formats PropertyValueModified message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json propertyValueModified(const std::string& arg1,
                                     const std::string& arg2)
{
    std::array<std::string_view, 2> args{arg1, arg2};
    return getLog("PropertyValueModified", args);
}

void propertyValueModified(crow::Response& res, const std::string& arg1,
                           const std::string& arg2)
{
    res.result(boost::beast::http::status::ok);
    addMessageToJson(res.jsonValue, propertyValueModified(arg1, arg2), arg1);
}

/**
 * @internal
 * @brief Formats AccountNotModified message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json accountNotModified(void)
{
    return getLog("AccountNotModified", {});
}

void accountNotModified(crow::Response& res)
{
    res.result(boost::beast::http::status::bad_request);
    addMessageToErrorJson(res.jsonValue, accountNotModified());
}

/**
 * @internal
 * @brief Formats QueryParameterValueFormatError message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json queryParameterValueFormatError(const std::string& arg1,
                                              const std::string& arg2)
{
    std::array<std::string_view, 2> args{arg1, arg2};
    return getLog("QueryParameterValueFormatError", args);
}

void queryParameterValueFormatError(crow::Response& res,
                                    const std::string& arg1,
                                    const std::string& arg2)
{
    res.result(boost::beast::http::status::bad_request);
    addMessageToErrorJson(res.jsonValue,
                          queryParameterValueFormatError(arg1, arg2));
}

/**
 * @internal
 * @brief Formats PropertyMissing message into JSON for the specified
 * property
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json propertyMissing(const std::string& arg1)
{
    std::array<std::string_view, 1> args{arg1};
    return getLog("PropertyMissing", args);
}

void propertyMissing(crow::Response& res, const std::string& arg1)
{
    res.result(boost::beast::http::status::bad_request);
    addMessageToJson(res.jsonValue, propertyMissing(arg1), arg1);
}

/**
 * @internal
 * @brief Formats ResourceExhaustion message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json resourceExhaustion(const std::string& arg1)
{
    std::array<std::string_view, 1> args{arg1};
    return getLog("ResourceExhaustion", args);
}

void resourceExhaustion(crow::Response& res, const std::string& arg1)
{
    res.result(boost::beast::http::status::service_unavailable);
    addMessageToErrorJson(res.jsonValue, resourceExhaustion(arg1));
}

/**
 * @internal
 * @brief Formats AccountModified message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json accountModified(void)
{
    return getLog("AccountModified", {});
}

void accountModified(crow::Response& res)
{
    res.result(boost::beast::http::status::ok);
    addMessageToErrorJson(res.jsonValue, accountModified());
}

/**
 * @internal
 * @brief Formats QueryParameterOutOfRange message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json queryParameterOutOfRange(const std::string& arg1,
                                        const std::string& arg2,
                                        const std::string& arg3)
{
    std::array<std::string_view, 3> args{arg1, arg2, arg3};
    return getLog("QueryParameterOutOfRange", args);
}

void queryParameterOutOfRange(crow::Response& res, const std::string& arg1,
                              const std::string& arg2, const std::string& arg3)
{
    res.result(boost::beast::http::status::bad_request);
    addMessageToErrorJson(res.jsonValue,
                          queryParameterOutOfRange(arg1, arg2, arg3));
}

nlohmann::json passwordChangeRequired(const boost::urls::url_view& arg1)
{
    std::array<std::string_view, 1> args{
        std::string_view(arg1.data(), arg1.size())};

    return getLog("PasswordChangeRequired", args);
}

/**
 * @internal
 * @brief Formats PasswordChangeRequired message into JSON
 *
 * See header file for more information
 * @endinternal
 */
void passwordChangeRequired(crow::Response& res,
                            const boost::urls::url_view& arg1)
{
    messages::addMessageToJsonRoot(res.jsonValue, passwordChangeRequired(arg1));
}

void invalidUpload(crow::Response& res, const std::string& arg1,
                   const std::string& arg2)
{
    res.result(boost::beast::http::status::bad_request);
    addMessageToErrorJson(res.jsonValue, invalidUpload(arg1, arg2));
}

/**
 * @internal
 * @brief Formats Invalid File message into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json invalidUpload(const std::string& arg1, const std::string& arg2)
{
    std::array<std::string_view, 2> args{arg1, arg2};
    return getLog("InvalidUpload", args);
}

/**
 * @internal
 * @brief Formats MutualExclusiveProperties into JSON
 *
 * See header file for more information
 * @endinternal
 */
nlohmann::json mutualExclusiveProperties(const std::string& arg1,
                                         const std::string& arg2)
{
    std::array<std::string_view, 2> args{arg1, arg2};
    return getLog("MutualExclusiveProperties", args);
}

void mutualExclusiveProperties(crow::Response& res, const std::string& arg1,
                               const std::string& arg2)
{
    res.result(boost::beast::http::status::bad_request);
    addMessageToErrorJson(res.jsonValue, mutualExclusiveProperties(arg1, arg2));
}

} // namespace messages

} // namespace redfish
