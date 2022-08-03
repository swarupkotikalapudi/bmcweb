#include "registries.hpp"

#include "registries/base_message_registry.hpp"
#include "registries/openbmc_message_registry.hpp"
#include "str_utility.hpp"

#include <ranges>
#include <string>
#include <vector>

namespace redfish::registries
{

const Message* getMessageFromRegistry(const std::string& messageKey,
                                      std::span<const Message> registry)
{
    std::span<const Message>::iterator messageIt = std::ranges::find_if(
        registry, [&messageKey](const Message& messageEntry) {
            return messageEntry.messageId == messageKey;
        });
    if (messageIt != registry.end())
    {
        return &(*messageIt);
    }

    return nullptr;
}

const Message* getMessage(std::string_view messageID)
{
    // Redfish MessageIds are in the form
    // RegistryName.MajorVersion.MinorVersion.MessageKey, so parse it to find
    // the right Message
    std::vector<std::string> fields;
    fields.reserve(4);
    bmcweb::split(fields, messageID, '.');
    const std::string& registryName = fields[0];
    const std::string& messageKey = fields[3];

    // Find the right registry and check it for the MessageKey
    if (std::string(base::header.registryPrefix) == registryName)
    {
        return getMessageFromRegistry(messageKey,
                                      std::span<const Message>(base::registry));
    }
    if (std::string(openbmc::header.registryPrefix) == registryName)
    {
        return getMessageFromRegistry(
            messageKey, std::span<const Message>(openbmc::registry));
    }
    return nullptr;
}

} // namespace redfish::registries
