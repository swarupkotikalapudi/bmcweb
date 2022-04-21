#pragma once

#include <app.hpp>
#include <async_resp.hpp>
#include <error_messages.hpp>
#include <nlohmann/json.hpp>
#include <utils/collection.hpp>
#include <utils/hex_utils.hpp>
#include <utils/json_utils.hpp>

#include <vector>

namespace crow
{
namespace google_api
{
constexpr const char* hothSearchPath = "/xyz/openbmc_project";
constexpr const char* hothInterface = "xyz.openbmc_project.Control.Hoth";
constexpr const char* rotCollectionPrefix = "/google/v1/RootOfTrustCollection";

inline void getGoogleV1(const crow::Request& /*req*/,
                        const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    asyncResp->res.jsonValue["@odata.type"] =
        "#GoogleServiceRoot.v1_0_0.GoogleServiceRoot";
    asyncResp->res.jsonValue["@odata.id"] = "/google/v1";
    asyncResp->res.jsonValue["Id"] = "Google Rest RootService";
    asyncResp->res.jsonValue["Name"] = "Google Service Root";
    asyncResp->res.jsonValue["Version"] = "1.0.0";
    asyncResp->res.jsonValue["RootOfTrustCollection"]["@odata.id"] =
        rotCollectionPrefix;
}

inline void getRootOfTrustCollection(
    const crow::Request& /*req*/,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    asyncResp->res.jsonValue["@odata.id"] = rotCollectionPrefix;
    asyncResp->res.jsonValue["@odata.type"] =
        "#RootOfTrustCollection.RootOfTrustCollection";
    redfish::collection_util::getCollectionMembers(
        asyncResp, rotCollectionPrefix, std::vector<const char*>{hothInterface},
        hothSearchPath);
}

// Helper struct to identify a resolved D-Bus object interface
struct ResolvedEntity
{
    std::string id;
    std::string service;
    std::string object;
    const char* interface;
};

using ResolvedEntityHandler = std::function<void(
    const std::string&, const std::shared_ptr<bmcweb::AsyncResp>&,
    const ResolvedEntity&)>;

inline void resolveRoT(const std::string& command,
                       const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                       const std::string& rotId,
                       ResolvedEntityHandler&& entityHandler)
{
    auto validateFunc = [command, asyncResp, rotId,
                         entityHandler{std::forward<ResolvedEntityHandler>(
                             entityHandler)}](
                            const boost::system::error_code ec,
                            const crow::openbmc_mapper::GetSubTreeType&
                                subtree) {
        if (ec)
        {
            redfish::messages::internalError(asyncResp->res);
            return;
        }
        // Iterate over all retrieved ObjectPaths.
        for (const std::pair<
                 std::string,
                 std::vector<std::pair<std::string, std::vector<std::string>>>>&
                 object : subtree)
        {
            sdbusplus::message::object_path objPath(object.first);
            if (objPath.filename() != rotId || object.second.empty())
            {
                continue;
            }

            ResolvedEntity resolvedEntity = {.id = rotId,
                                             .service = object.second[0].first,
                                             .object = object.first,
                                             .interface = hothInterface};
            entityHandler(command, asyncResp, resolvedEntity);
            return;
        }

        // Couldn't find an object with that name.  return an error
        redfish::messages::resourceNotFound(
            asyncResp->res, "#RootOfTrust.v1_0_0.RootOfTrust", rotId);
    };

    std::array<std::string, 1> hothIfaces = {hothInterface};
    crow::connections::systemBus->async_method_call(
        validateFunc, "xyz.openbmc_project.ObjectMapper",
        "/xyz/openbmc_project/object_mapper",
        "xyz.openbmc_project.ObjectMapper", "GetSubTree", hothSearchPath,
        /*depth=*/0, hothIfaces);
}

inline void populateRootOfTrustEntity(
    const std::string& /*command=*/,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const ResolvedEntity& resolvedEntity)
{
    asyncResp->res.jsonValue["@odata.type"] = "#RootOfTrust.v1_0_0.RootOfTrust";
    asyncResp->res.jsonValue["@odata.id"] =
        "/google/v1/RootOfTrustCollection/" + resolvedEntity.id;

    asyncResp->res.jsonValue["Status"]["State"] = "Enabled";
    asyncResp->res.jsonValue["Id"] = resolvedEntity.id;
    // Need to fix this later to a stabler property.
    asyncResp->res.jsonValue["Name"] = resolvedEntity.id;
    asyncResp->res.jsonValue["Description"] = "Google Root Of Trust";
    asyncResp->res.jsonValue["Actions"]["#RootOfTrust.SendCommand"]["target"] =
        "/google/v1/RootOfTrustCollection/" + resolvedEntity.id +
        "/Actions/RootOfTrust.SendCommand";

    asyncResp->res.jsonValue["Location"]["PartLocation"]["ServiceLabel"] =
        resolvedEntity.id;
    asyncResp->res.jsonValue["Location"]["PartLocation"]["LocationType"] =
        "Embedded";
}

inline void getRootOfTrust(const crow::Request&,
                           const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                           const std::string& param)
{
    resolveRoT("" /*Empty command*/, asyncResp, param,
               populateRootOfTrustEntity);
}

inline void
    invokeRoTCommand(const std::string& command,
                     const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                     const ResolvedEntity& resolvedEntity)
{
    auto handleFunc = [asyncResp](const boost::system::error_code ec,
                                  std::vector<uint8_t>& responseBytes) {
        if (ec)
        {
            BMCWEB_LOG_ERROR << "RootOfTrust.Actions.SendCommand failed: "
                             << ec.message();
            redfish::messages::internalError(asyncResp->res);
            return;
        }

        asyncResp->res.jsonValue["CommandResponse"] =
            bytesToHexString(responseBytes);
    };
    std::vector<uint8_t> bytes = hexStringToBytes(command);
    if (bytes.empty())
    {
        BMCWEB_LOG_DEBUG << "Invalid command: " << command;
        redfish::messages::actionParameterValueTypeError(command, "Command",
                                                         "SendCommand");
        return;
    }

    crow::connections::systemBus->async_method_call(
        handleFunc, resolvedEntity.service, resolvedEntity.object,
        resolvedEntity.interface, "SendHostCommand", bytes);
}

inline void sendRoTCommand(const crow::Request& request,
                           const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                           const std::string& rotId)
{
    std::string command;
    if (!redfish::json_util::readJsonAction(request, asyncResp->res, "Command",
                                            command))
    {
        BMCWEB_LOG_DEBUG << "Missing property Command.";
        redfish::messages::actionParameterMissing(asyncResp->res, "SendCommand",
                                                  "Command");
        return;
    }

    resolveRoT(command, asyncResp, rotId, invokeRoTCommand);
}

inline void requestRoutes(App& app)
{
    BMCWEB_ROUTE(app, "/google/v1/")
        .methods(boost::beast::http::verb::get)(getGoogleV1);

    BMCWEB_ROUTE(app, "/google/v1/RootOfTrustCollection")
        .privileges({{"ConfigureManager"}})
        .methods(boost::beast::http::verb::get)(getRootOfTrustCollection);

    BMCWEB_ROUTE(app, "/google/v1/RootOfTrustCollection/<str>")
        .privileges({{"ConfigureManager"}})
        .methods(boost::beast::http::verb::get)(getRootOfTrust);

    BMCWEB_ROUTE(
        app,
        "/google/v1/RootOfTrustCollection/<str>/Actions/RootOfTrust.SendCommand")
        .privileges({{"ConfigureManager"}})
        .methods(boost::beast::http::verb::post)(sendRoTCommand);
}

} // namespace google_api
} // namespace crow
