#pragma once

#include <app_class_decl.hpp>
#include <ethernet.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>
#include <dbus_singleton.hpp>
#include <error_messages.hpp>
#include <registries/privilege_registry.hpp>
#include <utils/json_utils.hpp>

#include <optional>
#include <utility>
#include <variant>

using crow::App;

// TODO(ed) requestRoutesHypervisorSystems seems to have copy-pasted a
// lot of code, and has a number of methods that have name conflicts with the
// normal ethernet internfaces in ethernet.hpp.  For the moment, we'll put
// hypervisor in a namespace to isolate it, but these methods eventually need
// deduplicated
namespace redfish::hypervisor
{

/**
 * @brief Retrieves hypervisor state properties over dbus
 *
 * The hypervisor state object is optional so this function will only set the
 * state variables if the object is found
 *
 * @param[in] aResp     Shared pointer for completing asynchronous calls.
 *
 * @return None.
 */
inline void getHypervisorState(const std::shared_ptr<bmcweb::AsyncResp>& aResp)
{
    BMCWEB_LOG_DEBUG << "Get hypervisor state information.";
    crow::connections::systemBus->async_method_call(
        [aResp](const boost::system::error_code ec,
                const std::variant<std::string>& hostState) {
            if (ec)
            {
                BMCWEB_LOG_DEBUG << "DBUS response error " << ec;
                // This is an optional D-Bus object so just return if
                // error occurs
                return;
            }

            const std::string* s = std::get_if<std::string>(&hostState);
            if (s == nullptr)
            {
                messages::internalError(aResp->res);
                return;
            }

            BMCWEB_LOG_DEBUG << "Hypervisor state: " << *s;
            // Verify Host State
            if (*s == "xyz.openbmc_project.State.Host.HostState.Running")
            {
                aResp->res.jsonValue["PowerState"] = "On";
                aResp->res.jsonValue["Status"]["State"] = "Enabled";
            }
            else if (*s == "xyz.openbmc_project.State.Host.HostState."
                           "Quiesced")
            {
                aResp->res.jsonValue["PowerState"] = "On";
                aResp->res.jsonValue["Status"]["State"] = "Quiesced";
            }
            else if (*s == "xyz.openbmc_project.State.Host.HostState."
                           "Standby")
            {
                aResp->res.jsonValue["PowerState"] = "On";
                aResp->res.jsonValue["Status"]["State"] = "StandbyOffline";
            }
            else if (*s == "xyz.openbmc_project.State.Host.HostState."
                           "TransitioningToRunning")
            {
                aResp->res.jsonValue["PowerState"] = "PoweringOn";
                aResp->res.jsonValue["Status"]["State"] = "Starting";
            }
            else if (*s == "xyz.openbmc_project.State.Host.HostState."
                           "TransitioningToOff")
            {
                aResp->res.jsonValue["PowerState"] = "PoweringOff";
                aResp->res.jsonValue["Status"]["State"] = "Enabled";
            }
            else if (*s == "xyz.openbmc_project.State.Host.HostState.Off")
            {
                aResp->res.jsonValue["PowerState"] = "Off";
                aResp->res.jsonValue["Status"]["State"] = "Disabled";
            }
            else
            {
                messages::internalError(aResp->res);
                return;
            }
        },
        "xyz.openbmc_project.State.Hypervisor",
        "/xyz/openbmc_project/state/hypervisor0",
        "org.freedesktop.DBus.Properties", "Get",
        "xyz.openbmc_project.State.Host", "CurrentHostState");
}

/**
 * @brief Populate Actions if any are valid for hypervisor object
 *
 * The hypervisor state object is optional so this function will only set the
 * Action if the object is found
 *
 * @param[in] aResp     Shared pointer for completing asynchronous calls.
 *
 * @return None.
 */
inline void
    getHypervisorActions(const std::shared_ptr<bmcweb::AsyncResp>& aResp)
{
    BMCWEB_LOG_DEBUG << "Get hypervisor actions.";
    crow::connections::systemBus->async_method_call(
        [aResp](
            const boost::system::error_code ec,
            const std::vector<std::pair<std::string, std::vector<std::string>>>&
                objInfo) {
            if (ec)
            {
                BMCWEB_LOG_DEBUG << "DBUS response error " << ec;
                // This is an optional D-Bus object so just return if
                // error occurs
                return;
            }

            if (objInfo.size() == 0)
            {
                // As noted above, this is an optional interface so just return
                // if there is no instance found
                return;
            }

            if (objInfo.size() > 1)
            {
                // More then one hypervisor object is not supported and is an
                // error
                messages::internalError(aResp->res);
                return;
            }

            // Object present so system support limited ComputerSystem Action
            aResp->res.jsonValue["Actions"]["#ComputerSystem.Reset"] = {
                {"target",
                 "/redfish/v1/Systems/hypervisor/Actions/ComputerSystem.Reset"},
                {"@Redfish.ActionInfo",
                 "/redfish/v1/Systems/hypervisor/ResetActionInfo"}};
        },
        "xyz.openbmc_project.ObjectMapper",
        "/xyz/openbmc_project/object_mapper",
        "xyz.openbmc_project.ObjectMapper", "GetObject",
        "/xyz/openbmc_project/state/hypervisor0",
        std::array<const char*, 1>{"xyz.openbmc_project.State.Host"});
}

inline bool extractHypervisorInterfaceData(
    const std::string& ethIfaceId, const GetManagedObjects& dbusData,
    EthernetInterfaceData& ethData,
    boost::container::flat_set<IPv4AddressData>& ipv4Config)
{
    bool idFound = false;
    for (const auto& objpath : dbusData)
    {
        for (const auto& ifacePair : objpath.second)
        {
            if (objpath.first ==
                "/xyz/openbmc_project/network/hypervisor/" + ethIfaceId)
            {
                idFound = true;
                if (ifacePair.first == "xyz.openbmc_project.Network.MACAddress")
                {
                    for (const auto& propertyPair : ifacePair.second)
                    {
                        if (propertyPair.first == "MACAddress")
                        {
                            const std::string* mac =
                                std::get_if<std::string>(&propertyPair.second);
                            if (mac != nullptr)
                            {
                                ethData.mac_address = *mac;
                            }
                        }
                    }
                }
                else if (ifacePair.first ==
                         "xyz.openbmc_project.Network.EthernetInterface")
                {
                    for (const auto& propertyPair : ifacePair.second)
                    {
                        if (propertyPair.first == "DHCPEnabled")
                        {
                            const std::string* dhcp =
                                std::get_if<std::string>(&propertyPair.second);
                            if (dhcp != nullptr)
                            {
                                ethData.DHCPEnabled = *dhcp;
                                break; // Interested on only "DHCPEnabled".
                                       // Stop parsing since we got the
                                       // "DHCPEnabled" value.
                            }
                        }
                    }
                }
            }
            if (objpath.first == "/xyz/openbmc_project/network/hypervisor/" +
                                     ethIfaceId + "/ipv4/addr0")
            {
                std::pair<boost::container::flat_set<IPv4AddressData>::iterator,
                          bool>
                    it = ipv4Config.insert(IPv4AddressData{});
                IPv4AddressData& ipv4Address = *it.first;
                if (ifacePair.first == "xyz.openbmc_project.Object.Enable")
                {
                    for (auto& property : ifacePair.second)
                    {
                        if (property.first == "Enabled")
                        {
                            const bool* intfEnable =
                                std::get_if<bool>(&property.second);
                            if (intfEnable != nullptr)
                            {
                                ipv4Address.isActive = *intfEnable;
                                break;
                            }
                        }
                    }
                }
                if (ifacePair.first == "xyz.openbmc_project.Network.IP")
                {
                    for (auto& property : ifacePair.second)
                    {
                        if (property.first == "Address")
                        {
                            const std::string* address =
                                std::get_if<std::string>(&property.second);
                            if (address != nullptr)
                            {
                                ipv4Address.address = *address;
                            }
                        }
                        else if (property.first == "Origin")
                        {
                            const std::string* origin =
                                std::get_if<std::string>(&property.second);
                            if (origin != nullptr)
                            {
                                ipv4Address.origin =
                                    translateAddressOriginDbusToRedfish(*origin,
                                                                        true);
                            }
                        }
                        else if (property.first == "PrefixLength")
                        {
                            const uint8_t* mask =
                                std::get_if<uint8_t>(&property.second);
                            if (mask != nullptr)
                            {
                                // convert it to the string
                                ipv4Address.netmask = getNetmask(*mask);
                            }
                        }
                        else
                        {
                            BMCWEB_LOG_ERROR
                                << "Got extra property: " << property.first
                                << " on the " << objpath.first.str << " object";
                        }
                    }
                }
            }
            if (objpath.first == "/xyz/openbmc_project/network/hypervisor")
            {
                // System configuration shows up in the global namespace, so no
                // need to check eth number
                if (ifacePair.first ==
                    "xyz.openbmc_project.Network.SystemConfiguration")
                {
                    for (const auto& propertyPair : ifacePair.second)
                    {
                        if (propertyPair.first == "HostName")
                        {
                            const std::string* hostName =
                                std::get_if<std::string>(&propertyPair.second);
                            if (hostName != nullptr)
                            {
                                ethData.hostname = *hostName;
                            }
                        }
                        else if (propertyPair.first == "DefaultGateway")
                        {
                            const std::string* defaultGateway =
                                std::get_if<std::string>(&propertyPair.second);
                            if (defaultGateway != nullptr)
                            {
                                ethData.default_gateway = *defaultGateway;
                            }
                        }
                    }
                }
            }
        }
    }
    return idFound;
}
/**
 * Function that retrieves all properties for given Hypervisor Ethernet
 * Interface Object from Settings Manager
 * @param ethIfaceId Hypervisor ethernet interface id to query on DBus
 * @param callback a function that shall be called to convert Dbus output
 * into JSON
 */
template <typename CallbackFunc>
void getHypervisorIfaceData(const std::string& ethIfaceId,
                            CallbackFunc&& callback)
{
    crow::connections::systemBus->async_method_call(
        [ethIfaceId{std::string{ethIfaceId}},
         callback{std::move(callback)}](const boost::system::error_code error,
                                        const GetManagedObjects& resp) {
            EthernetInterfaceData ethData{};
            boost::container::flat_set<IPv4AddressData> ipv4Data;
            if (error)
            {
                callback(false, ethData, ipv4Data);
                return;
            }

            bool found = extractHypervisorInterfaceData(ethIfaceId, resp,
                                                        ethData, ipv4Data);
            if (!found)
            {
                BMCWEB_LOG_INFO << "Hypervisor Interface not found";
            }
            callback(found, ethData, ipv4Data);
        },
        "xyz.openbmc_project.Settings", "/",
        "org.freedesktop.DBus.ObjectManager", "GetManagedObjects");
}

/**
 * @brief Sets the Hypervisor Interface IPAddress DBUS
 *
 * @param[in] aResp          Shared pointer for generating response message.
 * @param[in] ipv4Address    Address from the incoming request
 * @param[in] ethIfaceId     Hypervisor Interface Id
 *
 * @return None.
 */
inline void
    setHypervisorIPv4Address(const std::shared_ptr<bmcweb::AsyncResp>& aResp,
                             const std::string& ethIfaceId,
                             const std::string& ipv4Address)
{
    BMCWEB_LOG_DEBUG << "Setting the Hypervisor IPaddress : " << ipv4Address
                     << " on Iface: " << ethIfaceId;
    crow::connections::systemBus->async_method_call(
        [aResp](const boost::system::error_code ec) {
            if (ec)
            {
                BMCWEB_LOG_ERROR << "DBUS response error " << ec;
                return;
            }
            BMCWEB_LOG_DEBUG << "Hypervisor IPaddress is Set";
        },
        "xyz.openbmc_project.Settings",
        "/xyz/openbmc_project/network/hypervisor/" + ethIfaceId + "/ipv4/addr0",
        "org.freedesktop.DBus.Properties", "Set",
        "xyz.openbmc_project.Network.IP", "Address",
        std::variant<std::string>(ipv4Address));
}

/**
 * @brief Sets the Hypervisor Interface SubnetMask DBUS
 *
 * @param[in] aResp     Shared pointer for generating response message.
 * @param[in] subnet    SubnetMask from the incoming request
 * @param[in] ethIfaceId Hypervisor Interface Id
 *
 * @return None.
 */
inline void
    setHypervisorIPv4Subnet(const std::shared_ptr<bmcweb::AsyncResp>& aResp,
                            const std::string& ethIfaceId, const uint8_t subnet)
{
    BMCWEB_LOG_DEBUG << "Setting the Hypervisor subnet : " << subnet
                     << " on Iface: " << ethIfaceId;

    crow::connections::systemBus->async_method_call(
        [aResp](const boost::system::error_code ec) {
            if (ec)
            {
                BMCWEB_LOG_ERROR << "DBUS response error " << ec;
                return;
            }
            BMCWEB_LOG_DEBUG << "SubnetMask is Set";
        },
        "xyz.openbmc_project.Settings",
        "/xyz/openbmc_project/network/hypervisor/" + ethIfaceId + "/ipv4/addr0",
        "org.freedesktop.DBus.Properties", "Set",
        "xyz.openbmc_project.Network.IP", "PrefixLength",
        std::variant<uint8_t>(subnet));
}

/**
 * @brief Sets the Hypervisor Interface Gateway DBUS
 *
 * @param[in] aResp          Shared pointer for generating response message.
 * @param[in] gateway        Gateway from the incoming request
 * @param[in] ethIfaceId     Hypervisor Interface Id
 *
 * @return None.
 */
inline void
    setHypervisorIPv4Gateway(const std::shared_ptr<bmcweb::AsyncResp>& aResp,
                             const std::string& gateway)
{
    BMCWEB_LOG_DEBUG
        << "Setting the DefaultGateway to the last configured gateway";

    crow::connections::systemBus->async_method_call(
        [aResp](const boost::system::error_code ec) {
            if (ec)
            {
                BMCWEB_LOG_ERROR << "DBUS response error " << ec;
                return;
            }
            BMCWEB_LOG_DEBUG << "Default Gateway is Set";
        },
        "xyz.openbmc_project.Settings",
        "/xyz/openbmc_project/network/hypervisor",
        "org.freedesktop.DBus.Properties", "Set",
        "xyz.openbmc_project.Network.SystemConfiguration", "DefaultGateway",
        std::variant<std::string>(gateway));
}

/**
 * @brief Creates a static IPv4 entry
 *
 * @param[in] ifaceId      Id of interface upon which to create the IPv4 entry
 * @param[in] prefixLength IPv4 prefix syntax for the subnet mask
 * @param[in] gateway      IPv4 address of this interfaces gateway
 * @param[in] address      IPv4 address to assign to this interface
 * @param[io] asyncResp    Response object that will be returned to client
 *
 * @return None
 */
inline void
    createHypervisorIPv4(const std::string& ifaceId, uint8_t prefixLength,
                         const std::string& gateway, const std::string& address,
                         const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    setHypervisorIPv4Address(asyncResp, ifaceId, address);
    setHypervisorIPv4Gateway(asyncResp, gateway);
    setHypervisorIPv4Subnet(asyncResp, ifaceId, prefixLength);
}

/**
 * @brief Deletes given IPv4 interface
 *
 * @param[in] ifaceId     Id of interface whose IP should be deleted
 * @param[io] asyncResp   Response object that will be returned to client
 *
 * @return None
 */
inline void
    deleteHypervisorIPv4(const std::string& ifaceId,
                         const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    std::string address = "0.0.0.0";
    std::string gateway = "0.0.0.0";
    const uint8_t prefixLength = 0;
    setHypervisorIPv4Address(asyncResp, ifaceId, address);
    setHypervisorIPv4Gateway(asyncResp, gateway);
    setHypervisorIPv4Subnet(asyncResp, ifaceId, prefixLength);
}

inline void parseInterfaceData(
    nlohmann::json& jsonResponse, const std::string& ifaceId,
    const EthernetInterfaceData& ethData,
    const boost::container::flat_set<IPv4AddressData>& ipv4Data)
{
    jsonResponse["Id"] = ifaceId;
    jsonResponse["@odata.id"] =
        "/redfish/v1/Systems/hypervisor/EthernetInterfaces/" + ifaceId;
    jsonResponse["InterfaceEnabled"] = true;
    jsonResponse["MACAddress"] = ethData.mac_address;

    jsonResponse["HostName"] = ethData.hostname;
    jsonResponse["DHCPv4"]["DHCPEnabled"] =
        translateDHCPEnabledToBool(ethData.DHCPEnabled, true);

    nlohmann::json& ipv4Array = jsonResponse["IPv4Addresses"];
    nlohmann::json& ipv4StaticArray = jsonResponse["IPv4StaticAddresses"];
    ipv4Array = nlohmann::json::array();
    ipv4StaticArray = nlohmann::json::array();
    for (auto& ipv4Config : ipv4Data)
    {
        if (ipv4Config.isActive)
        {

            ipv4Array.push_back({{"AddressOrigin", ipv4Config.origin},
                                 {"SubnetMask", ipv4Config.netmask},
                                 {"Address", ipv4Config.address},
                                 {"Gateway", ethData.default_gateway}});
            if (ipv4Config.origin == "Static")
            {
                ipv4StaticArray.push_back(
                    {{"AddressOrigin", ipv4Config.origin},
                     {"SubnetMask", ipv4Config.netmask},
                     {"Address", ipv4Config.address},
                     {"Gateway", ethData.default_gateway}});
            }
        }
    }
}

inline void setDHCPEnabled(const std::string& ifaceId,
                           const bool& ipv4DHCPEnabled,
                           const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    const std::string dhcp = getDhcpEnabledEnumeration(ipv4DHCPEnabled, false);
    crow::connections::systemBus->async_method_call(
        [asyncResp](const boost::system::error_code ec) {
            if (ec)
            {
                BMCWEB_LOG_ERROR << "D-Bus responses error: " << ec;
                messages::internalError(asyncResp->res);
                return;
            }
        },
        "xyz.openbmc_project.Settings",
        "/xyz/openbmc_project/network/hypervisor/" + ifaceId,
        "org.freedesktop.DBus.Properties", "Set",
        "xyz.openbmc_project.Network.EthernetInterface", "DHCPEnabled",
        std::variant<std::string>{dhcp});

    // Set the IPv4 address origin to the DHCP / Static as per the new value
    // of the DHCPEnabled property
    std::string origin;
    if (ipv4DHCPEnabled == false)
    {
        origin = "xyz.openbmc_project.Network.IP.AddressOrigin.Static";
    }
    else
    {
        // DHCPEnabled is set to true. Delete the current IPv4 settings
        // to receive the new values from DHCP server.
        deleteHypervisorIPv4(ifaceId, asyncResp);
        origin = "xyz.openbmc_project.Network.IP.AddressOrigin.DHCP";
    }
    crow::connections::systemBus->async_method_call(
        [asyncResp](const boost::system::error_code ec) {
            if (ec)
            {
                BMCWEB_LOG_ERROR << "DBUS response error " << ec;
                messages::internalError(asyncResp->res);
                return;
            }
            BMCWEB_LOG_DEBUG << "Hypervisor IPaddress Origin is Set";
        },
        "xyz.openbmc_project.Settings",
        "/xyz/openbmc_project/network/hypervisor/" + ifaceId + "/ipv4/addr0",
        "org.freedesktop.DBus.Properties", "Set",
        "xyz.openbmc_project.Network.IP", "Origin",
        std::variant<std::string>(origin));
}

inline void handleHypervisorIPv4StaticPatch(
    const std::string& ifaceId, const nlohmann::json& input,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    if ((!input.is_array()) || input.empty())
    {
        messages::propertyValueTypeError(asyncResp->res, input.dump(),
                                         "IPv4StaticAddresses");
        return;
    }

    // Hypervisor considers the first IP address in the array list
    // as the Hypervisor's virtual management interface supports single IPv4
    // address
    const nlohmann::json& thisJson = input[0];

    // For the error string
    std::string pathString = "IPv4StaticAddresses/1";

    if (!thisJson.is_null() && !thisJson.empty())
    {
        std::optional<std::string> address;
        std::optional<std::string> subnetMask;
        std::optional<std::string> gateway;
        nlohmann::json thisJsonCopy = thisJson;
        if (!json_util::readJson(thisJsonCopy, asyncResp->res, "Address",
                                 address, "SubnetMask", subnetMask, "Gateway",
                                 gateway))
        {
            messages::propertyValueFormatError(
                asyncResp->res,
                thisJson.dump(2, ' ', true,
                              nlohmann::json::error_handler_t::replace),
                pathString);
            return;
        }

        uint8_t prefixLength = 0;
        bool errorInEntry = false;
        if (address)
        {
            if (!ipv4VerifyIpAndGetBitcount(*address))
            {
                messages::propertyValueFormatError(asyncResp->res, *address,
                                                   pathString + "/Address");
                errorInEntry = true;
            }
        }
        else
        {
            messages::propertyMissing(asyncResp->res, pathString + "/Address");
            errorInEntry = true;
        }

        if (subnetMask)
        {
            if (!ipv4VerifyIpAndGetBitcount(*subnetMask, &prefixLength))
            {
                messages::propertyValueFormatError(asyncResp->res, *subnetMask,
                                                   pathString + "/SubnetMask");
                errorInEntry = true;
            }
        }
        else
        {
            messages::propertyMissing(asyncResp->res,
                                      pathString + "/SubnetMask");
            errorInEntry = true;
        }

        if (gateway)
        {
            if (!ipv4VerifyIpAndGetBitcount(*gateway))
            {
                messages::propertyValueFormatError(asyncResp->res, *gateway,
                                                   pathString + "/Gateway");
                errorInEntry = true;
            }
        }
        else
        {
            messages::propertyMissing(asyncResp->res, pathString + "/Gateway");
            errorInEntry = true;
        }

        if (errorInEntry)
        {
            return;
        }

        BMCWEB_LOG_DEBUG << "Calling createHypervisorIPv4 on : " << ifaceId
                         << "," << *address;
        createHypervisorIPv4(ifaceId, prefixLength, *gateway, *address,
                             asyncResp);
        // Set the DHCPEnabled to false since the Static IPv4 is set
        setDHCPEnabled(ifaceId, false, asyncResp);
    }
    else
    {
        if (thisJson.is_null())
        {
            deleteHypervisorIPv4(ifaceId, asyncResp);
        }
    }
}

inline void
    handleHostnamePatch(const std::string& hostName,
                        const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    if (!isHostnameValid(hostName))
    {
        messages::propertyValueFormatError(asyncResp->res, hostName,
                                           "HostName");
        return;
    }

    asyncResp->res.jsonValue["HostName"] = hostName;
    crow::connections::systemBus->async_method_call(
        [asyncResp](const boost::system::error_code ec) {
            if (ec)
            {
                messages::internalError(asyncResp->res);
            }
        },
        "xyz.openbmc_project.Settings",
        "/xyz/openbmc_project/network/hypervisor",
        "org.freedesktop.DBus.Properties", "Set",
        "xyz.openbmc_project.Network.SystemConfiguration", "HostName",
        std::variant<std::string>(hostName));
}

inline void
    setIPv4InterfaceEnabled(const std::string& ifaceId, const bool& isActive,
                            const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    crow::connections::systemBus->async_method_call(
        [asyncResp](const boost::system::error_code ec) {
            if (ec)
            {
                BMCWEB_LOG_ERROR << "D-Bus responses error: " << ec;
                messages::internalError(asyncResp->res);
                return;
            }
        },
        "xyz.openbmc_project.Settings",
        "/xyz/openbmc_project/network/hypervisor/" + ifaceId + "/ipv4/addr0",
        "org.freedesktop.DBus.Properties", "Set",
        "xyz.openbmc_project.Object.Enable", "Enabled",
        std::variant<bool>(isActive));
}

void requestRoutesHypervisorSystems(App& app);

} // namespace redfish::hypervisor
