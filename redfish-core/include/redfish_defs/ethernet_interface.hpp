#pragma once
#include <nlohmann/json.hpp>

namespace ethernet_interface
{
// clang-format off

enum class LinkStatus{
    Invalid,
    LinkUp,
    NoLink,
    LinkDown,
};

enum class DHCPv6OperatingMode{
    Invalid,
    Stateful,
    Stateless,
    Disabled,
    Enabled,
};

enum class DHCPFallback{
    Invalid,
    Static,
    AutoConfig,
    None,
};

enum class EthernetDeviceType{
    Invalid,
    Physical,
    Virtual,
};

NLOHMANN_JSON_SERIALIZE_ENUM(LinkStatus, { //NOLINT
    {LinkStatus::Invalid, "Invalid"},
    {LinkStatus::LinkUp, "LinkUp"},
    {LinkStatus::NoLink, "NoLink"},
    {LinkStatus::LinkDown, "LinkDown"},
});

NLOHMANN_JSON_SERIALIZE_ENUM(DHCPv6OperatingMode, { //NOLINT
    {DHCPv6OperatingMode::Invalid, "Invalid"},
    {DHCPv6OperatingMode::Stateful, "Stateful"},
    {DHCPv6OperatingMode::Stateless, "Stateless"},
    {DHCPv6OperatingMode::Disabled, "Disabled"},
    {DHCPv6OperatingMode::Enabled, "Enabled"},
});

NLOHMANN_JSON_SERIALIZE_ENUM(DHCPFallback, { //NOLINT
    {DHCPFallback::Invalid, "Invalid"},
    {DHCPFallback::Static, "Static"},
    {DHCPFallback::AutoConfig, "AutoConfig"},
    {DHCPFallback::None, "None"},
});

NLOHMANN_JSON_SERIALIZE_ENUM(EthernetDeviceType, { //NOLINT
    {EthernetDeviceType::Invalid, "Invalid"},
    {EthernetDeviceType::Physical, "Physical"},
    {EthernetDeviceType::Virtual, "Virtual"},
});

}
// clang-format on
