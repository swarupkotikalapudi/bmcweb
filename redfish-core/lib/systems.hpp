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

#include <utils/json_utils.hpp>
#include "node.hpp"
#include "boost/container/flat_map.hpp"

namespace redfish {

/**
 * @brief Retrieves computer system properties over dbus
 *
 * @param[in] aResp Shared pointer for completing asynchronous calls
 * @param[in] name  Computer system name from request
 *
 * @return None.
 */
void getComputerSystem(std::shared_ptr<AsyncResp> aResp,
                       const std::string &name) {
  BMCWEB_LOG_DEBUG << "Get available system components.";
  crow::connections::systemBus->async_method_call(
      [ name, aResp{std::move(aResp)} ](
          const boost::system::error_code ec,
          const std::vector<std::pair<
              std::string,
              std::vector<std::pair<std::string, std::vector<std::string>>>>>
              &subtree) {
        if (ec) {
          BMCWEB_LOG_DEBUG << "DBUS response error";
          aResp->res.result(boost::beast::http::status::internal_server_error);
          return;
        }
        bool foundName = false;
        // Iterate over all retrieved ObjectPaths.
        for (const std::pair<
                 std::string,
                 std::vector<std::pair<std::string, std::vector<std::string>>>>
                 &object : subtree) {
          const std::string &path = object.first;
          BMCWEB_LOG_DEBUG << "Got path: " << path;
          const std::vector<std::pair<std::string, std::vector<std::string>>>
              &connectionNames = object.second;
          if (connectionNames.size() < 1) {
            continue;
          }
          // Check if computer system exist
          if (boost::ends_with(path, name)) {
            foundName = true;
            BMCWEB_LOG_DEBUG << "Found name: " << name;
            const std::string connectionName = connectionNames[0].first;
            crow::connections::systemBus->async_method_call(
                [ aResp, name(std::string(name)) ](
                    const boost::system::error_code ec,
                    const std::vector<std::pair<std::string, VariantType>>
                        &propertiesList) {
                  if (ec) {
                    BMCWEB_LOG_ERROR << "DBUS response error: " << ec;
                    aResp->res.result(
                        boost::beast::http::status::internal_server_error);
                    return;
                  }
                  BMCWEB_LOG_DEBUG << "Got " << propertiesList.size()
                                   << "properties for system";
                  for (const std::pair<std::string, VariantType> &property :
                       propertiesList) {
                    const std::string *value =
                        mapbox::getPtr<const std::string>(property.second);
                    if (value != nullptr) {
                      aResp->res.jsonValue[property.first] = *value;
                    }
                  }
                  aResp->res.jsonValue["Name"] = name;
                  aResp->res.jsonValue["Id"] =
                      aResp->res.jsonValue["SerialNumber"];
                },
                connectionName, path, "org.freedesktop.DBus.Properties",
                "GetAll", "xyz.openbmc_project.Inventory.Decorator.Asset");
          } else {
            // This is not system, so check if it's cpu, dimm, UUID or BiosVer
            for (auto const &s : connectionNames) {
              for (auto const &i : s.second) {
                if (boost::ends_with(i, "Dimm")) {
                  BMCWEB_LOG_DEBUG << "Found Dimm, now get it properties.";
                  crow::connections::systemBus->async_method_call(
                      [aResp](
                          const boost::system::error_code ec,
                          const std::vector<std::pair<std::string, VariantType>>
                              &properties) {
                        if (ec) {
                          BMCWEB_LOG_ERROR << "DBUS response error " << ec;
                          aResp->res.result(boost::beast::http::status::
                                                internal_server_error);
                          return;
                        }
                        BMCWEB_LOG_DEBUG << "Got " << properties.size()
                                         << "Dimm properties.";
                        for (const auto &p : properties) {
                          if (p.first == "MemorySize") {
                            const std::string *value =
                                mapbox::getPtr<const std::string>(p.second);
                            if ((value != nullptr) && (*value != "NULL")) {
                              // Remove units char
                              int32_t unitCoeff;
                              if (boost::ends_with(*value, "MB")) {
                                unitCoeff = 1000;
                              } else if (boost::ends_with(*value, "KB")) {
                                unitCoeff = 1000000;
                              } else {
                                BMCWEB_LOG_ERROR << "Unsupported memory units";
                                aResp->res.result(boost::beast::http::status::
                                                      internal_server_error);
                                return;
                              }

                              auto memSize = boost::lexical_cast<int>(
                                  value->substr(0, value->length() - 2));
                              aResp->res.jsonValue["TotalSystemMemoryGiB"] +=
                                  memSize * unitCoeff;
                              aResp->res.jsonValue["MemorySummary"]["Status"]
                                                  ["State"] = "Enabled";
                            }
                          }
                        }
                      },
                      s.first, path, "org.freedesktop.DBus.Properties",
                      "GetAll", "xyz.openbmc_project.Inventory.Item.Dimm");
                } else if (boost::ends_with(i, "Cpu")) {
                  BMCWEB_LOG_DEBUG << "Found Cpu, now get it properties.";
                  crow::connections::systemBus->async_method_call(
                      [aResp](
                          const boost::system::error_code ec,
                          const std::vector<std::pair<std::string, VariantType>>
                              &properties) {
                        if (ec) {
                          BMCWEB_LOG_ERROR << "DBUS response error " << ec;
                          aResp->res.result(boost::beast::http::status::
                                                internal_server_error);
                          return;
                        }
                        BMCWEB_LOG_DEBUG << "Got " << properties.size()
                                         << "Cpu properties.";
                        for (const auto &p : properties) {
                          if (p.first == "ProcessorFamily") {
                            const std::string *value =
                                mapbox::getPtr<const std::string>(p.second);
                            if (value != nullptr) {
                              aResp->res
                                  .jsonValue["ProcessorSummary"]["Count"] =
                                  aResp->res
                                      .jsonValue["ProcessorSummary"]["Count"]
                                      .get<int>() +
                                  1;
                              aResp->res.jsonValue["ProcessorSummary"]["Status"]
                                                  ["State"] = "Enabled";
                              aResp->res
                                  .jsonValue["ProcessorSummary"]["Model"] =
                                  *value;
                            }
                          }
                        }
                      },
                      s.first, path, "org.freedesktop.DBus.Properties",
                      "GetAll", "xyz.openbmc_project.Inventory.Item.Cpu");
                } else if (boost::ends_with(i, "UUID")) {
                  BMCWEB_LOG_DEBUG << "Found UUID, now get it properties.";
                  crow::connections::systemBus->async_method_call(
                      [aResp](
                          const boost::system::error_code ec,
                          const std::vector<std::pair<std::string, VariantType>>
                              &properties) {
                        if (ec) {
                          BMCWEB_LOG_DEBUG << "DBUS response error " << ec;
                          aResp->res.result(boost::beast::http::status::
                                                internal_server_error);
                          return;
                        }
                        BMCWEB_LOG_DEBUG << "Got " << properties.size()
                                         << "UUID properties.";
                        for (const std::pair<std::string, VariantType> &p :
                             properties) {
                          if (p.first == "BIOSVer") {
                            const std::string *value =
                                mapbox::getPtr<const std::string>(p.second);
                            if (value != nullptr) {
                              aResp->res.jsonValue["BiosVersion"] = *value;
                            }
                          }
                          if (p.first == "UUID") {
                            const std::string *value =
                                mapbox::getPtr<const std::string>(p.second);
                            BMCWEB_LOG_DEBUG << "UUID = " << *value
                                             << " length " << value->length();
                            if (value != nullptr) {
                              // Workaround for to short return str in smbios
                              // demo app, 32 bytes are described by spec
                              if (value->length() > 0 && value->length() < 32) {
                                std::string correctedValue = *value;
                                correctedValue.append(32 - value->length(),
                                                      '0');
                                value = &correctedValue;
                              } else if (value->length() == 32) {
                                aResp->res.jsonValue["UUID"] =
                                    value->substr(0, 8) + "-" +
                                    value->substr(8, 4) + "-" +
                                    value->substr(12, 4) + "-" +
                                    value->substr(16, 4) + "-" +
                                    value->substr(20, 12);
                              }
                            }
                          }
                        }
                      },
                      s.first, path, "org.freedesktop.DBus.Properties",
                      "GetAll", "xyz.openbmc_project.Common.UUID");
                }
              }
            }
          }
        }
        if (foundName == false) {
          aResp->res.result(boost::beast::http::status::internal_server_error);
        }
      },
      "xyz.openbmc_project.ObjectMapper", "/xyz/openbmc_project/object_mapper",
      "xyz.openbmc_project.ObjectMapper", "GetSubTree",
      "/xyz/openbmc_project/inventory", int32_t(0),
      std::array<const char *, 5>{
          "xyz.openbmc_project.Inventory.Decorator.Asset",
          "xyz.openbmc_project.Inventory.Item.Cpu",
          "xyz.openbmc_project.Inventory.Item.Dimm",
          "xyz.openbmc_project.Inventory.Item.System",
          "xyz.openbmc_project.Common.UUID",
      });
}

/**
 * @brief Retrieves identify led group properties over dbus
 *
 * @param[in] aResp     Shared pointer for completing asynchronous calls.
 * @param[in] callback  Callback for process retrieved data.
 *
 * @return None.
 */
template <typename CallbackFunc>
void getLedGroupIdentify(std::shared_ptr<AsyncResp> aResp,
                         CallbackFunc &&callback) {
  BMCWEB_LOG_DEBUG << "Get led groups";
  crow::connections::systemBus->async_method_call(
      [ aResp{std::move(aResp)}, callback{std::move(callback)} ](
          const boost::system::error_code &ec, const ManagedObjectsType &resp) {
        if (ec) {
          BMCWEB_LOG_DEBUG << "DBUS response error " << ec;
          aResp->res.result(boost::beast::http::status::internal_server_error);
          return;
        }
        BMCWEB_LOG_DEBUG << "Got " << resp.size() << "led group objects.";
        for (const auto &objPath : resp) {
          const std::string &path = objPath.first;
          if (path.rfind("enclosure_identify") != std::string::npos) {
            for (const auto &interface : objPath.second) {
              if (interface.first == "xyz.openbmc_project.Led.Group") {
                for (const auto &property : interface.second) {
                  if (property.first == "Asserted") {
                    const bool *asserted =
                        mapbox::getPtr<const bool>(property.second);
                    if (nullptr != asserted) {
                      callback(*asserted, aResp);
                    } else {
                      callback(false, aResp);
                    }
                  }
                }
              }
            }
          }
        }
      },
      "xyz.openbmc_project.LED.GroupManager", "/xyz/openbmc_project/led/groups",
      "org.freedesktop.DBus.ObjectManager", "GetManagedObjects");
}

template <typename CallbackFunc>
void getLedIdentify(std::shared_ptr<AsyncResp> aResp, CallbackFunc &&callback) {
  BMCWEB_LOG_DEBUG << "Get identify led properties";
  crow::connections::systemBus->async_method_call(
      [
        aResp, callback{std::move(callback)}
      ](const boost::system::error_code ec, const PropertiesType &properties) {
        if (ec) {
          BMCWEB_LOG_DEBUG << "DBUS response error " << ec;
          aResp->res.result(boost::beast::http::status::internal_server_error);
          return;
        }
        BMCWEB_LOG_DEBUG << "Got " << properties.size() << "led properties.";
        std::string output;
        for (const auto &property : properties) {
          if (property.first == "State") {
            const std::string *s = mapbox::getPtr<std::string>(property.second);
            if (nullptr != s) {
              BMCWEB_LOG_DEBUG << "Identify Led State: " << *s;
              const auto pos = s->rfind('.');
              if (pos != std::string::npos) {
                auto led = s->substr(pos + 1);
                for (const std::pair<const char *, const char *> &p :
                     std::array<std::pair<const char *, const char *>, 3>{
                         {{"On", "Lit"},
                          {"Blink", "Blinking"},
                          {"Off", "Off"}}}) {
                  if (led == p.first) {
                    output = p.second;
                  }
                }
              }
            }
          }
        }
        callback(output, aResp);
      },
      "xyz.openbmc_project.LED.Controller.identify",
      "/xyz/openbmc_project/led/physical/identify",
      "org.freedesktop.DBus.Properties", "GetAll",
      "xyz.openbmc_project.Led.Physical");
}

/**
 * @brief Retrieves host state properties over dbus
 *
 * @param[in] aResp     Shared pointer for completing asynchronous calls.
 *
 * @return None.
 */
void getHostState(std::shared_ptr<AsyncResp> aResp) {
  BMCWEB_LOG_DEBUG << "Get host information.";
  crow::connections::systemBus->async_method_call(
      [aResp{std::move(aResp)}](
          const boost::system::error_code ec,
          const sdbusplus::message::variant<std::string> &hostState) {
        if (ec) {
          BMCWEB_LOG_DEBUG << "DBUS response error " << ec;
          aResp->res.result(boost::beast::http::status::internal_server_error);
          return;
        }

        const std::string *s = mapbox::getPtr<const std::string>(hostState);
        BMCWEB_LOG_DEBUG << "Host state: " << *s;
        if (s != nullptr) {
          // Verify Host State
          if (*s == "xyz.openbmc_project.State.Host.Running") {
            aResp->res.jsonValue["PowerState"] = "On";
            aResp->res.jsonValue["Status"]["State"] = "Enabled";
          } else {
            aResp->res.jsonValue["PowerState"] = "Off";
            aResp->res.jsonValue["Status"]["State"] = "Disabled";
          }
        }
      },
      "xyz.openbmc_project.State.Host", "/xyz/openbmc_project/state/host0",
      "org.freedesktop.DBus.Properties", "Get",
      "xyz.openbmc_project.State.Host", "CurrentHostState");
}

/**
 * SystemsCollection derived class for delivering ComputerSystems Collection
 * Schema
 */
class SystemsCollection : public Node {
 public:
  SystemsCollection(CrowApp &app) : Node(app, "/redfish/v1/Systems/") {
    Node::json["@odata.type"] =
        "#ComputerSystemCollection.ComputerSystemCollection";
    Node::json["@odata.id"] = "/redfish/v1/Systems";
    Node::json["@odata.context"] =
        "/redfish/v1/"
        "$metadata#ComputerSystemCollection.ComputerSystemCollection";
    Node::json["Name"] = "Computer System Collection";

    entityPrivileges = {
        {boost::beast::http::verb::get, {{"Login"}}},
        {boost::beast::http::verb::head, {{"Login"}}},
        {boost::beast::http::verb::patch, {{"ConfigureComponents"}}},
        {boost::beast::http::verb::put, {{"ConfigureComponents"}}},
        {boost::beast::http::verb::delete_, {{"ConfigureComponents"}}},
        {boost::beast::http::verb::post, {{"ConfigureComponents"}}}};
  }

 private:
  void doGet(crow::Response &res, const crow::Request &req,
             const std::vector<std::string> &params) override {
    BMCWEB_LOG_DEBUG << "Get list of available boards.";
    std::shared_ptr<AsyncResp> asyncResp = std::make_shared<AsyncResp>(res);
    res.jsonValue = Node::json;
    crow::connections::systemBus->async_method_call(
        [asyncResp](const boost::system::error_code ec,
                    const std::vector<std::string> &resp) {
          if (ec) {
            asyncResp->res.result(
                boost::beast::http::status::internal_server_error);
            return;
          }
          BMCWEB_LOG_DEBUG << "Got " << resp.size() << " boards.";

          // ... prepare json array with appropriate @odata.id links
          nlohmann::json &boardArray = asyncResp->res.jsonValue["Members"];
          boardArray = nlohmann::json::array();

          // Iterate over all retrieved ObjectPaths.
          for (const std::string &objpath : resp) {
            std::size_t lastPos = objpath.rfind("/");
            if (lastPos != std::string::npos) {
              boardArray.push_back(
                  {{"@odata.id",
                    "/redfish/v1/Systems/" + objpath.substr(lastPos + 1)}});
            }
          }

          asyncResp->res.jsonValue["Members@odata.count"] = boardArray.size();
        },
        "xyz.openbmc_project.ObjectMapper",
        "/xyz/openbmc_project/object_mapper",
        "xyz.openbmc_project.ObjectMapper", "GetSubTreePaths",
        "/xyz/openbmc_project/inventory", int32_t(0),
        std::array<const char *, 1>{
            "xyz.openbmc_project.Inventory.Item.Board"});
  }
};

/**
 * Systems derived class for delivering Computer Systems Schema.
 */
class Systems : public Node {
 public:
  /*
   * Default Constructor
   */
  Systems(CrowApp &app)
      : Node(app, "/redfish/v1/Systems/<str>/", std::string()) {
    Node::json["@odata.type"] = "#ComputerSystem.v1_3_0.ComputerSystem";
    Node::json["@odata.context"] =
        "/redfish/v1/$metadata#ComputerSystem.ComputerSystem";
    Node::json["SystemType"] = "Physical";
    Node::json["Description"] = "Computer System";
    Node::json["Boot"]["BootSourceOverrideEnabled"] =
        "Disabled";  // TODO(Dawid), get real boot data
    Node::json["Boot"]["BootSourceOverrideTarget"] =
        "None";  // TODO(Dawid), get real boot data
    Node::json["Boot"]["BootSourceOverrideMode"] =
        "Legacy";  // TODO(Dawid), get real boot data
    Node::json["Boot"]["BootSourceOverrideTarget@Redfish.AllowableValues"] = {
        "None",      "Pxe",       "Hdd", "Cd",
        "BiosSetup", "UefiShell", "Usb"};  // TODO(Dawid), get real boot data
    Node::json["ProcessorSummary"]["Count"] = int(0);
    Node::json["ProcessorSummary"]["Status"]["State"] = "Disabled";
    Node::json["MemorySummary"]["TotalSystemMemoryGiB"] = int(0);
    Node::json["MemorySummary"]["Status"]["State"] = "Disabled";
    entityPrivileges = {
        {boost::beast::http::verb::get, {{"Login"}}},
        {boost::beast::http::verb::head, {{"Login"}}},
        {boost::beast::http::verb::patch, {{"ConfigureComponents"}}},
        {boost::beast::http::verb::put, {{"ConfigureComponents"}}},
        {boost::beast::http::verb::delete_, {{"ConfigureComponents"}}},
        {boost::beast::http::verb::post, {{"ConfigureComponents"}}}};
  }

 private:
  /**
   * Functions triggers appropriate requests on DBus
   */
  void doGet(crow::Response &res, const crow::Request &req,
             const std::vector<std::string> &params) override {
    // Check if there is required param, truly entering this shall be
    // impossible
    if (params.size() != 1) {
      res.result(boost::beast::http::status::internal_server_error);
      res.end();
      return;
    }

    const std::string &name = params[0];

    res.jsonValue = Node::json;
    res.jsonValue["@odata.id"] = "/redfish/v1/Systems/" + name;

    auto asyncResp = std::make_shared<AsyncResp>(res);

    getLedGroupIdentify(
        asyncResp,
        [&](const bool &asserted, const std::shared_ptr<AsyncResp> &aResp) {
          if (asserted) {
            // If led group is asserted, then another call is needed to
            // get led status
            getLedIdentify(aResp, [](const std::string &ledStatus,
                                     const std::shared_ptr<AsyncResp> &aResp) {
              if (!ledStatus.empty()) {
                aResp->res.jsonValue["IndicatorLED"] = ledStatus;
              }
            });
          } else {
            aResp->res.jsonValue["IndicatorLED"] = "Off";
          }
        });
    getComputerSystem(asyncResp, name);
    getHostState(asyncResp);
  }

  void doPatch(crow::Response &res, const crow::Request &req,
               const std::vector<std::string> &params) override {
    // Check if there is required param, truly entering this shall be
    // impossible
    auto asyncResp = std::make_shared<AsyncResp>(res);
    if (params.size() != 1) {
      res.result(boost::beast::http::status::internal_server_error);
      return;
    }
    // Parse JSON request body
    nlohmann::json patch;
    if (!json_util::processJsonFromRequest(res, req, patch)) {
      return;
    }

    const std::string &name = params[0];

    res.jsonValue = Node::json;
    res.jsonValue["@odata.id"] = "/redfish/v1/Systems/" + name;

    for (const auto &item : patch.items()) {
      if (item.key() == "IndicatorLed") {
        const std::string *reqLedState =
            item.value().get_ptr<const std::string *>();
        if (reqLedState == nullptr) {
          messages::addMessageToErrorJson(asyncResp->res.jsonValue,
                                          messages::propertyValueFormatError(
                                              item.value().dump(), item.key()));
          return;
        }

        // Verify key value
        std::string dbusLedState;
        if (*reqLedState == "On") {
          dbusLedState = "xyz.openbmc_project.Led.Physical.Action.Lit";
        } else if (*reqLedState == "Blink") {
          dbusLedState = "xyz.openbmc_project.Led.Physical.Action.Blinking";
        } else if (*reqLedState == "Off") {
          dbusLedState = "xyz.openbmc_project.Led.Physical.Action.Off";
        } else {
          messages::addMessageToJsonRoot(
              res.jsonValue,
              messages::propertyValueNotInList(*reqLedState, "IndicatorLED"));
          return;
        }

        getHostState(asyncResp);
        getComputerSystem(asyncResp, name);

        // Update led group
        BMCWEB_LOG_DEBUG << "Update led group.";
        crow::connections::systemBus->async_method_call(
            [asyncResp{std::move(asyncResp)}](
                const boost::system::error_code ec) {
              if (ec) {
                BMCWEB_LOG_DEBUG << "DBUS response error " << ec;
                asyncResp->res.result(
                    boost::beast::http::status::internal_server_error);
                return;
              }
              BMCWEB_LOG_DEBUG << "Led group update done.";
            },
            "xyz.openbmc_project.LED.GroupManager",
            "/xyz/openbmc_project/led/groups/enclosure_identify",
            "org.freedesktop.DBus.Properties", "Set",
            "xyz.openbmc_project.Led.Group", "Asserted",
            sdbusplus::message::variant<bool>(
                (dbusLedState == "xyz.openbmc_project.Led.Physical.Action.Off"
                     ? false
                     : true)));
        // Update identify led status
        BMCWEB_LOG_DEBUG << "Update led SoftwareInventoryCollection.";
        crow::connections::systemBus->async_method_call(
            [
              asyncResp{std::move(asyncResp)},
              reqLedState{std::move(*reqLedState)}
            ](const boost::system::error_code ec) {
              if (ec) {
                BMCWEB_LOG_DEBUG << "DBUS response error " << ec;
                asyncResp->res.result(
                    boost::beast::http::status::internal_server_error);
                return;
              }
              BMCWEB_LOG_DEBUG << "Led state update done.";
              asyncResp->res.jsonValue["IndicatorLED"] = std::move(reqLedState);
            },
            "xyz.openbmc_project.LED.Controller.identify",
            "/xyz/openbmc_project/led/physical/identify",
            "org.freedesktop.DBus.Properties", "Set",
            "xyz.openbmc_project.Led.Physical", "State",
            sdbusplus::message::variant<std::string>(dbusLedState));
      } else {
        messages::addMessageToErrorJson(
            asyncResp->res.jsonValue,
            messages::propertyNotWritable(item.key()));
        return;
      }
    }
  }
};
}  // namespace redfish
