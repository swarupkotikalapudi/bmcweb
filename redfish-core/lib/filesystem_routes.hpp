/*
// Copyright (c) 2022 Intel Corporation
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

#include <bmcweb_config.h>

#include <app.hpp>
#include <async_resp.hpp>
#include <http_request.hpp>
#include <nlohmann/json.hpp>
#include <persistent_data.hpp>
#include <query.hpp>
#include <registries/privilege_registry.hpp>
#include <utils/systemd_utils.hpp>

#include <filesystem>

namespace redfish
{

inline void
    handleFileSystemGet(App& app, const crow::Request& req,
                        const std::shared_ptr<bmcweb::AsyncResp>& asyncResp)
{
    std::cerr << "handleFileSystemGet: " << req.url << "\n";
    asyncResp->res.jsonValue["@odata.type"] = "OEM.Test";
    if (!redfish::setUpRedfishRoute(app, req, asyncResp))
    {
        return;
    }
}

inline void requestRoutesFromFilesystem(App& app)
{
    const std::filesystem::path redfishDir = "/var/redfish/v1";

    if (!std::filesystem::exists(redfishDir))
    {
        std::cerr << "No redfish dir found\n";
        return;
    }

    for (const std::filesystem::directory_entry& dirEntry :
         std::filesystem::recursive_directory_iterator(redfishDir))
    {
        std::cerr << dirEntry << '\n';

        if (!dirEntry.is_directory() || dirEntry.is_symlink())
        {
            continue;
        }
        std::string url(
            "/" + std::filesystem::relative(dirEntry.path(), "/var").string() +
            "/");
        std::cerr << url << '\n';
        app.template route<0>(std::move(url))
            .privileges(redfish::privileges::privilegeSetConfigureComponents)
            .methods(boost::beast::http::verb::get)(
                std::bind_front(handleFileSystemGet, std::ref(app)));
    }
}

} // namespace redfish
