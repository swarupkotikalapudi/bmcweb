/*
// Copyright (c) 2018 Intel Corporation
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

#include "../lib/redfish_sessions.hpp"
#include "../lib/roles.hpp"
#include "../lib/service_root.hpp"

namespace redfish {
/*
 * @brief Top level class installing and providing Redfish services
 */
class RedfishService {
 public:
  /*
   * @brief Redfish service constructor
   *
   * Loads Redfish configuration and installs schema resources
   *
   * @param[in] app   Crow app on which Redfish will initialize
   */
  template <typename CrowApp>
  RedfishService(CrowApp& app) {
    auto privilegeProvider = PrivilegeProvider();

    nodes.emplace_back(
        std::make_unique<SessionCollection>(app, privilegeProvider));
    nodes.emplace_back(std::make_unique<Roles>(app, privilegeProvider));
    nodes.emplace_back(
        std::make_unique<RoleCollection>(app, privilegeProvider));
    nodes.emplace_back(std::make_unique<ServiceRoot>(app, privilegeProvider));
  }

 private:
  std::vector<std::unique_ptr<Node>> nodes;
};

}  // namespace redfish
