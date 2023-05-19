#pragma once
#include "dbus_to_json.hpp"

namespace dbus
{
namespace utility
{
struct DbusBaseHandler
{
    struct HeapVectors
    {
        std::vector<handler_pair> vec;
        ValueExtractor valueExtractor;
        explicit HeapVectors(std::vector<handler_pair>&& v) :
            vec(std::move(v)), valueExtractor(makeExtractor(vec))
        {}
    };
    using handler_map =
        boost::container::flat_map<std::string, std::unique_ptr<HeapVectors>>;
    handler_map handlers;
    auto& get(const std::string& k) const
    {
        return handlers.at(k);
    }
    auto registerdHandlers() const
    {
        return handlers | std::ranges::views::keys;
    }

    void addInterfaceHandlers(const std::string& iFaceName,
                              std::vector<handler_pair>&& iFaceHandlers)
    {
        handlers.emplace(
            iFaceName, std::make_unique<HeapVectors>(std::move(iFaceHandlers)));
    }
    void addInterfaceHandler(const std::string& iFaceName,
                             std::string_view name,
                             std::unique_ptr<BaseNodeMapper> mapper)
    {
        auto iter = handlers.find(iFaceName);

        if (iter != end(handlers))
        {
            iter->second->vec.emplace_back(name, std::move(mapper));
            return;
        }
        std::vector<handler_pair> vec;
        vec.emplace_back(name, std::move(mapper));
        addInterfaceHandlers(iFaceName, std::move(vec));
    }
};

enum class DbusParserStatus
{
    Ok,
    Failed
};
template <typename Extraction_Handlers>
struct DbusTreeParser
{
    using FilterType =
        std::function<bool(const ManagedObjectType::value_type&)>;

    const Extraction_Handlers* handlers{nullptr};

    FilterType filter;
    bool ignoreUnknown{false};
    DbusTreeParser(Extraction_Handlers& h, bool ignUnkwn = false) :
        handlers(&h), ignoreUnknown(ignUnkwn)
    {}
    DbusTreeParser& withFilter(FilterType t)
    {
        filter = std::move(t);
        return *this;
    }

    static auto selectRegisteredInterfaceHandlers(auto& handlers)
    {
        return std::views::filter(
            [&](auto k) { // filter out all unregistered intefaces
            const auto& [infacename, propmap] = k;
            auto rh = handlers->registerdHandlers();
            return std::ranges::find(rh, infacename) != rh.end();
        });
    }
    static auto convertInterfacedataToJson(const auto& path, auto& handlers,
                                           bool ignoreUnknown, auto& metaData)
    {
        return std::ranges::views::transform(
            [path, &handlers, ignoreUnknown,
             &metaData](auto k) { // convert dbus properties to json
            const auto& [ifacename, ifacedata] = k;

            return handlers->get(ifacename)->valueExtractor(
                path, ifacename, ifacedata, ignoreUnknown, metaData);

        });
    }
    static nlohmann::json parseObjectProperties(const auto& path,
                                                const auto& ifaceList,
                                                const auto& handlers,
                                                bool ignoreUnknown,
                                                auto& metaData)
    {
        nlohmann::json ifaceData;
        for (const auto& j :
             ifaceList | selectRegisteredInterfaceHandlers(handlers) |
                 convertInterfacedataToJson(path, handlers, ignoreUnknown,
                                            metaData))
        {
            if (j != nlohmann::json())
            {
                ifaceData.merge_patch(j);
            }
        }
        return ifaceData;
    }
    template <typename CompletionHandler>
        requires(std::invocable<CompletionHandler, DbusParserStatus,
                                const nlohmann::json&>)
    void parse(const DBusPropertiesMap& propMap,
               CompletionHandler&& compHandler)
    {
        MetaData metaData;
        try
        {
            nlohmann::json root;
            for (auto& key : handlers->registerdHandlers())
            {
                auto js = handlers->get(key)->valueExtractor(
                    sdbusplus::message::object_path{}, key, propMap,
                    ignoreUnknown, metaData);
                root.merge_patch(js);
            }
            if (!metaData.res.empty())
            {
                root.merge_patch(metaData.res);
            }
            compHandler(DbusParserStatus::Ok, root);
        }
        catch (...)
        {
            compHandler(DbusParserStatus::Failed, metaData.res);
        }
    }
    template <typename CompletionHandler>
        requires(std::invocable<CompletionHandler, DbusParserStatus,
                                const nlohmann::json&>)
    void parse(const ManagedObjectType& objects,
               CompletionHandler&& compHandler)
    {
        MetaData metaData;
        try
        {
            nlohmann::json root;

            auto paths = objects |
                         std::views::filter( // if filter set for object path
                                             // use it else use identity filter
                             filter ? filter : [](auto&&) { return true; });

            for (const auto& [objectPath, interface_data] : paths)
            {
                root.merge_patch(
                    parseObjectProperties(objectPath, interface_data, handlers,
                                          ignoreUnknown, metaData));
            }
            if (metaData.res != nlohmann::json())
            {
                root.merge_patch(metaData.res);
            }

            compHandler(DbusParserStatus::Ok, root);
        }
        catch (...)
        {
            compHandler(DbusParserStatus::Failed, metaData.res);
        }
    }
};
template <typename Handler>
DbusTreeParser(Handler&, bool) -> DbusTreeParser<Handler>;
} // namespace utility

} // namespace dbus
