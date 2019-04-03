#pragma once

#include <boost/container/flat_map.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <dbus_singleton.hpp>
#include <nlohmann/json.hpp>
#include <pam_authenticate.hpp>
#include <random>

#include "crow/logging.h"

namespace crow
{

namespace persistent_data
{

enum class PersistenceType
{
    TIMEOUT, // User session times out after a predetermined amount of time
    SINGLE_REQUEST // User times out once this request is completed.
};

constexpr auto userService = "xyz.openbmc_project.User.Manager";
constexpr auto userObjPath = "/xyz/openbmc_project/user";
constexpr auto userAttrIface = "xyz.openbmc_project.User.Attributes";

using GetManagedPropertyType =
    boost::container::flat_map<std::string, std::variant<std::string, bool>>;

using InterfacesPropertiesType =
    boost::container::flat_map<std::string, GetManagedPropertyType>;

using GetManagedObjectsType =
    boost::container::flat_map<sdbusplus::message::object_path,
                               InterfacesPropertiesType>;

struct UserRoleMap
{
    static UserRoleMap& getInstance()
    {
        static UserRoleMap userRoleMap;
        return userRoleMap;
    }

    UserRoleMap(const UserRoleMap&) = delete;
    UserRoleMap& operator=(const UserRoleMap&) = delete;

    std::string getUserRole(const std::string& name)
    {
        auto it = roleMap.find(name);
        if (it == roleMap.end())
        {
            return {};
        }
        return roleMap[name];
    }

    std::string
        getUserRole(const InterfacesPropertiesType& interfacesProperties)
    {
        auto iface = interfacesProperties.find(userAttrIface);
        if (iface == interfacesProperties.end())
        {
            return {};
        }

        auto& properties = iface->second;
        auto property = properties.find("UserPrivilege");
        if (property == properties.end())
        {
            return {};
        }

        auto role = std::get_if<std::string>(&property->second);
        if (role == nullptr)
        {
            BMCWEB_LOG_ERROR << "UserPrivilege property value is null";
        }

        return *role;
    }

    ~UserRoleMap()
    {
        userAddedSignal = nullptr;
        userRemovedSignal = nullptr;

        // Destroy propertiesChanged signal handler installed for
        // all the users.
        userPropertiesChangedSignal.clear();
    }

  private:
    void userAdded(sdbusplus::message::message& m)
    {
        sdbusplus::message::object_path objPath;
        InterfacesPropertiesType interfacesProperties;

        try
        {
            m.read(objPath, interfacesProperties);
        }
        catch (const sdbusplus::exception::SdBusError& e)
        {
            BMCWEB_LOG_ERROR << "Failed to parse user add signal."
                             << "ERROR=" << e.what()
                             << "REPLY_SIG=" << m.get_signature();
            return;
        }
        BMCWEB_LOG_DEBUG << "obj path = " << objPath.str;

        std::size_t lastPos = objPath.str.rfind("/");
        if (lastPos == std::string::npos)
        {
            return;
        };

        std::string&& name = objPath.str.substr(lastPos + 1);
        std::string&& role = this->getUserRole(interfacesProperties);

        // Insert the newly added user name and the role
        auto res = roleMap.emplace(name, role);
        if (res.second == false)
        {
            BMCWEB_LOG_ERROR << "Insertion of the user=\"" << name
                             << "\" in the roleMap failed.";
            return;
        }

        // Install the properties changed signal for the
        // newly added user.

        userPropertiesChangedSignal.emplace(
            name, std::make_unique<sdbusplus::bus::match_t>(
                      *crow::connections::systemBus,
                      sdbusplus::bus::match::rules::propertiesChanged(
                          objPath.str, userAttrIface),
                      [this](sdbusplus::message::message& m) {
                          BMCWEB_LOG_DEBUG << "Properties Changed";
                          this->userPropertiesChanged(m);
                      }));
    }

    void userRemoved(sdbusplus::message::message& m)
    {
        sdbusplus::message::object_path objPath;

        try
        {
            m.read(objPath);
        }
        catch (const sdbusplus::exception::SdBusError& e)
        {
            BMCWEB_LOG_ERROR << "Failed to parse user delete signal.";
            BMCWEB_LOG_ERROR << "ERROR=" << e.what()
                             << "REPLY_SIG=" << m.get_signature();
            return;
        }

        BMCWEB_LOG_DEBUG << "obj path = " << objPath.str;

        std::size_t lastPos = objPath.str.rfind("/");
        if (lastPos == std::string::npos)
        {
            return;
        };
        std::string&& name = objPath.str.substr(lastPos + 1);

        // Remove the user from the role map
        roleMap.erase(name);

        // Remove the properties changed signal for this user

        auto it = userPropertiesChangedSignal.find(name);
        if (it == userPropertiesChangedSignal.end())
        {
            return;
        }
        userPropertiesChangedSignal.erase(name);
    }

    void userPropertiesChanged(sdbusplus::message::message& m)
    {
        std::string interface;
        GetManagedPropertyType changedProperties;
        m.read(interface, changedProperties);
        const std::string& path = m.get_path();

        BMCWEB_LOG_DEBUG << "Object Path = \"" << path << "\"";

        std::size_t lastPos = path.rfind("/");
        if (lastPos == std::string::npos)
        {
            return;
        };

        std::string&& user = path.substr(lastPos + 1);

        BMCWEB_LOG_DEBUG << "User Name = \"" << user << "\"";

        // Has the UserPrivilege property modified ?
        auto index = changedProperties.find("UserPrivilege");
        if (index == changedProperties.end())
        {
            return;
        }

        auto role = std::get_if<std::string>(&index->second);
        if (role == nullptr)
        {
            return;
        }
        BMCWEB_LOG_DEBUG << "Role = \"" << *role << "\"";

        auto it = roleMap.find(user);
        if (it == roleMap.end())
        {
            BMCWEB_LOG_ERROR << "User Name = \"" << user
                             << "\" is not found. But, received "
                                "propertiesChanged signal";
            return;
        }
        roleMap[user] = *role;
    }

    UserRoleMap()
    {
        auto method = crow::connections::systemBus->new_method_call(
            userService, userObjPath, "org.freedesktop.DBus.ObjectManager",
            "GetManagedObjects");

        auto reply = crow::connections::systemBus->call(method);
        GetManagedObjectsType managedObjects;
        reply.read(managedObjects);
        for (auto& managedObj : managedObjects)
        {
            std::size_t lastPos = managedObj.first.str.rfind("/");
            if (lastPos == std::string::npos)
            {
                continue;
            };

            std::string&& name = managedObj.first.str.substr(lastPos + 1);
            std::string&& role = getUserRole(managedObj.second);

            roleMap.emplace(name, role);

            // Add properties changed signal for this user
            std::string& userPath = managedObj.first.str;

            userPropertiesChangedSignal.emplace(
                name, std::make_unique<sdbusplus::bus::match_t>(
                          *crow::connections::systemBus,
                          sdbusplus::bus::match::rules::propertiesChanged(
                              userPath, userAttrIface),
                          [this](sdbusplus::message::message& m) {
                              BMCWEB_LOG_DEBUG << "Properties Changed";
                              this->userPropertiesChanged(m);
                          }));
        }

        userAddedSignal = std::make_unique<sdbusplus::bus::match_t>(
            *crow::connections::systemBus,
            sdbusplus::bus::match::rules::interfacesAdded(userObjPath),
            [this](sdbusplus::message::message& m) {
                BMCWEB_LOG_DEBUG << "User Added";
                this->userAdded(m);
            });

        userRemovedSignal = std::make_unique<sdbusplus::bus::match_t>(
            *crow::connections::systemBus,
            sdbusplus::bus::match::rules::interfacesRemoved(userObjPath),
            [this](sdbusplus::message::message& m) {
                BMCWEB_LOG_DEBUG << "User Removed";
                this->userRemoved(m);
            });
    }

    std::map<std::string, std::string> roleMap;
    std::unique_ptr<sdbusplus::bus::match_t> userAddedSignal;
    std::unique_ptr<sdbusplus::bus::match_t> userRemovedSignal;
    std::map<std::string, std::unique_ptr<sdbusplus::bus::match_t>>
        userPropertiesChangedSignal;
};

struct UserSession
{
    std::string uniqueId;
    std::string sessionToken;
    std::string username;
    std::string userRole;
    std::string csrfToken;
    std::chrono::time_point<std::chrono::steady_clock> lastUpdated;
    PersistenceType persistence;

    /**
     * @brief Fills object with data from UserSession's JSON representation
     *
     * This replaces nlohmann's from_json to ensure no-throw approach
     *
     * @param[in] j   JSON object from which data should be loaded
     *
     * @return a shared pointer if data has been loaded properly, nullptr
     * otherwise
     */
    static std::shared_ptr<UserSession> fromJson(const nlohmann::json& j)
    {
        std::shared_ptr<UserSession> userSession =
            std::make_shared<UserSession>();
        for (const auto& element : j.items())
        {
            const std::string* thisValue =
                element.value().get_ptr<const std::string*>();
            if (thisValue == nullptr)
            {
                BMCWEB_LOG_ERROR << "Error reading persistent store.  Property "
                                 << element.key() << " was not of type string";
                return nullptr;
            }
            if (element.key() == "unique_id")
            {
                userSession->uniqueId = *thisValue;
            }
            else if (element.key() == "session_token")
            {
                userSession->sessionToken = *thisValue;
            }
            else if (element.key() == "csrf_token")
            {
                userSession->csrfToken = *thisValue;
            }
            else if (element.key() == "username")
            {
                userSession->username = *thisValue;
            }
            else if (element.key() == "user_role")
            {
                userSession->userRole = *thisValue;
            }
            else
            {
                BMCWEB_LOG_ERROR
                    << "Got unexpected property reading persistent file: "
                    << element.key();
                return nullptr;
            }
        }

        // For now, sessions that were persisted through a reboot get their idle
        // timer reset.  This could probably be overcome with a better
        // understanding of wall clock time and steady timer time, possibly
        // persisting values with wall clock time instead of steady timer, but
        // the tradeoffs of all the corner cases involved are non-trivial, so
        // this is done temporarily
        userSession->lastUpdated = std::chrono::steady_clock::now();
        userSession->persistence = PersistenceType::TIMEOUT;

        return userSession;
    }
};

class Middleware;

class SessionStore
{
  public:
    std::shared_ptr<UserSession> generateUserSession(
        const std::string_view username,
        PersistenceType persistence = PersistenceType::TIMEOUT)
    {
        // TODO(ed) find a secure way to not generate session identifiers if
        // persistence is set to SINGLE_REQUEST
        static constexpr std::array<char, 62> alphanum = {
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'b', 'C',
            'D', 'E', 'F', 'g', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
            'Q', 'r', 'S', 'T', 'U', 'v', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c',
            'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
            'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};

        // entropy: 30 characters, 62 possibilities.  log2(62^30) = 178 bits of
        // entropy.  OWASP recommends at least 60
        // https://www.owasp.org/index.php/Session_Management_Cheat_Sheet#Session_ID_Entropy
        std::string sessionToken;
        sessionToken.resize(20, '0');
        std::uniform_int_distribution<int> dist(0, alphanum.size() - 1);
        for (int i = 0; i < sessionToken.size(); ++i)
        {
            sessionToken[i] = alphanum[dist(rd)];
        }
        // Only need csrf tokens for cookie based auth, token doesn't matter
        std::string csrfToken;
        csrfToken.resize(20, '0');
        for (int i = 0; i < csrfToken.size(); ++i)
        {
            csrfToken[i] = alphanum[dist(rd)];
        }

        std::string uniqueId;
        uniqueId.resize(10, '0');
        for (int i = 0; i < uniqueId.size(); ++i)
        {
            uniqueId[i] = alphanum[dist(rd)];
        }

        // Get the User Privilege
        const std::string& role =
            UserRoleMap::getInstance().getUserRole(std::string(username));

        BMCWEB_LOG_DEBUG << "user name=\"" << username << "\" role = " << role;
        auto session = std::make_shared<UserSession>(UserSession{
            uniqueId, sessionToken, std::string(username), role, csrfToken,
            std::chrono::steady_clock::now(), persistence});
        auto it = authTokens.emplace(std::make_pair(sessionToken, session));
        // Only need to write to disk if session isn't about to be destroyed.
        needWrite = persistence == PersistenceType::TIMEOUT;
        return it.first->second;
    }

    std::shared_ptr<UserSession>
        loginSessionByToken(const std::string_view token)
    {
        applySessionTimeouts();
        auto sessionIt = authTokens.find(std::string(token));
        if (sessionIt == authTokens.end())
        {
            return nullptr;
        }
        std::shared_ptr<UserSession> userSession = sessionIt->second;
        userSession->lastUpdated = std::chrono::steady_clock::now();
        return userSession;
    }

    std::shared_ptr<UserSession> getSessionByUid(const std::string_view uid)
    {
        applySessionTimeouts();
        // TODO(Ed) this is inefficient
        auto sessionIt = authTokens.begin();
        while (sessionIt != authTokens.end())
        {
            if (sessionIt->second->uniqueId == uid)
            {
                return sessionIt->second;
            }
            sessionIt++;
        }
        return nullptr;
    }

    void removeSession(std::shared_ptr<UserSession> session)
    {
        authTokens.erase(session->sessionToken);
        needWrite = true;
    }

    std::vector<const std::string*> getUniqueIds(
        bool getAll = true,
        const PersistenceType& type = PersistenceType::SINGLE_REQUEST)
    {
        applySessionTimeouts();

        std::vector<const std::string*> ret;
        ret.reserve(authTokens.size());
        for (auto& session : authTokens)
        {
            if (getAll || type == session.second->persistence)
            {
                ret.push_back(&session.second->uniqueId);
            }
        }
        return ret;
    }

    bool needsWrite()
    {
        return needWrite;
    }
    int getTimeoutInSeconds() const
    {
        return std::chrono::seconds(timeoutInMinutes).count();
    };

    // Persistent data middleware needs to be able to serialize our authTokens
    // structure, which is private
    friend Middleware;

    static SessionStore& getInstance()
    {
        static SessionStore sessionStore;
        return sessionStore;
    }

    SessionStore(const SessionStore&) = delete;
    SessionStore& operator=(const SessionStore&) = delete;

  private:
    SessionStore() : timeoutInMinutes(60)
    {
    }

    void applySessionTimeouts()
    {
        auto timeNow = std::chrono::steady_clock::now();
        if (timeNow - lastTimeoutUpdate > std::chrono::minutes(1))
        {
            lastTimeoutUpdate = timeNow;
            auto authTokensIt = authTokens.begin();
            while (authTokensIt != authTokens.end())
            {
                if (timeNow - authTokensIt->second->lastUpdated >=
                    timeoutInMinutes)
                {
                    authTokensIt = authTokens.erase(authTokensIt);
                    needWrite = true;
                }
                else
                {
                    authTokensIt++;
                }
            }
        }
    }

    std::string getUserRole(const std::string& username)
    {
        auto method = crow::connections::systemBus->new_method_call(
            "xyz.openbmc_project.User.Manager", "/xyz/openbmc_project/user",
            "xyz.openbmc_project.User.Manager", "GetUserInfo");
        method.append(username);

        auto reply = crow::connections::systemBus->call(method);

        std::map<std::string, sdbusplus::message::variant<
                                  bool, std::string, std::vector<std::string>>>
            userInfo;

        reply.read(userInfo);

        std::string* userRole = nullptr;
        auto userInfoIter = userInfo.find("UserPrivilege");
        if (userInfoIter != userInfo.end())
        {
            userRole = std::get_if<std::string>(&userInfoIter->second);
        }

        if (userRole != nullptr)
        {
            BMCWEB_LOG_DEBUG << "User Role is " << *userRole;
            return *userRole;
        }
        else
        {
            BMCWEB_LOG_ERROR << "Unable to find userRole";
        }
        return " ";
    }

    std::chrono::time_point<std::chrono::steady_clock> lastTimeoutUpdate;
    boost::container::flat_map<std::string, std::shared_ptr<UserSession>>
        authTokens;
    std::random_device rd;
    bool needWrite{false};
    std::chrono::minutes timeoutInMinutes;
};

} // namespace persistent_data
} // namespace crow

// to_json(...) definition for objects of UserSession type
namespace nlohmann
{
template <>
struct adl_serializer<std::shared_ptr<crow::persistent_data::UserSession>>
{
    static void
        to_json(nlohmann::json& j,
                const std::shared_ptr<crow::persistent_data::UserSession>& p)
    {
        if (p->persistence !=
            crow::persistent_data::PersistenceType::SINGLE_REQUEST)
        {
            j = nlohmann::json{{"unique_id", p->uniqueId},
                               {"session_token", p->sessionToken},
                               {"username", p->username},
                               {"csrf_token", p->csrfToken},
                               {"user_role", p->userRole}};
        }
    }
};
} // namespace nlohmann
