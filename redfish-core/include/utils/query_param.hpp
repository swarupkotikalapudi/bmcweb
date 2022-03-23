#pragma once
#include "app.hpp"
#include "async_resp.hpp"
#include "error_messages.hpp"
#include "http_request.hpp"
#include "routing.hpp"

#include <charconv>
#include <string_view>
#include <utility>
#include <vector>

namespace redfish
{
namespace query_param
{

enum class ExpandType : uint8_t
{
    None,
    Hyperlinks,
    NotHyperlinks,
    Both,
};

struct Query
{
    bool isOnly = false;
    uint8_t expandLevel = 1;
    ExpandType expandType = ExpandType::None;
};

inline bool getExpandType(std::string_view value, Query& query)
{
    if (value.empty())
    {
        return false;
    }
    switch (value[0])
    {
        case '*':
            query.expandType = ExpandType::Hyperlinks;
            break;
        case '.':
            query.expandType = ExpandType::NotHyperlinks;
            break;
        case '~':
            query.expandType = ExpandType::Both;
            break;
        default:
            return false;

            break;
    }
    value.remove_prefix(1);
    if (value.empty())
    {
        query.expandLevel = 1;
        return true;
    }
    constexpr std::string_view levels = "($levels=";
    if (!value.starts_with(levels))
    {
        return false;
    }
    value.remove_prefix(levels.size());

    auto it = std::from_chars(value.data(), value.data() + value.size(),
                              query.expandLevel);
    if (it.ec != std::errc())
    {
        return false;
    }
    value.remove_prefix(static_cast<size_t>(it.ptr - value.data()));
    return value == ")";
}

inline std::optional<Query>
    parseParameters(const boost::urls::params_view& urlParams,
                    crow::Response& res)
{
    Query ret{};
    for (const boost::urls::params_view::value_type& it : urlParams)
    {
        std::string_view key(it.key.data(), it.key.size());
        std::string_view value(it.value.data(), it.value.size());
        if (key == "only")
        {
            if (!it.value.empty())
            {
                messages::queryParameterValueFormatError(res, value, key);
                return std::nullopt;
            }
            ret.isOnly = true;
        }
        else if (key == "$expand")
        {
            if (!getExpandType(value, ret))
            {
                messages::queryParameterValueFormatError(res, value, key);
                return std::nullopt;
            }
        }
        else
        {
            // Intentionally ignore other errors Redfish spec, 7.3.1
            if (key.starts_with("$"))
            {
                // Services shall return... The HTTP 501 Not Implemented status
                // code for any unsupported query parameters that start with $ .
                messages::queryParameterValueFormatError(res, value, key);
                res.result(boost::beast::http::status::not_implemented);
                return std::nullopt;
            }
            // "Shall ignore unknown or unsupported query parameters that do not
            // begin with $ ."
        }
    }

    return ret;
}

inline bool processOnly(crow::App& app, crow::Response& res,
                        std::function<void(crow::Response&)>& completionHandler)
{
    BMCWEB_LOG_DEBUG << "Processing only query param";
    auto itMembers = res.jsonValue.find("Members");
    if (itMembers == res.jsonValue.end())
    {
        messages::queryNotSupportedOnResource(res);
        completionHandler(res);
        return false;
    }
    auto itMemBegin = itMembers->begin();
    if (itMemBegin == itMembers->end() || itMembers->size() != 1)
    {
        BMCWEB_LOG_DEBUG << "Members contains " << itMembers->size()
                         << " element, returning full collection.";
        completionHandler(res);
        return false;
    }

    auto itUrl = itMemBegin->find("@odata.id");
    if (itUrl == itMemBegin->end())
    {
        BMCWEB_LOG_DEBUG << "No found odata.id";
        messages::internalError(res);
        completionHandler(res);
        return false;
    }
    const std::string* url = itUrl->get_ptr<const std::string*>();
    if (url == nullptr)
    {
        BMCWEB_LOG_DEBUG << "@odata.id wasn't a string????";
        messages::internalError(res);
        completionHandler(res);
        return false;
    }
    // TODO(Ed) copy request headers?
    // newReq.session = req.session;
    std::error_code ec;
    crow::Request newReq({boost::beast::http::verb::get, *url, 11}, ec);
    if (ec)
    {
        messages::internalError(res);
        completionHandler(res);
        return false;
    }

    auto asyncResp = std::make_shared<bmcweb::AsyncResp>();
    BMCWEB_LOG_DEBUG << "setting completion handler on " << &asyncResp->res;
    asyncResp->res.setCompleteRequestHandler(std::move(completionHandler));
    asyncResp->res.setIsAliveHelper(res.releaseIsAliveHelper());
    app.handle(newReq, asyncResp);
    return true;
}

struct ExpandNode
{
    nlohmann::json::json_pointer location;
    std::string uri;

    inline bool operator==(const ExpandNode& other) const
    {
        return location == other.location && uri == other.uri;
    }
};

// Walks a json object looking for Redfish NavigationReference entries that
// might need resolved
inline void
    findNavigationReferencesRecursive(nlohmann::json& jsonResponse,
                                      const nlohmann::json::json_pointer& p,
                                      std::vector<ExpandNode>& out)
{
    nlohmann::json::array_t* array =
        jsonResponse.get_ptr<nlohmann::json::array_t*>();
    if (array != nullptr)
    {
        size_t index = 0;
        for (auto& element : *array)
        {
            nlohmann::json::json_pointer newPtr = p / index;
            BMCWEB_LOG_DEBUG << "Traversing response at " << newPtr.to_string();
            findNavigationReferencesRecursive(element, newPtr, out);
            index++;
        }
    }
    nlohmann::json::object_t* obj =
        jsonResponse.get_ptr<nlohmann::json::object_t*>();
    if (obj != nullptr)
    {
        // Navigation References only ever have a single element
        if (obj->size() == 1)
        {
            if (obj->begin()->first == "@odata.id")
            {
                const std::string* uri =
                    obj->begin()->second.get_ptr<const std::string*>();
                if (uri != nullptr)
                {
                    BMCWEB_LOG_DEBUG << "Found element at " << p.to_string();
                    out.emplace_back(p, *uri);
                }
            }
        }
        for (auto& element : *obj)
        {
            BMCWEB_LOG_DEBUG << "Traversing response at " << p / element.first;
            nlohmann::json::json_pointer newPtr = p / element.first;

            findNavigationReferencesRecursive(element.second, newPtr, out);
        }
    }
}

inline std::vector<ExpandNode>
    findNavigationReferences(nlohmann::json& jsonResponse,
                             nlohmann::json::json_pointer root)
{
    std::vector<ExpandNode> ret;
    findNavigationReferencesRecursive(jsonResponse, std::move(root), ret);
    return ret;
}

struct MultiAsyncResp : public std::enable_shared_from_this<MultiAsyncResp>
{
    // This object takes a single asyncResp object as the "final" one, then
    // allows callers to attach sub-responses within the json tree that need to
    // be executed and filled into their appropriate locations.  This class
    // manages the final "merge" of the json resources.
    MultiAsyncResp(crow::App& app, std::shared_ptr<bmcweb::AsyncResp> final) :
        app(app), finalRes(std::move(final))
    {}

    void addAwaitingResponse(
        Query query, std::shared_ptr<bmcweb::AsyncResp>& res,
        const nlohmann::json::json_pointer& finalExpandLocation)
    {
        res->res.setCompleteRequestHandler(std::bind_front(
            onEndStatic, shared_from_this(), query, finalExpandLocation));
    }

    void onEnd(Query query, const nlohmann::json::json_pointer& locationToPlace,
               crow::Response& res)
    {
        nlohmann::json& finalObj = finalRes->res.jsonValue[locationToPlace];
        finalObj = std::move(res.jsonValue);

        if (query.expandLevel <= 0)
        {
            // Last level to expand, no need to go deeper
            return;
        }
        // Now decrease the depth by one to account for the tree node we just
        // resolved
        query.expandLevel--;

        std::vector<ExpandNode> nodes =
            findNavigationReferences(finalObj, locationToPlace);
        BMCWEB_LOG_DEBUG << nodes.size() << " nodes to traverse";
        for (const ExpandNode& node : nodes)
        {
            BMCWEB_LOG_DEBUG << "Expanding " << locationToPlace;
            std::error_code ec;
            crow::Request newReq({boost::beast::http::verb::get, node.uri, 11},
                                 ec);
            if (ec)
            {
                messages::internalError(res);
                return;
            }

            auto asyncResp = std::make_shared<bmcweb::AsyncResp>();
            BMCWEB_LOG_DEBUG << "setting completion handler on "
                             << &asyncResp->res;
            addAwaitingResponse(query, asyncResp, node.location);
            app.handle(newReq, asyncResp);
        }
    }

  private:
    static void onEndStatic(const std::shared_ptr<MultiAsyncResp>& multi,
                            Query query,
                            const nlohmann::json::json_pointer& locationToPlace,
                            crow::Response& res)
    {
        multi->onEnd(query, locationToPlace, res);
    }

    crow::App& app;
    std::shared_ptr<bmcweb::AsyncResp> finalRes;
};

inline void
    processAllParams(crow::App& app, const std::optional<Query>& query,

                     std::function<void(crow::Response&)>& completionHandler,
                     crow::Response& intermediateResponse)
{
    if (!completionHandler)
    {
        BMCWEB_LOG_DEBUG << "Function was invalid?";
        return;
    }

    BMCWEB_LOG_DEBUG << "Processing query params";
    if (!query)
    {
        BMCWEB_LOG_DEBUG << "No query params to process";
        // Query params weren't valid, no need to continue;
        completionHandler(intermediateResponse);
        return;
    }

    // If the request failed, there's no reason to even try to run query
    // params.
    if (intermediateResponse.resultInt() < 200 ||
        intermediateResponse.resultInt() >= 400)
    {
        completionHandler(intermediateResponse);
        return;
    }
    if (query->isOnly)
    {
        processOnly(app, intermediateResponse, completionHandler);
        return;
    }
    if (query->expandType != ExpandType::None)
    {
        BMCWEB_LOG_DEBUG << "Executing expand query";
        // TODO(ed) this is a copy of the response object.  Admittedly, we're
        // inherently doing something inefficient, but we shouldn't have to do a
        // full copy
        auto asyncResp = std::make_shared<bmcweb::AsyncResp>();
        asyncResp->res.setCompleteRequestHandler(std::move(completionHandler));
        asyncResp->res.jsonValue = std::move(intermediateResponse.jsonValue);
        auto multi = std::make_shared<MultiAsyncResp>(app, asyncResp);

        // Start the chain by "ending" the root response
        multi->onEnd(*query, nlohmann::json::json_pointer(""), asyncResp->res);
        return;
    }
    completionHandler(intermediateResponse);
}

} // namespace query_param
} // namespace redfish
