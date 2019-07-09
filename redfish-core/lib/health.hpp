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

#include "async_resp.hpp"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/container/flat_set.hpp>
#include <dbus_singleton.hpp>
#include <variant>

namespace redfish
{

struct HealthPopulate : std::enable_shared_from_this<HealthPopulate>
{
    HealthPopulate(const std::shared_ptr<AsyncResp> &asyncResp) :
        asyncResp(asyncResp)
    {
    }

    ~HealthPopulate()
    {
        nlohmann::json &health = asyncResp->res.jsonValue["Status"]["Health"];
        nlohmann::json &rollup =
            asyncResp->res.jsonValue["Status"]["HealthRollup"];

        health = "OK";
        rollup = "OK";

        for (const auto &[path, interfaces] : statuses)
        {
            bool isChild = false;

            // managers inventory is all the inventory, don't skip any
            if (!isManagersHealth)
            {

                // We only want to look at this association if either the path
                // of this association is an inventory item, or one of the
                // endpoints in this association is a child

                for (const std::string &child : inventory)
                {
                    if (boost::starts_with(path.str, child))
                    {
                        isChild = true;
                        break;
                    }
                }
                if (!isChild)
                {
                    auto assocIt =
                        interfaces.find("xyz.openbmc_project.Association");
                    if (assocIt == interfaces.end())
                    {
                        continue;
                    }
                    auto endpointsIt = assocIt->second.find("endpoints");
                    if (endpointsIt == assocIt->second.end())
                    {
                        BMCWEB_LOG_ERROR << "Illegal association at "
                                         << path.str;
                        continue;
                    }
                    const std::vector<std::string> *endpoints =
                        std::get_if<std::vector<std::string>>(
                            &endpointsIt->second);
                    if (endpoints == nullptr)
                    {
                        BMCWEB_LOG_ERROR << "Illegal association at "
                                         << path.str;
                        continue;
                    }
                    bool containsChild = false;
                    for (const std::string &endpoint : *endpoints)
                    {
                        if (std::find(inventory.begin(), inventory.end(),
                                      endpoint) != inventory.end())
                        {
                            containsChild = true;
                            break;
                        }
                    }
                    if (!containsChild)
                    {
                        continue;
                    }
                }
            }

            if (boost::starts_with(path.str, globalInventoryPath) &&
                boost::ends_with(path.str, "critical"))
            {
                health = "Critical";
                rollup = "Critical";
                return;
            }
            else if (boost::starts_with(path.str, globalInventoryPath) &&
                     boost::ends_with(path.str, "warning"))
            {
                health = "Warning";
                if (rollup != "Critical")
                {
                    rollup = "Warning";
                }
            }
            else if (boost::ends_with(path.str, "critical"))
            {
                rollup = "Critical";
            }
            else if (boost::ends_with(path.str, "warning"))
            {
                if (rollup != "Critical")
                {
                    rollup = "Warning";
                }
            }
        }
    }

    void populate()
    {
        getAllStatusAssociations();
        getGlobalPath();
    }

    void getGlobalPath()
    {
        std::shared_ptr<HealthPopulate> self = shared_from_this();
        crow::connections::systemBus->async_method_call(
            [self](const boost::system::error_code ec,
                   std::vector<std::string> &resp) {
                if (ec || resp.size() != 1)
                {
                    // no global item, or too many
                    return;
                }
                self->globalInventoryPath = std::move(resp[0]);
            },
            "xyz.openbmc_project.ObjectMapper",
            "/xyz/openbmc_project/object_mapper",
            "xyz.openbmc_project.ObjectMapper", "GetSubTreePaths", "/", 0,
            std::array<const char *, 1>{
                "xyz.openbmc_project.Inventory.Item.Global"});
    }

    void getAllStatusAssociations()
    {
        std::shared_ptr<HealthPopulate> self = shared_from_this();
        crow::connections::systemBus->async_method_call(
            [self](const boost::system::error_code ec,
                   dbus::utility::ManagedObjectType &resp) {
                if (ec)
                {
                    return;
                }
                for (auto it = resp.begin(); it != resp.end();)
                {
                    if (boost::ends_with(it->first.str, "critical") ||
                        boost::ends_with(it->first.str, "warning"))
                    {
                        it++;
                        continue;
                    }
                    it = resp.erase(it);
                }
                self->statuses = std::move(resp);
            },
            "xyz.openbmc_project.ObjectMapper", "/",
            "org.freedesktop.DBus.ObjectManager", "GetManagedObjects");
    }

    std::shared_ptr<AsyncResp> asyncResp;
    std::vector<std::string> inventory;
    bool isManagersHealth = false;
    dbus::utility::ManagedObjectType statuses;
    std::string globalInventoryPath = "-"; // default to illegal dbus path
};
} // namespace redfish