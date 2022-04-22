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

#include "logging.hpp"

#include <account_service.hpp>
#include <app.hpp>
#include <async_resp.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/process/async_pipe.hpp>
#include <boost/type_traits/has_dereference.hpp>
#include <boost/url/url_view.hpp>
#include <dbus_singleton.hpp>
#include <query.hpp>
#include <registries/privilege_registry.hpp>
#include <sdbusplus/bus/match.hpp>
#include <sdbusplus/message/native_types.hpp>
#include <utils/json_utils.hpp>

#include <functional>

namespace redfish
{

const char* legacyMode = "Legacy";
const char* proxyMode = "Proxy";

std::string getModeName(bool isLegacy)
{
    if (isLegacy)
    {
        return legacyMode;
    }
    return proxyMode;
}

std::optional<std::string>
    parseObjectPathAndGetMode(const sdbusplus::message::object_path& itemPath,
                              const std::string& resName)
{
    std::string thisPath = itemPath.filename();
    BMCWEB_LOG_DEBUG << "Filename: " << itemPath.str
                     << ", ThisPath: " << thisPath;

    if (thisPath.empty())
    {
        return std::nullopt;
    }

    if (thisPath != resName)
    {
        return std::nullopt;
    }

    auto mode = itemPath.parent_path();
    auto type = mode.parent_path();

    if (mode.filename().empty() || type.filename().empty())
    {
        return std::nullopt;
    }

    if (type.filename() != "VirtualMedia")
    {
        return std::nullopt;
    }

    return {mode.filename()};
}

using CheckItemHandler = std::function<bool(
    const std::string& service, const std::shared_ptr<bmcweb::AsyncResp>&,
    std::pair<sdbusplus::message::object_path,
              dbus::utility::DBusInteracesMap>&)>;

void findAndParseObject(const std::string& service, const std::string& resName,
                        const std::shared_ptr<bmcweb::AsyncResp>& aResp,
                        CheckItemHandler&& handler)
{
    crow::connections::systemBus->async_method_call(
        [service, resName, aResp,
         handler](const boost::system::error_code ec,
                  dbus::utility::ManagedObjectType& subtree) {
        if (ec)
        {
            BMCWEB_LOG_DEBUG << "DBUS response error";

            return;
        }

        for (auto& item : subtree)
        {
            auto mode = parseObjectPathAndGetMode(item.first, resName);

            if (mode && handler(service, aResp, item))
            {
                return;
            }
        }

        BMCWEB_LOG_DEBUG << "Parent item not found";
        aResp->res.result(boost::beast::http::status::not_found);
        },
        service, "/xyz/openbmc_project/VirtualMedia",
        "org.freedesktop.DBus.ObjectManager", "GetManagedObjects");
}

/**
 * @brief Function parses getManagedObject response, finds item, makes
 * generic validation and invokes callback handler on this item.
 *
 */
void findItemAndRunHandler(const std::shared_ptr<bmcweb::AsyncResp>& aResp,
                           const std::string& name, const std::string& resName,
                           CheckItemHandler&& handler)
{
    if (name != "bmc")
    {
        messages::resourceNotFound(aResp->res, "VirtualMedia.InsertMedia",
                                   resName);

        return;
    }

    crow::connections::systemBus->async_method_call(
        [aResp, resName,
         handler](const boost::system::error_code ec,
                  const dbus::utility::MapperGetObject& getObjectType) mutable {
        if (ec)
        {
            BMCWEB_LOG_ERROR << "ObjectMapper::GetObject call failed: " << ec;
            aResp->res.result(boost::beast::http::status::not_found);

            return;
        }

        if (getObjectType.empty())
        {
            BMCWEB_LOG_ERROR << "ObjectMapper : No Service found";
            aResp->res.result(boost::beast::http::status::not_found);
            return;
        }

        std::string service = getObjectType.begin()->first;
        BMCWEB_LOG_DEBUG << "GetObjectType: " << service;

        findAndParseObject(service, resName, aResp, std::move(handler));
        },
        "xyz.openbmc_project.ObjectMapper",
        "/xyz/openbmc_project/object_mapper",
        "xyz.openbmc_project.ObjectMapper", "GetObject",
        "/xyz/openbmc_project/VirtualMedia", std::array<const char*, 0>());
}

/**
 * @brief Function extracts transfer protocol name from URI.
 */
inline std::string getTransferProtocolTypeFromUri(const std::string& imageUri)
{
    boost::urls::result<boost::urls::url_view> url =
        boost::urls::parse_uri(boost::string_view(imageUri));
    if (!url)
    {
        return "None";
    }
    boost::string_view scheme = url->scheme();
    if (scheme == "smb")
    {
        return "CIFS";
    }
    if (scheme == "https")
    {
        return "HTTPS";
    }

    return "None";
}

/**
 * @brief Read all known properties from VM object interfaces
 */
inline void
    vmParseInterfaceObject(const dbus::utility::DBusInteracesMap& interface,
                           const std::shared_ptr<bmcweb::AsyncResp>& aResp)
{
    for (const auto& [interface, values] : interface)
    {
        if (interface == "xyz.openbmc_project.VirtualMedia.MountPoint")
        {
            for (const auto& [property, value] : values)
            {
                if (property == "EndpointId")
                {
                    const std::string* endpointIdValue =
                        std::get_if<std::string>(&value);
                    if (endpointIdValue == nullptr)
                    {
                        continue;
                    }
                    if (!endpointIdValue->empty())
                    {
                        // Proxy mode
                        aResp->res
                            .jsonValue["Oem"]["OpenBMC"]["WebSocketEndpoint"] =
                            *endpointIdValue;
                        aResp->res.jsonValue["TransferProtocolType"] = "OEM";
                    }
                }
                if (property == "ImageURL")
                {
                    const std::string* imageUrlValue =
                        std::get_if<std::string>(&value);
                    if (imageUrlValue != nullptr && !imageUrlValue->empty())
                    {
                        std::filesystem::path filePath = *imageUrlValue;
                        if (!filePath.has_filename())
                        {
                            // this will handle https share, which not
                            // necessarily has to have filename given.
                            aResp->res.jsonValue["ImageName"] = "";
                        }
                        else
                        {
                            aResp->res.jsonValue["ImageName"] =
                                filePath.filename();
                        }

                        aResp->res.jsonValue["Image"] = *imageUrlValue;
                        aResp->res.jsonValue["TransferProtocolType"] =
                            getTransferProtocolTypeFromUri(*imageUrlValue);

                        aResp->res.jsonValue["ConnectedVia"] = "URI";
                    }
                }
                if (property == "WriteProtected")
                {
                    const bool* writeProtectedValue = std::get_if<bool>(&value);
                    if (writeProtectedValue != nullptr)
                    {
                        aResp->res.jsonValue["WriteProtected"] =
                            *writeProtectedValue;
                    }
                }
            }
        }
        if (interface == "xyz.openbmc_project.VirtualMedia.Process")
        {
            for (const auto& [property, value] : values)
            {
                if (property == "Active")
                {
                    const bool* activeValue = std::get_if<bool>(&value);
                    if (activeValue == nullptr)
                    {
                        BMCWEB_LOG_DEBUG << "Value Active not found";
                        return;
                    }
                    aResp->res.jsonValue["Inserted"] = *activeValue;

                    if (*activeValue)
                    {
                        aResp->res.jsonValue["ConnectedVia"] = "Applet";
                    }
                }
            }
        }
    }
}

/**
 * @brief Fill template for Virtual Media Item.
 */
inline nlohmann::json vmItemTemplate(const std::string& name,
                                     const std::string& resName)
{
    nlohmann::json item;

    std::string id = "/redfish/v1/Managers/";
    id += name;
    id += "/VirtualMedia/";
    id += resName;
    item["@odata.id"] = std::move(id);

    item["@odata.type"] = "#VirtualMedia.v1_3_0.VirtualMedia";
    item["ConnectedVia"] = "NotConnected";
    item["Name"] = "Virtual Removable Media";
    item["Id"] = resName;
    item["WriteProtected"] = true;
    item["MediaTypes"] = {"CD", "USBStick"};
    item["TransferMethod"] = "Stream";
    item["Oem"]["OpenBMC"]["@odata.type"] =
        "#OemVirtualMedia.v1_0_0.VirtualMedia";

    return item;
}

/**
 *  @brief Fills collection data
 */
inline void getVmResourceList(std::shared_ptr<bmcweb::AsyncResp> aResp,
                              const std::string& service,
                              const std::string& name)
{
    BMCWEB_LOG_DEBUG << "Get available Virtual Media resources.";
    crow::connections::systemBus->async_method_call(
        [name,
         aResp{std::move(aResp)}](const boost::system::error_code ec,
                                  dbus::utility::ManagedObjectType& subtree) {
        if (ec)
        {
            BMCWEB_LOG_DEBUG << "DBUS response error";
            return;
        }
        nlohmann::json& members = aResp->res.jsonValue["Members"];
        members = nlohmann::json::array();

        for (const auto& object : subtree)
        {
            nlohmann::json item;
            std::string path = object.first.filename();
            if (path.empty())
            {
                continue;
            }

            std::string id = "/redfish/v1/Managers/";
            id += name;
            id += "/VirtualMedia/";
            id += path;

            item["@odata.id"] = std::move(id);
            members.emplace_back(std::move(item));
        }
        aResp->res.jsonValue["Members@odata.count"] = members.size();
        },
        service, "/xyz/openbmc_project/VirtualMedia",
        "org.freedesktop.DBus.ObjectManager", "GetManagedObjects");
}

/**
 *  @brief Fills data for specific resource
 */
inline void getVmData(const std::shared_ptr<bmcweb::AsyncResp>& aResp,
                      const std::string& service, const std::string& name,
                      const std::string& resName)
{
    BMCWEB_LOG_DEBUG << "Get Virtual Media resource data.";

    crow::connections::systemBus->async_method_call(
        [resName, name,
         aResp](const boost::system::error_code ec,
                const dbus::utility::ManagedObjectType& subtree) {
        if (ec)
        {
            BMCWEB_LOG_DEBUG << "DBUS response error";

            return;
        }

        for (const auto& item : subtree)
        {
            auto mode = parseObjectPathAndGetMode(item.first, resName);
            if (mode == std::nullopt)
            {
                continue;
            }

            aResp->res.jsonValue = vmItemTemplate(name, resName);
            std::string actionsId = "/redfish/v1/Managers/";
            actionsId += name;
            actionsId += "/VirtualMedia/";
            actionsId += resName;
            actionsId += "/Actions";

            // Check if dbus path is Legacy type
            if (*mode == legacyMode)
            {
                aResp->res.jsonValue["Actions"]["#VirtualMedia.InsertMedia"]
                                    ["target"] =
                    actionsId + "/VirtualMedia.InsertMedia";
            }

            vmParseInterfaceObject(item.second, aResp);

            aResp->res
                .jsonValue["Actions"]["#VirtualMedia.EjectMedia"]["target"] =
                actionsId + "/VirtualMedia.EjectMedia";

            return;
        }

        messages::resourceNotFound(
            aResp->res, "#VirtualMedia.v1_3_0.VirtualMedia", resName);
        },
        service, "/xyz/openbmc_project/VirtualMedia",
        "org.freedesktop.DBus.ObjectManager", "GetManagedObjects");
}

/**
 * @brief Transfer protocols supported for InsertMedia action.
 *
 */
enum class TransferProtocol
{
    https,
    smb,
    invalid
};

/**
 * @brief Function extracts transfer protocol type from URI.
 *
 */
inline std::optional<TransferProtocol>
    getTransferProtocolFromUri(const boost::urls::url_view& imageUri)
{
    boost::string_view scheme = imageUri.scheme();
    if (scheme == "smb")
    {
        return TransferProtocol::smb;
    }
    if (scheme == "https")
    {
        return TransferProtocol::https;
    }
    if (!scheme.empty())
    {
        return TransferProtocol::invalid;
    }

    return {};
}

/**
 * @brief Function convert transfer protocol from string param.
 *
 */
inline std::optional<TransferProtocol> getTransferProtocolFromParam(
    const std::optional<std::string>& transferProtocolType)
{
    if (transferProtocolType == std::nullopt)
    {
        return {};
    }

    if (*transferProtocolType == "CIFS")
    {
        return TransferProtocol::smb;
    }

    if (*transferProtocolType == "HTTPS")
    {
        return TransferProtocol::https;
    }

    return TransferProtocol::invalid;
}

/**
 * @brief Function extends URI with transfer protocol type.
 *
 */
inline std::string
    getUriWithTransferProtocol(const std::string& imageUri,
                               const TransferProtocol& transferProtocol)
{
    if (transferProtocol == TransferProtocol::smb)
    {
        return "smb://" + imageUri;
    }

    if (transferProtocol == TransferProtocol::https)
    {
        return "https://" + imageUri;
    }

    return imageUri;
}

struct InsertMediaActionParams
{
    std::string imageUrl;
    std::optional<std::string> userName;
    std::optional<std::string> password;
    std::optional<std::string> transferMethod;
    std::optional<std::string> transferProtocolType;
    std::optional<bool> writeProtected = true;
    std::optional<bool> inserted;
};

/**
 * @brief Function validate parameters of insert media request.
 *
 */
inline bool validateParams(crow::Response& res,
                           InsertMediaActionParams& actionParams)
{
    BMCWEB_LOG_DEBUG << "Validation started";
    // required param imageUrl must not be empty
    if (actionParams.imageUrl.empty())
    {
        BMCWEB_LOG_ERROR << "Request action parameter Image is empty.";

        messages::propertyValueFormatError(res, "<empty>", "Image");

        return false;
    }

    // optional param inserted must be true
    if ((actionParams.inserted != std::nullopt) && !*actionParams.inserted)
    {
        BMCWEB_LOG_ERROR
            << "Request action optional parameter Inserted must be true.";

        messages::actionParameterNotSupported(res, "Inserted", "InsertMedia");

        return false;
    }

    // optional param transferMethod must be stream
    if ((actionParams.transferMethod != std::nullopt) &&
        (*actionParams.transferMethod != "Stream"))
    {
        BMCWEB_LOG_ERROR << "Request action optional parameter "
                            "TransferMethod must be Stream.";

        messages::actionParameterNotSupported(res, "TransferMethod",
                                              "InsertMedia");

        return false;
    }
    boost::urls::result<boost::urls::url_view> url =
        boost::urls::parse_uri(boost::string_view(actionParams.imageUrl));
    if (!url)
    {
        messages::resourceAtUriInUnknownFormat(res, *url);
        return {};
    }
    std::optional<TransferProtocol> uriTransferProtocolType =
        getTransferProtocolFromUri(*url);

    std::optional<TransferProtocol> paramTransferProtocolType =
        getTransferProtocolFromParam(actionParams.transferProtocolType);

    // ImageUrl does not contain valid protocol type
    if (*uriTransferProtocolType == TransferProtocol::invalid)
    {
        BMCWEB_LOG_ERROR << "Request action parameter ImageUrl must "
                            "contain specified protocol type from list: "
                            "(smb, https).";

        messages::resourceAtUriInUnknownFormat(res, *url);

        return false;
    }

    // transferProtocolType should contain value from list
    if (*paramTransferProtocolType == TransferProtocol::invalid)
    {
        BMCWEB_LOG_ERROR << "Request action parameter TransferProtocolType "
                            "must be provided with value from list: "
                            "(CIFS, HTTPS).";

        messages::propertyValueNotInList(
            res, *actionParams.transferProtocolType, "TransferProtocolType");
        return false;
    }

    // valid transfer protocol not provided either with URI nor param
    if ((uriTransferProtocolType == std::nullopt) &&
        (paramTransferProtocolType == std::nullopt))
    {
        BMCWEB_LOG_ERROR << "Request action parameter ImageUrl must "
                            "contain specified protocol type or param "
                            "TransferProtocolType must be provided.";

        messages::resourceAtUriInUnknownFormat(res, *url);

        return false;
    }

    // valid transfer protocol provided both with URI and param
    if ((paramTransferProtocolType != std::nullopt) &&
        (uriTransferProtocolType != std::nullopt))
    {
        // check if protocol is the same for URI and param
        if (*paramTransferProtocolType != *uriTransferProtocolType)
        {
            BMCWEB_LOG_ERROR << "Request action parameter "
                                "TransferProtocolType must  contain the "
                                "same protocol type as protocol type "
                                "provided with param imageUrl.";

            messages::actionParameterValueTypeError(
                res, *actionParams.transferProtocolType, "TransferProtocolType",
                "InsertMedia");

            return false;
        }
    }

    // validation passed add protocol to URI if needed
    if (uriTransferProtocolType == std::nullopt)
    {
        actionParams.imageUrl = getUriWithTransferProtocol(
            actionParams.imageUrl, *paramTransferProtocolType);
    }

    return true;
}

template <typename T>
static void secureCleanup(T& value)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    auto raw = const_cast<typename T::value_type*>(value.data());
    explicit_bzero(raw, value.size() * sizeof(*raw));
}

class Credentials
{
  public:
    Credentials(std::string&& user, std::string&& password) :
        userBuf(std::move(user)), passBuf(std::move(password))
    {}

    ~Credentials()
    {
        secureCleanup(userBuf);
        secureCleanup(passBuf);
    }

    const std::string& user()
    {
        return userBuf;
    }

    const std::string& password()
    {
        return passBuf;
    }

    Credentials() = delete;
    Credentials(const Credentials&) = delete;
    Credentials& operator=(const Credentials&) = delete;
    Credentials(Credentials&&) = delete;
    Credentials& operator=(Credentials&&) = delete;

  private:
    std::string userBuf;
    std::string passBuf;
};

class CredentialsProvider
{
  public:
    template <typename T>
    struct Deleter
    {
        void operator()(T* buff) const
        {
            if (buff)
            {
                secureCleanup(*buff);
                delete buff;
            }
        }
    };

    using Buffer = std::vector<char>;
    using SecureBuffer = std::unique_ptr<Buffer, Deleter<Buffer>>;
    // Using explicit definition instead of std::function to avoid implicit
    // conversions eg. stack copy instead of reference
    using FormatterFunc = void(const std::string& username,
                               const std::string& password, Buffer& dest);

    CredentialsProvider(std::string&& user, std::string&& password) :
        credentials(std::move(user), std::move(password))
    {}

    const std::string& user()
    {
        return credentials.user();
    }

    const std::string& password()
    {
        return credentials.password();
    }

    SecureBuffer pack(FormatterFunc formatter)
    {
        SecureBuffer packed{new Buffer{}};
        if (formatter != nullptr)
        {
            formatter(credentials.user(), credentials.password(), *packed);
        }

        return packed;
    }

  private:
    Credentials credentials;
};

// Wrapper for boost::async_pipe ensuring proper pipe cleanup
template <typename Buffer>
class Pipe
{
  public:
    using unix_fd = sdbusplus::message::unix_fd;

    Pipe(boost::asio::io_context& io, Buffer&& buffer) :
        impl(io), buffer{std::move(buffer)}
    {}

    ~Pipe()
    {
        // Named pipe needs to be explicitly removed
        impl.close();
    }

    Pipe(const Pipe&) = delete;
    Pipe(Pipe&&) = delete;
    Pipe& operator=(const Pipe&) = delete;
    Pipe& operator=(Pipe&&) = delete;

    unix_fd fd()
    {
        return unix_fd{impl.native_source()};
    }

    template <typename WriteHandler>
    void asyncWrite(WriteHandler&& handler)
    {
        impl.async_write_some(data(), std::forward<WriteHandler>(handler));
    }

  private:
    // Specialization for pointer types
    template <typename B = Buffer>
    typename std::enable_if<boost::has_dereference<B>::value,
                            boost::asio::const_buffer>::type
        data()
    {
        return boost::asio::buffer(*buffer);
    }

    template <typename B = Buffer>
    typename std::enable_if<!boost::has_dereference<B>::value,
                            boost::asio::const_buffer>::type
        data()
    {
        return boost::asio::buffer(buffer);
    }

    const std::string name;
    boost::process::async_pipe impl;
    Buffer buffer;
};

/**
 * @brief holder for dbus signal matchers
 */
struct MatchWrapper
{
    void stop()
    {
        timer->cancel();
        matcher = std::nullopt;
    }

    std::optional<sdbusplus::bus::match::match> matcher{};
    std::optional<boost::asio::steady_timer> timer;
};

/**
 * @brief Function starts waiting for signal completion
 */
static inline std::shared_ptr<MatchWrapper>
    doListenForCompletion(const std::string& name,
                          const std::string& objectPath,
                          const std::string& action, bool legacy,
                          std::shared_ptr<bmcweb::AsyncResp> asyncResp)
{
    BMCWEB_LOG_DEBUG << "Start Listening for completion : " << action;
    std::string matcherString = sdbusplus::bus::match::rules::type::signal();

    std::string interface =
        std::string("xyz.openbmc_project.VirtualMedia.") + getModeName(legacy);

    matcherString += sdbusplus::bus::match::rules::interface(interface);
    matcherString += sdbusplus::bus::match::rules::member("Completion");
    matcherString += sdbusplus::bus::match::rules::sender(
        "xyz.openbmc_project.VirtualMedia");
    matcherString += sdbusplus::bus::match::rules::path(objectPath);

    auto matchWrapper = std::make_shared<MatchWrapper>();
    auto matchHandler = [asyncResp = std::move(asyncResp), name, action,
                         objectPath,
                         matchWrapper](sdbusplus::message::message& m) {
        int errorCode = 0;
        try
        {
            BMCWEB_LOG_INFO << "Completion signal from " << m.get_path()
                            << " has been received";

            m.read(errorCode);
            switch (errorCode)
            {
                case 0: // success
                    BMCWEB_LOG_INFO << "Signal received: Success";
                    messages::success(asyncResp->res);
                    break;
                case EPERM:
                    BMCWEB_LOG_ERROR << "Signal received: EPERM";
                    messages::accessDenied(
                        asyncResp->res, crow::utility::urlFromPieces(action));
                    break;
                case EBUSY:
                    BMCWEB_LOG_ERROR << "Signal received: EAGAIN";
                    messages::resourceInUse(asyncResp->res);
                    break;
                default:
                    BMCWEB_LOG_ERROR << "Signal received: Other: " << errorCode;
                    messages::operationFailed(asyncResp->res);
                    break;
            };
        }
        catch (sdbusplus::exception::SdBusError& e)
        {
            BMCWEB_LOG_ERROR << e.what();
        };
        // postpone matcher deletion after callback finishes
        boost::asio::post(crow::connections::systemBus->get_io_context(),
                          [name, matchWrapper = matchWrapper]()

                          {
            BMCWEB_LOG_DEBUG << "Removing matcher for " << name << " node.";
            matchWrapper->stop();
        });
    };
    matchWrapper->timer.emplace(crow::connections::systemBus->get_io_context());

    // Safety valve. Clean itself after 3 minutes without signal
    matchWrapper->timer->expires_after(std::chrono::minutes(3));
    matchWrapper->timer->async_wait(
        [matchWrapper](const boost::system::error_code& ec) {
        if (ec != boost::asio::error::operation_aborted)
        {
            BMCWEB_LOG_DEBUG << "Timer expired! Signal did not come";
            matchWrapper->matcher = std::nullopt;
            return;
        }
    });

    matchWrapper->matcher.emplace(*crow::connections::systemBus, matcherString,
                                  matchHandler);
    return matchWrapper;
}

/**
 * @brief Function transceives data with dbus directly.
 *
 * All BMC state properties will be retrieved before sending reset request.
 */
inline void doMountVmLegacy(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                            const std::string& service, const std::string& name,
                            const std::string& imageUrl, const bool rw,
                            std::string& userName, std::string& password)
{
    using SecurePipe = Pipe<CredentialsProvider::SecureBuffer>;
    constexpr const size_t secretLimit = 1024;

    std::shared_ptr<SecurePipe> secretPipe;
    dbus::utility::DbusVariantType unixFd = -1;

    if (!userName.empty() || !password.empty())
    {
        // Encapsulate in safe buffer
        CredentialsProvider credentials(std::move(userName),
                                        std::move(password));

        // Payload must contain data + NULL delimiters
        if (credentials.user().size() + credentials.password().size() + 2 >
            secretLimit)
        {
            BMCWEB_LOG_ERROR << "Credentials too long to handle";
            messages::unrecognizedRequestBody(asyncResp->res);
            return;
        }

        // Pack secret
        auto secret = credentials.pack(
            [](const auto& user, const auto& pass, auto& buff) {
            std::copy(user.begin(), user.end(), std::back_inserter(buff));
            buff.push_back('\0');
            std::copy(pass.begin(), pass.end(), std::back_inserter(buff));
            buff.push_back('\0');
        });

        // Open pipe
        secretPipe = std::make_shared<SecurePipe>(
            crow::connections::systemBus->get_io_context(), std::move(secret));
        unixFd = secretPipe->fd();

        // Pass secret over pipe
        secretPipe->asyncWrite(
            [asyncResp](const boost::system::error_code& ec, std::size_t) {
            if (ec)
            {
                BMCWEB_LOG_ERROR << "Failed to pass secret: " << ec;
                messages::internalError(asyncResp->res);
            }
        });
    }

    const std::string objectPath =
        "/xyz/openbmc_project/VirtualMedia/Legacy/" + name;
    const std::string action = "VirtualMedia.Insert";
    auto wrapper =
        doListenForCompletion(name, objectPath, action, true, asyncResp);

    crow::connections::systemBus->async_method_call(
        [asyncResp, secretPipe, name, action, wrapper,
         objectPath](const boost::system::error_code ec, bool success) {
        if (ec)
        {
            BMCWEB_LOG_ERROR << "Bad D-Bus request error: " << ec;
            if (ec == boost::system::errc::device_or_resource_busy)
            {
                messages::resourceInUse(asyncResp->res);
            }
            else if (ec == boost::system::errc::permission_denied)
            {
                messages::accessDenied(
                    asyncResp->res,
                    crow::utility::urlFromPieces("redfish", "v1", "Managers",
                                                 "bmc", "VirtualMedia", name,
                                                 "Actions", action));
            }
            else
            {
                messages::internalError(asyncResp->res);
            }
            wrapper->stop();
            return;
        }
        if (!success)
        {
            BMCWEB_LOG_ERROR << "Service responded with error ";
            messages::operationFailed(asyncResp->res);
            wrapper->stop();
        }
        },
        service, objectPath, "xyz.openbmc_project.VirtualMedia.Legacy", "Mount",
        imageUrl, rw, unixFd);
}

/**
 * @brief Function transceives data with dbus directly.
 *
 * All BMC state properties will be retrieved before sending reset request.
 */
inline void doEjectAction(const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                          const std::string& service, const std::string& name,
                          bool legacy)
{
    const std::string vmMode = getModeName(legacy);
    const std::string objectPath =
        "/xyz/openbmc_project/VirtualMedia/" + vmMode + "/" + name;
    const std::string ifaceName = "xyz.openbmc_project.VirtualMedia." + vmMode;
    std::string action = "VirtualMedia.Eject";

    auto wrapper =
        doListenForCompletion(name, objectPath, action, legacy, asyncResp);

    crow::connections::systemBus->async_method_call(
        [asyncResp, name, action, objectPath,
         wrapper](const boost::system::error_code ec, bool success) {
        if (ec)
        {
            BMCWEB_LOG_ERROR << "Bad D-Bus request error: " << ec;
            if (ec == boost::system::errc::device_or_resource_busy)
            {
                messages::resourceInUse(asyncResp->res);
            }
            else if (ec == boost::system::errc::permission_denied)
            {
                messages::accessDenied(
                    asyncResp->res,
                    crow::utility::urlFromPieces("redfish", "v1", "Managers",
                                                 "bmc", "VirtualMedia", name,
                                                 "Actions", action));
            }
            else
            {
                messages::internalError(asyncResp->res);
            }
            wrapper->stop();
            return;
        }

        if (!success)
        {
            messages::operationFailed(asyncResp->res);
            wrapper->stop();
        }
        },
        service, objectPath, ifaceName, "Unmount");
}

bool insertMediaCheckMode([[maybe_unused]] const std::string& service,
                          const std::shared_ptr<bmcweb::AsyncResp>& aResp,
                          std::pair<sdbusplus::message::object_path,
                                    dbus::utility::DBusInteracesMap>& item)
{
    auto mode = item.first.parent_path();
    auto type = mode.parent_path();
    // Check if dbus path is Legacy type
    if (mode.filename() == legacyMode)
    {
        BMCWEB_LOG_DEBUG << "InsertMedia only allowed "
                            "with POST method "
                            "in legacy mode";
        aResp->res.result(boost::beast::http::status::method_not_allowed);

        return true;
    }
    // Check if dbus path is Proxy type
    if (mode.filename() == proxyMode)
    {
        BMCWEB_LOG_DEBUG << "InsertMedia not "
                            "allowed in proxy mode";
        aResp->res.result(boost::beast::http::status::not_found);

        return true;
    }
    return false;
}

void vmInsertActionPostHandler(
    crow::App& app, const crow::Request& req,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& name, const std::string& resName)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp->res))
    {
        return;
    }

    std::optional<InsertMediaActionParams> actionParams =
        InsertMediaActionParams();

    // Read obligatory parameters (url of image)
    if (!json_util::readJsonAction(
            req, asyncResp->res, "Image", actionParams->imageUrl,
            "WriteProtected", actionParams->writeProtected, "UserName",
            actionParams->userName, "Password", actionParams->password,
            "Inserted", actionParams->inserted, "TransferMethod",
            actionParams->transferMethod, "TransferProtocolType",
            actionParams->transferProtocolType))
    {
        BMCWEB_LOG_DEBUG << "FAIL: Image: " << actionParams->imageUrl;

        actionParams = std::nullopt;
    }

    // handle legacy mode (parse parameters and start action) for proxy mode
    // return 404.
    auto handler =
        [actionParams,
         resName](const std::string& service,
                  const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                  std::pair<sdbusplus::message::object_path,
                            dbus::utility::DBusInteracesMap>& item) mutable {
        auto mode = parseObjectPathAndGetMode(item.first, resName);

        if (*mode == proxyMode)
        {
            // Not possible in proxy mode
            BMCWEB_LOG_DEBUG << "InsertMedia not "
                                "allowed in proxy mode";
            messages::resourceNotFound(asyncResp->res,
                                       "VirtualMedia.InsertMedia", resName);

            return true;
        }

        if (!actionParams)
        {
            BMCWEB_LOG_DEBUG << "ActionParams can't be empty for legacy mode";
            return true;
        }

        if (!validateParams(asyncResp->res, *actionParams))
        {
            return true;
        }

        // manager is irrelevant for VirtualMedia dbus calls
        doMountVmLegacy(asyncResp, service, resName, actionParams->imageUrl,
                        !(*actionParams->writeProtected),
                        *actionParams->userName, *actionParams->password);

        return true;
    };
    findItemAndRunHandler(asyncResp, name, resName, std::move(handler));
}

void vmEjectActionPostHandler(
    crow::App& app, const crow::Request& req,
    const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
    const std::string& name, const std::string& resName)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp->res))
    {
        return;
    }
    if (name != "bmc")
    {
        messages::resourceNotFound(asyncResp->res, "VirtualMedia.Eject",
                                   resName);

        return;
    }

    crow::connections::systemBus->async_method_call(
        [asyncResp,
         resName](const boost::system::error_code ec,
                  const dbus::utility::MapperGetObject& getObjectType) {
        if (ec)
        {
            BMCWEB_LOG_ERROR << "ObjectMapper::GetObject call failed: " << ec;
            messages::internalError(asyncResp->res);

            return;
        }
        std::string service = getObjectType.begin()->first;
        BMCWEB_LOG_DEBUG << "GetObjectType: " << service;

        crow::connections::systemBus->async_method_call(
            [resName, service,
             asyncResp{asyncResp}](const boost::system::error_code ec,
                                   dbus::utility::ManagedObjectType& subtree) {
            if (ec)
            {
                BMCWEB_LOG_DEBUG << "DBUS response error";

                return;
            }

            for (const auto& object : subtree)
            {
                auto mode = parseObjectPathAndGetMode(object.first, resName);
                if (mode)
                {
                    doEjectAction(asyncResp, service, resName,
                                  *mode == legacyMode);

                    return;
                }
            }
            BMCWEB_LOG_DEBUG << "Parent item not found";
            messages::resourceNotFound(asyncResp->res, "VirtualMedia", resName);
            },
            service, "/xyz/openbmc_project/VirtualMedia",
            "org.freedesktop.DBus.ObjectManager", "GetManagedObjects");
        },
        "xyz.openbmc_project.ObjectMapper",
        "/xyz/openbmc_project/object_mapper",
        "xyz.openbmc_project.ObjectMapper", "GetObject",
        "/xyz/openbmc_project/VirtualMedia", std::array<const char*, 0>());
};

void vmCollectionGetHandler(crow::App& app, const crow::Request& req,
                            const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                            const std::string& name)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp->res))
    {
        return;
    }
    if (name != "bmc")
    {
        messages::resourceNotFound(asyncResp->res, "VirtualMedia", name);

        return;
    }

    asyncResp->res.jsonValue["@odata.type"] =
        "#VirtualMediaCollection.VirtualMediaCollection";
    asyncResp->res.jsonValue["Name"] = "Virtual Media Services";
    asyncResp->res.jsonValue["@odata.id"] =
        "/redfish/v1/Managers/" + name + "/VirtualMedia";

    crow::connections::systemBus->async_method_call(
        [asyncResp, name](const boost::system::error_code ec,
                          const dbus::utility::MapperGetObject& getObjectType) {
        if (ec)
        {
            BMCWEB_LOG_ERROR << "ObjectMapper::GetObject call failed: " << ec;
            messages::internalError(asyncResp->res);

            return;
        }
        std::string service = getObjectType.begin()->first;
        BMCWEB_LOG_DEBUG << "GetObjectType: " << service;

        getVmResourceList(asyncResp, service, name);
        },
        "xyz.openbmc_project.ObjectMapper",
        "/xyz/openbmc_project/object_mapper",
        "xyz.openbmc_project.ObjectMapper", "GetObject",
        "/xyz/openbmc_project/VirtualMedia", std::array<const char*, 0>());
}

void vmItemGetHandler(crow::App& app, const crow::Request& req,
                      const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
                      const std::string& name, const std::string& resName)
{
    if (!redfish::setUpRedfishRoute(app, req, asyncResp->res))
    {
        return;
    }
    if (name != "bmc")
    {
        messages::resourceNotFound(asyncResp->res, "VirtualMedia", resName);

        return;
    }

    crow::connections::systemBus->async_method_call(
        [asyncResp, name,
         resName](const boost::system::error_code ec,
                  const dbus::utility::MapperGetObject& getObjectType) {
        if (ec)
        {
            BMCWEB_LOG_ERROR << "ObjectMapper::GetObject call failed: " << ec;
            messages::internalError(asyncResp->res);

            return;
        }
        std::string service = getObjectType.begin()->first;
        BMCWEB_LOG_DEBUG << "GetObjectType: " << service;

        getVmData(asyncResp, service, name, resName);
        },
        "xyz.openbmc_project.ObjectMapper",
        "/xyz/openbmc_project/object_mapper",
        "xyz.openbmc_project.ObjectMapper", "GetObject",
        "/xyz/openbmc_project/VirtualMedia", std::array<const char*, 0>());
}

inline void requestNBDVirtualMediaRoutes(App& app)
{
    BMCWEB_ROUTE(app, "/redfish/v1/Managers/<str>/VirtualMedia/<str>/Actions/"
                      "VirtualMedia.InsertMedia")
        .privileges(redfish::privileges::getVirtualMedia)
        .methods(boost::beast::http::verb::get)(
            []([[maybe_unused]] const crow::Request& req,
               const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
               const std::string& name, const std::string& resName) {
        findItemAndRunHandler(asyncResp, name, resName, insertMediaCheckMode);
        });

    BMCWEB_ROUTE(app, "/redfish/v1/Managers/<str>/VirtualMedia/<str>/Actions/"
                      "VirtualMedia.InsertMedia")
        .privileges(redfish::privileges::patchVirtualMedia)
        .methods(boost::beast::http::verb::patch)(
            []([[maybe_unused]] const crow::Request& req,
               const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
               const std::string& name, const std::string& resName) {
        findItemAndRunHandler(asyncResp, name, resName, insertMediaCheckMode);
        });
    BMCWEB_ROUTE(app, "/redfish/v1/Managers/<str>/VirtualMedia/<str>/Actions/"
                      "VirtualMedia.InsertMedia")
        .privileges(redfish::privileges::putVirtualMedia)
        .methods(boost::beast::http::verb::put)(
            []([[maybe_unused]] const crow::Request& req,
               const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
               const std::string& name, const std::string& resName) {
        findItemAndRunHandler(asyncResp, name, resName, insertMediaCheckMode);
        });
    BMCWEB_ROUTE(app, "/redfish/v1/Managers/<str>/VirtualMedia/<str>/Actions/"
                      "VirtualMedia.InsertMedia")
        .privileges(redfish::privileges::deleteVirtualMedia)
        .methods(boost::beast::http::verb::delete_)(
            []([[maybe_unused]] const crow::Request& req,
               const std::shared_ptr<bmcweb::AsyncResp>& asyncResp,
               const std::string& name, const std::string& resName) {
        findItemAndRunHandler(asyncResp, name, resName, insertMediaCheckMode);
        });
    BMCWEB_ROUTE(
        app,
        "/redfish/v1/Managers/<str>/VirtualMedia/<str>/Actions/VirtualMedia.InsertMedia")
        .privileges(redfish::privileges::postVirtualMedia)
        .methods(boost::beast::http::verb::post)(
            std::bind_front(vmInsertActionPostHandler, std::ref(app)));

    BMCWEB_ROUTE(
        app,
        "/redfish/v1/Managers/<str>/VirtualMedia/<str>/Actions/VirtualMedia.EjectMedia")
        .privileges(redfish::privileges::postVirtualMedia)
        .methods(boost::beast::http::verb::post)(
            std::bind_front(vmEjectActionPostHandler, std::ref(app)));
    BMCWEB_ROUTE(app, "/redfish/v1/Managers/<str>/VirtualMedia/")
        .privileges(redfish::privileges::getVirtualMediaCollection)
        .methods(boost::beast::http::verb::get)(
            std::bind_front(vmCollectionGetHandler, std::ref(app)));
    BMCWEB_ROUTE(app, "/redfish/v1/Managers/<str>/VirtualMedia/<str>/")
        .privileges(redfish::privileges::getVirtualMedia)
        .methods(boost::beast::http::verb::get)(
            std::bind_front(vmItemGetHandler, std::ref(app)));
}

} // namespace redfish
