#pragma once

#include "app.hpp"
#include "dbus_utility.hpp"
#include "error_messages.hpp"

#include <nlohmann/json.hpp>
#include <sdbusplus/asio/property.hpp>
#include <sdbusplus/message/types.hpp>

#include <string>
#include <string_view>

namespace redfish
{
namespace service_util
{
namespace details
{

bool matchService(const sdbusplus::message::object_path& objPath,
                  const std::string_view serviceName)
{
    // For service named as <unitName>@<instanceName>, only compare the unitName
    // part. Object path is automatically decoded as it is encoded by sdbusplus.
    std::string fullUnitName = objPath.filename();
    size_t pos = fullUnitName.rfind('@');
    return std::string_view(fullUnitName).substr(0, pos) == serviceName;
}

enum class FindError
{
    NotFound,
    DBusError,
};

template <typename SuccessCallback, typename ErrorCallback>
inline void findMatchedServices(const std::string& serviceName,
                                SuccessCallback&& onSuccess,
                                ErrorCallback&& onError)
{
    crow::connections::systemBus->async_method_call(
        [serviceName, onSuccess,
         onError](const boost::system::error_code ec,
                  const dbus::utility::ManagedObjectType& objects) {
        if (ec)
        {
            onError(FindError::DBusError);
            return;
        }

        bool serviceFound = false;
        for (const auto& object : objects)
        {
            if (!matchService(object.first, serviceName))
            {
                continue;
            }

            serviceFound = true;
            // The return value indicates whether to break the loop or not,
            // used for get property
            if (onSuccess(object))
            {
                return;
            }
        }

        if (!serviceFound)
        {
            onError(FindError::NotFound);
        }
        },
        "xyz.openbmc_project.Control.Service.Manager",
        "/xyz/openbmc_project/control/service",
        "org.freedesktop.DBus.ObjectManager", "GetManagedObjects");
}

template <typename T>
inline T
    getPropertyFromInterface(const dbus::utility::DBusInteracesMap& interfaces,
                             const std::string& interfaceName,
                             const std::string& propertyName)
{
    for (const auto& [interface, properties] : interfaces)
    {
        if (interface != interfaceName)
        {
            continue;
        }

        for (const auto& [key, val] : properties)
        {
            if (key != propertyName)
            {
                continue;
            }

            const auto* value = std::get_if<T>(&val);
            if (value != nullptr)
            {
                return *value;
            }
            return T{};
        }
    }
    return T{};
}

} // namespace details

void getEnabled(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                const std::string& serviceName,
                const nlohmann::json::json_pointer& valueJsonPtr)
{
    details::findMatchedServices(
        serviceName,
        [asyncResp, valueJsonPtr](
            const dbus::utility::ManagedObjectType::value_type& object) -> int {
            bool enabled = details::getPropertyFromInterface<bool>(
                object.second, "xyz.openbmc_project.Control.Service.Attributes",
                "Running");
            asyncResp->res.jsonValue[valueJsonPtr] = enabled;

            // If one of the service instance is running, show it as Enabled
            // in redfish.
            if (enabled)
            {
                return 1;
            }
            return 0;
        },
        [asyncResp](details::FindError error) {
        if (error == details::FindError::DBusError)
        {
            messages::internalError(asyncResp->res);
        }
        });
}

void getPortNumber(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                   const std::string& serviceName,
                   const nlohmann::json::json_pointer& valueJsonPtr)
{
    details::findMatchedServices(
        serviceName,
        [asyncResp, valueJsonPtr](
            const dbus::utility::ManagedObjectType::value_type& object) -> int {
            uint16_t port = details::getPropertyFromInterface<uint16_t>(
                object.second,
                "xyz.openbmc_project.Control.Service.SocketAttributes", "Port");
            asyncResp->res.jsonValue[valueJsonPtr] = port;

            // For service with multiple instances, return the port of first
            // valid instance found as redfish only support one port value, they
            // should be same
            if (port != 0)
            {
                return 1;
            }
            return 0;
        },
        [asyncResp](details::FindError error) {
        if (error == details::FindError::DBusError)
        {
            messages::internalError(asyncResp->res);
        }
        });
}

void setEnabled(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                const std::string& propertyName, const std::string& serviceName,
                const bool enabled)
{
    details::findMatchedServices(
        serviceName,
        [asyncResp, enabled](
            const dbus::utility::ManagedObjectType::value_type& object) -> int {
            auto errorCallback =
                [asyncResp](const boost::system::error_code ec) {
            if (ec)
            {
                messages::internalError(asyncResp->res);
                return;
            }
            };
            sdbusplus::asio::setProperty(
                *crow::connections::systemBus,
                "xyz.openbmc_project.Control.Service.Manager", object.first,
                "xyz.openbmc_project.Control.Service.Attributes", "Running",
                enabled, errorCallback);
            sdbusplus::asio::setProperty(
                *crow::connections::systemBus,
                "xyz.openbmc_project.Control.Service.Manager", object.first,
                "xyz.openbmc_project.Control.Service.Attributes", "Enabled",
                enabled, errorCallback);
            return 0;
        },
        [asyncResp, propertyName](details::FindError error) {
        if (error == details::FindError::DBusError)
        {
            messages::internalError(asyncResp->res);
            return;
        }
        else if (error == details::FindError::NotFound)
        {
            // The Redfish property will not be populated in if service is not
            // found, return PropertyUnknown for PATCH request
            messages::propertyUnknown(asyncResp->res, propertyName);
            return;
        }
        });
}

void setPortNumber(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                   const std::string& propertyName,
                   const std::string& serviceName, const uint16_t portNumber)
{
    details::findMatchedServices(
        serviceName,
        [asyncResp, portNumber](
            const dbus::utility::ManagedObjectType::value_type& object) -> int {
            sdbusplus::asio::setProperty(
                *crow::connections::systemBus,
                "xyz.openbmc_project.Control.Service.Manager", object.first,
                "xyz.openbmc_project.Control.Service.SocketAttributes", "Port",
                portNumber, [asyncResp](const boost::system::error_code ec2) {
                    if (ec2)
                    {
                        messages::internalError(asyncResp->res);
                        return;
                    }
                });
            return 0;
        },
        [asyncResp, propertyName](details::FindError error) {
        if (error == details::FindError::DBusError)
        {
            messages::internalError(asyncResp->res);
            return;
        }
        else if (error == details::FindError::NotFound)
        {
            // The Redfish property will not be populated in if service is not
            // found, return PropertyUnknown for PATCH request
            messages::propertyUnknown(asyncResp->res, propertyName);
            return;
        }
        });
}

} // namespace service_util
} // namespace redfish
