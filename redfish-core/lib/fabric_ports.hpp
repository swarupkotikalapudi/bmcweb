#pragma once

#include "bmcweb_config.h"

#include "app.hpp"
#include "dbus_utility.hpp"
#include "fabric_adapters.hpp"
#include "human_sort.hpp"
#include "query.hpp"
#include "registries/privilege_registry.hpp"

#include <boost/system/error_code.hpp>
#include <boost/url/format.hpp>

#include <array>
#include <functional>
#include <memory>
#include <ranges>
#include <string>
#include <string_view>

namespace redfish
{

inline void getFabricPortProperties(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& systemName, const std::string& adapterId,
    const std::string& portId, const std::string& /*portPath*/,
    const std::string& /*serviceName*/
)
{
    asyncResp->res.addHeader(
        boost::beast::http::field::link,
        "</redfish/v1/JsonSchemas/Port/Port.json>; rel=describedby");

    asyncResp->res.jsonValue["@odata.type"] = "#Port.v1_7_0.Port";
    asyncResp->res.jsonValue["@odata.id"] =
        boost::urls::format("/redfish/v1/Systems/{}/FabricAdapters/{}/Ports/{}",
                            systemName, adapterId, portId);
    asyncResp->res.jsonValue["Id"] = portId;
    asyncResp->res.jsonValue["Name"] = "Fabric Port";
}

inline void getFabricAssociatedPortSubTree(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::function<void(const boost::system::error_code&,
                             const dbus::utility::MapperGetSubTreeResponse&)>&&
        callback,
    const std::string& fabricAdapterPath,
    const std::string& /*fabricServiceName*/
)
{
    static constexpr std::array<std::string_view, 1> portInterfaces{
        "xyz.openbmc_project.Inventory.Connector.Port"};

    dbus::utility::getAssociatedSubTree(
        fabricAdapterPath + "/connecting",
        sdbusplus::message::object_path("/xyz/openbmc_project/inventory"), 0,
        portInterfaces,
        [asyncResp,
         callback](const boost::system::error_code& ec,
                   const dbus::utility::MapperGetSubTreeResponse& subtree) {
        callback(ec, subtree);
    });
}

template <typename Callback>
inline void
    getFabricPortSubTree(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                         const std::string& systemName,
                         const std::string& adapterId, Callback&& callback)
{
    getValidFabricAdapterPath(
        adapterId, systemName, asyncResp,
        std::bind_front(getFabricAssociatedPortSubTree, asyncResp,
                        std::forward<Callback>(callback)));
}

inline void doGetValidFabricPortSubTree(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::function<void(const dbus::utility::MapperGetSubTreeResponse&)>&&
        callback,
    const boost::system::error_code& ec,
    const dbus::utility::MapperGetSubTreeResponse& subtree)
{
    if (ec)
    {
        if (ec.value() != EBADR)
        {
            BMCWEB_LOG_ERROR("DBUS response error {}", ec.value());
            messages::internalError(asyncResp->res);
            return;
        }
        // caller will handle the case as empty Ports
        BMCWEB_LOG_DEBUG("Port association not found");
    }
    callback(subtree);
}

template <typename Callback>
inline void getValidFabricPortSubTree(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& systemName, const std::string& adapterId,
    Callback&& callback)
{
    getFabricPortSubTree(asyncResp, systemName, adapterId,
                         std::bind_front(doGetValidFabricPortSubTree, asyncResp,
                                         std::forward<Callback>(callback)));
}

inline void afterGetValidFabricPortPath(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& portId,
    const std::function<void(const std::string& portPath,
                             const std::string& portServiceName)>&& callback,
    const dbus::utility::MapperGetSubTreeResponse& portSubTree)
{
    auto portIter = std::ranges::find_if(
        portSubTree,
        [&portId](const std::pair<std::string, dbus::utility::MapperServiceMap>&
                      object) {
        return sdbusplus::message::object_path(object.first).filename() ==
               portId;
    });
    if (portIter == portSubTree.end())
    {
        BMCWEB_LOG_WARNING("Port {} not found", portId);
        messages::resourceNotFound(asyncResp->res, "Port", portId);
        return;
    }
    if (portIter->first.empty() || portIter->second.empty())
    {
        BMCWEB_LOG_ERROR("Fabric Port: mapper error!");
        messages::internalError(asyncResp->res);
        return;
    }

    const std::string& portPath = portIter->first;
    const std::string& portServiceName = portIter->second.begin()->first;
    if (portServiceName.empty())
    {
        BMCWEB_LOG_ERROR("Fabric Port: mapper error!");
        messages::internalError(asyncResp->res);
        return;
    }

    callback(portPath, portServiceName);
}

template <typename Callback>
inline void
    getValidFabricPortPath(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                           const std::string& systemName,
                           const std::string& adapterId,
                           const std::string& portId, Callback&& callback)
{
    getValidFabricPortSubTree(
        asyncResp, systemName, adapterId,
        std::bind_front(afterGetValidFabricPortPath, asyncResp, portId,
                        std::forward<Callback>(callback)));
}

inline void
    handleFabricPortHead(crow::App& app, const crow::Request& req,
                         const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                         const std::string& systemName,
                         const std::string& adapterId,
                         const std::string& portId)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }
    if constexpr (bmcwebEnableMultiHost)
    {
        // Option currently returns no systems.  TBD
        messages::resourceNotFound(asyncResp->res, "ComputerSystem",
                                   systemName);
        return;
    }

    getValidFabricPortPath(asyncResp, systemName, adapterId, portId,
                           [asyncResp](const std::string& /*portPath*/,
                                       const std::string& /*serviceName*/) {
        asyncResp->res.addHeader(
            boost::beast::http::field::link,
            "</redfish/v1/JsonSchemas/Port/Port.json>; rel=describedby");
    });
}

inline void
    handleFabricPortGet(App& app, const crow::Request& req,
                        const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                        const std::string& systemName,
                        const std::string& adapterId, const std::string& portId)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }
    if constexpr (bmcwebEnableMultiHost)
    {
        // Option currently returns no systems.  TBD
        messages::resourceNotFound(asyncResp->res, "ComputerSystem",
                                   systemName);
        return;
    }

    getValidFabricPortPath(asyncResp, systemName, adapterId, portId,
                           std::bind_front(getFabricPortProperties, asyncResp,
                                           systemName, adapterId, portId));
}

inline void doHandleFabricPortCollectionGet(
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& systemName, const std::string& adapterId,
    const dbus::utility::MapperGetSubTreeResponse& portSubTree)
{
    asyncResp->res.addHeader(
        boost::beast::http::field::link,
        "</redfish/v1/JsonSchemas/PortCollection/PortCollection.json>; rel=describedby");

    asyncResp->res.jsonValue["@odata.type"] = "#PortCollection.PortCollection";
    asyncResp->res.jsonValue["Name"] = "Fabric Port Collection";
    asyncResp->res.jsonValue["@odata.id"] =
        boost::urls::format("/redfish/v1/Systems/{}/FabricAdapters/{}/Ports",
                            systemName, adapterId);

    std::vector<std::string> portIdNames;
    for (const auto& [portPath, serviceMap] : portSubTree)
    {
        std::string portId =
            sdbusplus::message::object_path(portPath).filename();
        if (portId.empty())
        {
            BMCWEB_LOG_ERROR("Invalid portPath");
            messages::internalError(asyncResp->res);
            return;
        }
        portIdNames.emplace_back(std::move(portId));
    }
    std::sort(portIdNames.begin(), portIdNames.end(),
              AlphanumLess<std::string>());

    nlohmann::json::array_t members;
    for (const std::string& portId : portIdNames)
    {
        nlohmann::json::object_t item;
        item["@odata.id"] = boost::urls::format(
            "/redfish/v1/Systems/{}/FabricAdapters/{}/Ports/{}", systemName,
            adapterId, portId);
        members.emplace_back(std::move(item));
    }

    asyncResp->res.jsonValue["Members@odata.count"] = members.size();
    asyncResp->res.jsonValue["Members"] = std::move(members);
}

inline void handleFarbicPortCollectionHead(
    crow::App& app, const crow::Request& req,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& systemName, const std::string& adapterId)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }
    if constexpr (bmcwebEnableMultiHost)
    {
        // Option currently returns no systems.  TBD
        messages::resourceNotFound(asyncResp->res, "ComputerSystem",
                                   systemName);
        return;
    }
    if (systemName != "system")
    {
        messages::resourceNotFound(asyncResp->res, "ComputerSystem",
                                   systemName);
        return;
    }

    getValidFabricPortSubTree(
        asyncResp, systemName, adapterId,
        [asyncResp](
            const dbus::utility::MapperGetSubTreeResponse& /*portSubTree*/) {
        asyncResp->res.addHeader(
            boost::beast::http::field::link,
            "</redfish/v1/JsonSchemas/PortCollection/PortCollection.json>; rel=describedby");
    });
}

inline void handleFabricPortCollectionGet(
    App& app, const crow::Request& req,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& systemName, const std::string& adapterId)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }
    if constexpr (bmcwebEnableMultiHost)
    {
        // Option currently returns no systems.  TBD
        messages::resourceNotFound(asyncResp->res, "ComputerSystem",
                                   systemName);
        return;
    }
    if (systemName != "system")
    {
        messages::resourceNotFound(asyncResp->res, "ComputerSystem",
                                   systemName);
        return;
    }

    getValidFabricPortSubTree(asyncResp, systemName, adapterId,
                              std::bind_front(doHandleFabricPortCollectionGet,
                                              asyncResp, systemName,
                                              adapterId));
}

inline void requestRoutesFabricPort(App& app)
{
    BMCWEB_ROUTE(app,
                 "/redfish/v1/Systems/<str>/FabricAdapters/<str>/Ports/<str>/")
        .privileges(redfish::privileges::headPort)
        .methods(boost::beast::http::verb::head)(
            std::bind_front(handleFabricPortHead, std::ref(app)));

    BMCWEB_ROUTE(app,
                 "/redfish/v1/Systems/<str>/FabricAdapters/<str>/Ports/<str>/")
        .privileges(redfish::privileges::getPort)
        .methods(boost::beast::http::verb::get)(
            std::bind_front(handleFabricPortGet, std::ref(app)));

    BMCWEB_ROUTE(app, "/redfish/v1/Systems/<str>/FabricAdapters/<str>/Ports/")
        .privileges(redfish::privileges::headPortCollection)
        .methods(boost::beast::http::verb::head)(
            std::bind_front(handleFarbicPortCollectionHead, std::ref(app)));

    BMCWEB_ROUTE(app, "/redfish/v1/Systems/<str>/FabricAdapters/<str>/Ports/")
        .privileges(redfish::privileges::getPortCollection)
        .methods(boost::beast::http::verb::get)(
            std::bind_front(handleFabricPortCollectionGet, std::ref(app)));
}

} // namespace redfish
