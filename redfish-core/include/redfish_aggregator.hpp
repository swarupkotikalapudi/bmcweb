#pragma once

#include <http_client.hpp>

namespace redfish
{

class RedfishAggregator
{
  private:
    crow::HttpClient& client = crow::HttpClient::getInstance();
    std::unique_ptr<boost::asio::deadline_timer> filterTimer;
    boost::asio::io_context& ioc =
        crow::connections::systemBus->get_io_context();

    // Match signals for adding Satellite Config
    std::unique_ptr<sdbusplus::bus::match::match> matchSatelliteSignalMonitor;

    // Maps a chosen alias representing a satellite BMC to a url containing
    // the information required to create a http connection to the satellite
    std::unordered_map<std::string, boost::urls::url> satelliteInfo;

    RedfishAggregator()
    {
        filterTimer = std::make_unique<boost::asio::deadline_timer>(ioc);

        // Setup signal matching first in case a satellite becomes available
        // before we complete manual searching
        registerSatelliteConfigSignal();

        // Search for satellite config information that's available before
        // HttpClient is initialized
        getSatelliteConfigs();
    }

    // Setup a D-Bus match to add the config info for any satellites
    // that are added or changed after bmcweb starts
    void registerSatelliteConfigSignal()
    {
        // This handler will get called per each property created or updated.
        // We want to wait until its final call and then query the D-Bus to get
        // all of the satellite config information
        std::function<void(sdbusplus::message::message&)> eventHandler =
            [this](sdbusplus::message::message& message) {
                if (message.is_method_error())
                {
                    BMCWEB_LOG_ERROR
                        << "registerSatelliteConfigSignal callback method error";
                    return;
                }

                // This implicitly cancels the timer
                filterTimer->expires_from_now(boost::posix_time::seconds(1));

                filterTimer->async_wait([this](const boost::system::error_code&
                                                   ec) {
                    if (ec == boost::asio::error::operation_aborted)
                    {
                        // We were cancelled
                        return;
                    }
                    if (ec)
                    {
                        BMCWEB_LOG_ERROR
                            << "registerSatelliteConfigSignal timer error";
                        return;
                    }
                    // Now manually scan to get all of the new satellite config
                    // information
                    BMCWEB_LOG_DEBUG
                        << "Match received for SatelliteController.  Updating satellite configs";
                    this->getSatelliteConfigs();
                });
            };

        BMCWEB_LOG_DEBUG << "Satellite config signal - Register";
        std::string matchStr =
            "type='signal',member='PropertiesChanged',"
            "interface='org.freedesktop.DBus.Properties',"
            "arg0namespace='xyz.openbmc_project.Configuration.SatelliteController'";
        matchSatelliteSignalMonitor =
            std::make_unique<sdbusplus::bus::match::match>(
                *crow::connections::systemBus, matchStr, eventHandler);
    }

    // Polls D-Bus to get all available satellite config information
    void getSatelliteConfigs()
    {
        BMCWEB_LOG_DEBUG << "Gathering satellite configs";
        crow::connections::systemBus->async_method_call(
            [this](const boost::system::error_code ec,
                   const dbus::utility::ManagedObjectType& objects) {
                if (ec)
                {
                    BMCWEB_LOG_ERROR << "DBUS response error " << ec.value()
                                     << ", " << ec.message();
                    return;
                }
                findSatelliteConfigs(objects);
            },
            "xyz.openbmc_project.EntityManager", "/",
            "org.freedesktop.DBus.ObjectManager", "GetManagedObjects");
    }

    // Search D-Bus objects for satellite config objects and add their
    // information if valid
    void findSatelliteConfigs(const dbus::utility::ManagedObjectType& objects)
    {
        for (const auto& objectPath : objects)
        {
            for (const auto& interface : objectPath.second)
            {
                if (interface.first ==
                    "xyz.openbmc_project.Configuration.SatelliteController")
                {
                    BMCWEB_LOG_INFO << "Found Satellite Controller at "
                                    << objectPath.first.str;

                    addSatelliteConfig(interface.second);
                }
            }
        }

        if (!satelliteInfo.empty())
        {
            BMCWEB_LOG_INFO << "Redfish Aggregation enabled with "
                            << std::to_string(satelliteInfo.size())
                            << " satellite BMCs";
        }
        else
        {
            BMCWEB_LOG_INFO
                << "No satellite BMCs detected.  Redfish Aggregation not enabled";
        }
    }

    // Parse the properties of a satellite config object and add the
    // configuration if the properties are valid
    void addSatelliteConfig(const dbus::utility::DBusPropertiesMap& properties)
    {
        boost::urls::url url;
        std::string name;

        for (const auto& prop : properties)
        {
            if (prop.first == "Name")
            {
                const std::string* propVal =
                    std::get_if<std::string>(&prop.second);
                if (propVal == nullptr)
                {
                    BMCWEB_LOG_ERROR << "Invalid Name value";
                    return;
                }

                // The IDs will become <Name>_<ID> so the name should not
                // contain a '_'
                if (propVal->find('_') != std::string::npos)
                {
                    BMCWEB_LOG_ERROR << "Name cannot contain a \"_\"";
                    return;
                }
                name = *propVal;
            }

            else if (prop.first == "Hostname")
            {
                const std::string* propVal =
                    std::get_if<std::string>(&prop.second);
                if (propVal == nullptr)
                {
                    BMCWEB_LOG_ERROR << "Invalid Hostname value";
                    return;
                }
                url.set_host(*propVal);
            }

            else if (prop.first == "Port")
            {
                const uint64_t* propVal = std::get_if<uint64_t>(&prop.second);
                if (propVal == nullptr)
                {
                    BMCWEB_LOG_ERROR << "Invalid Port value";
                    return;
                }

                if ((*propVal > 65535) || (*propVal == 0))
                {
                    BMCWEB_LOG_ERROR << "Port value out of range";
                }
                url.set_port(static_cast<uint16_t>(*propVal));
            }

            else if (prop.first == "AuthType")
            {
                const std::string* propVal =
                    std::get_if<std::string>(&prop.second);
                if (propVal == nullptr)
                {
                    BMCWEB_LOG_ERROR << "Invalid AuthType value";
                    return;
                }

                // For now assume authentication not required to communicate
                // with the satellite BMC
                if (*propVal != "none")
                {
                    BMCWEB_LOG_ERROR
                        << "Unsupported AuthType value: " << *propVal
                        << ", only \"none\" is supported";
                    return;
                }
                url.set_scheme("http");
            }
        } // Finished reading properties

        // Make sure all required config information was made available
        if (name.empty())
        {
            BMCWEB_LOG_ERROR << "Satellite config missing Name";
            return;
        }

        if (url.host().empty())
        {
            BMCWEB_LOG_ERROR << "Satellite config " << name << " missing Host";
            return;
        }

        if (!url.has_port())
        {
            BMCWEB_LOG_ERROR << "Satellite config " << name << " missing Port";
            return;
        }

        if (!url.has_scheme())
        {
            BMCWEB_LOG_ERROR << "Satellite config " << name
                             << " missing AuthType";
            return;
        }

        std::string resultString;
        auto result = satelliteInfo.insert_or_assign(name, std::move(url));
        if (result.second)
        {
            resultString = "Added new satellite config ";
        }
        else
        {
            resultString = "Updated existing satellite config ";
        }

        BMCWEB_LOG_DEBUG << resultString << name << " at "
                         << result.first->second.scheme() << "://"
                         << result.first->second.encoded_host_and_port();
    }

  public:
    RedfishAggregator(const RedfishAggregator&) = delete;
    RedfishAggregator& operator=(const RedfishAggregator&) = delete;
    RedfishAggregator(RedfishAggregator&&) = delete;
    RedfishAggregator& operator=(RedfishAggregator&&) = delete;
    ~RedfishAggregator() = default;

    static RedfishAggregator& getInstance()
    {
        static RedfishAggregator handler;
        return handler;
    }
};

} // namespace redfish
