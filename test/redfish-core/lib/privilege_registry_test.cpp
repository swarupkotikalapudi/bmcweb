#include "async_resp.hpp"
#include "privilege_registry.hpp"

#include <nlohmann/json.hpp>
#include <registries/privilege_registry.hpp>

#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <span>
#include <string>

#include <gtest/gtest.h>

namespace redfish
{
namespace
{

constexpr const char* baseRegistryRelativePath =
    "../static/redfish/v1/registries/Redfish_1.3.0_PrivilegeRegistry.json";

const static std::array<std::string, 3> overrideStrings{
    {"PropertyOverrides", "ResourceURIOverrides", "SubordinateOverrides"}};

nlohmann::json getBasePrivilegeRegistry()
{
    auto baseRegistryPath = std::filesystem::canonical(
        std::filesystem::path(baseRegistryRelativePath));
    std::ifstream ifs(baseRegistryPath);
    return nlohmann::json::parse(ifs);
}

// bool containsOverrides(nlohmann::json mapping)
// {
//     for (const auto& override : overrideStrings)
//     {
//         if (mapping.contains(override))
//         {
//             return true;
//         }
//     }
//     return false;
// }

void assertPrivilegeRegistryGet(crow::Response& res)
{
    nlohmann::json::object_t baseRegistryJson = getBasePrivilegeRegistry();
    nlohmann::json& respJson = res.jsonValue;
    EXPECT_EQ(baseRegistryJson["Name"], respJson["Name"]);
    EXPECT_EQ(baseRegistryJson["PrivilegesUsed"], respJson["PrivilegesUsed"]);

    nlohmann::json::array_t privilegeSetJson;
        nlohmann::json::array_t checkjson;

    const std::span<const Privileges> privilegeSet =
        redfish::privileges::getPrivilegeFromEntityAndMethod(redfish::privileges::EntityTag::tagSession, HttpVerb::Delete);
    
    const std::span<const Privileges> check = redfish::privileges::privilegeSetMap[158][5];

    for (const Privileges& privilege : privilegeSet)
    {
        nlohmann::json::object_t privilegeJson;
        privilegeJson["Privilege"] = privilege.getAllActivePrivilegeNames();
        privilegeSetJson.emplace_back(std::move(privilegeJson));
    }

    for (const Privileges& privilege : check)
    {
        nlohmann::json::object_t privilegeJson;
        privilegeJson["Privilege"] = privilege.getAllActivePrivilegeNames();
        checkjson.emplace_back(std::move(privilegeJson));
    }


    std::cerr << privilegeSetJson << std::endl;
    std::cerr << checkjson << std::endl;
    std::cerr << baseRegistryJson["Mappings"][158]["OperationMap"]["DELETE"] << std::endl;

    // for(size_t i = 0; i < baseRegistryJson["Mappings"].size(); ++i){
    //   if(baseRegistryJson["Mappings"][i]["OperationMap"] !=
    //   respJson["Mappings"][i]["OperationMap"])
    //   {
    //     std::cerr << "ERROR " << std::endl;
    //   }
    //   std::cerr << "base" << baseRegistryJson["Mappings"][i] << std::endl;
    //   std::cerr << "resp" << respJson["Mappings"][i] << std::endl;
    //   EXPECT_EQ(baseRegistryJson["Mappings"][i]["Entity"],
    //   respJson["Mappings"][i]["Entity"]);
    //   EXPECT_EQ(baseRegistryJson["Mappings"][i]["OperationMap"],
    //   respJson["Mappings"][i]["OperationMap"]);
    //   if(containsOverrides(baseRegistryJson["Mappings"][i])){
    //     std::cout << baseRegistryJson["Mappings"][i]["Entity"] << " contains
    //     override" << std::endl;
    //   }
    // }
}

TEST(PrivilegeRegistryTest, PrivilegeRegistryIsSameAsBaseWithoutPatches)
{
    auto shareAsyncResp = std::make_shared<bmcweb::AsyncResp>();
    shareAsyncResp->res.setCompleteRequestHandler(assertPrivilegeRegistryGet);
    fillInPrivilegeRegistry(shareAsyncResp);
}

} // namespace
} // namespace redfish
