#pragma once

#include "app.hpp"
#include "query.hpp"
#include "registries/privilege_registry.hpp"
#include "utils/chassis_utils.hpp"

#include <memory>
#include <optional>
#include <string>

namespace redfish
{

inline void getChassisAssoction(const std::string& service,
                                const std::string& objPath,
                                const std::string& chassisPath,
                                const std::function<void()>&& callback)
{
    sdbusplus::asio::getProperty<dbus::utility::MapperGetAssociationResponse>(
        *crow::connections::systemBus, service, objPath,
        "xyz.openbmc_project.Association.Definitions", "Associations",
        [callback, chassisPath](
            const boost::system::error_code ec,
            const dbus::utility::MapperGetAssociationResponse& associations) {
        if (ec)
        {
            return;
        }

        for (const auto& assoc : associations)
        {
            const auto& [rType, tType, endpoint] = assoc;
            if (rType == "chassis" && endpoint == chassisPath)
            {
                callback();
                return;
            }
        }
        });
}

inline void updatePowerSupplyList(
    const std::shared_ptr<bmcweb::AsyncResp>& /* asyncResp */,
    const std::string& /* chassisId */, const std::string& powerSupplyPath)
{
    std::string powerSupplyName =
        sdbusplus::message::object_path(powerSupplyPath).filename();
    if (powerSupplyName.empty())
    {
        return;
    }

    // TODO In order for the validator to pass, the Members property will be
    // implemented on the next commit
}

inline void powerSupplyCollection(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& chassisId, const std::string& validChassisPath,
    const dbus::utility::MapperGetSubTreeResponse& subtree)
{
    asyncResp->res.jsonValue["@odata.type"] =
        "#PowerSupplyCollection.PowerSupplyCollection";
    asyncResp->res.jsonValue["Name"] = "Power Supply Collection";
    asyncResp->res.jsonValue["@odata.id"] =
        crow::utility::urlFromPieces("redfish", "v1", "Chassis", chassisId,
                                     "PowerSubsystem", "PowerSupplies");
    asyncResp->res.jsonValue["Description"] =
        "The collection of PowerSupply resource instances.";
    asyncResp->res.jsonValue["Members"] = nlohmann::json::array();
    asyncResp->res.jsonValue["Members@odata.count"] = 0;

    for (const auto& [powerSupplyPath, serviceMap] : subtree)
    {
        for (const auto& [service, interfaces] : serviceMap)
        {
            auto respHandler =
                [asyncResp, powerSupplyPath{powerSupplyPath}, chassisId]() {
                updatePowerSupplyList(asyncResp, chassisId, powerSupplyPath);
            };
            getChassisAssoction(service, powerSupplyPath, validChassisPath,
                                std::move(respHandler));
        }
    }
}

inline void
    doPowerSupplyCollection(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                            const std::string& chassisId,
                            const std::optional<std::string>& validChassisPath)
{
    if (!validChassisPath)
    {
        BMCWEB_LOG_ERROR << "Not a valid chassis ID" << chassisId;
        messages::resourceNotFound(asyncResp->res, "Chassis", chassisId);
        return;
    }

    crow::connections::systemBus->async_method_call(
        [asyncResp, chassisId, validChassisPath{*validChassisPath}](
            const boost::system::error_code ec,
            const dbus::utility::MapperGetSubTreeResponse& subtree) {
        if (ec)
        {
            BMCWEB_LOG_ERROR << "D-Bus response error on GetSubTree " << ec;
            if (ec.value() ==
                boost::system::linux_error::bad_request_descriptor)
            {
                messages::resourceNotFound(asyncResp->res, "Chassis",
                                           chassisId);
                return;
            }
            messages::internalError(asyncResp->res);
            return;
        }

        powerSupplyCollection(asyncResp, chassisId, validChassisPath, subtree);
        },
        "xyz.openbmc_project.ObjectMapper",
        "/xyz/openbmc_project/object_mapper",
        "xyz.openbmc_project.ObjectMapper", "GetSubTree",
        "/xyz/openbmc_project", 0,
        std::array<const char*, 1>{
            "xyz.openbmc_project.Inventory.Item.PowerSupply"});
}

inline void handlePowerSupplyCollectionHead(
    App& app, const crow::Request& req,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& /* chassisId */)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }
    asyncResp->res.addHeader(
        boost::beast::http::field::link,
        "</redfish/v1/JsonSchemas/PowerSupplyCollection/PowerSupplyCollection.json>; rel=describedby");
}

inline void handlePowerSupplyCollectionGet(
    App& app, const crow::Request& req,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& chassisId)
{
    handlePowerSupplyCollectionHead(app, req, asyncResp, chassisId);

    redfish::chassis_utils::getValidChassisPath(
        asyncResp, chassisId,
        std::bind_front(doPowerSupplyCollection, asyncResp, chassisId));
}

inline void requestRoutesPowerSupplyCollection(App& app)
{
    BMCWEB_ROUTE(app, "/redfish/v1/Chassis/<str>/PowerSubsystem/PowerSupplies/")
        .privileges(redfish::privileges::headPowerSupplyCollection)
        .methods(boost::beast::http::verb::head)(
            std::bind_front(handlePowerSupplyCollectionHead, std::ref(app)));

    BMCWEB_ROUTE(app, "/redfish/v1/Chassis/<str>/PowerSubsystem/PowerSupplies/")
        .privileges(redfish::privileges::getPowerSupplyCollection)
        .methods(boost::beast::http::verb::get)(
            std::bind_front(handlePowerSupplyCollectionGet, std::ref(app)));
}

} // namespace redfish
