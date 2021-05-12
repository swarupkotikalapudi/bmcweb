#pragma once
#include "async_resp.hpp"
#include "error_messages.hpp"
#include "http_request.hpp"

#include <systemd/sd-journal.h>

#include <app.hpp>

#include <string_view>
#include <vector>
namespace redfish
{
namespace query_param
{
class ProcessParam
{
  public:
    ProcessParam(App& appIn, crow::Response& resIn) : app(appIn), res(resIn)
    {}
    ~ProcessParam() = default;

    bool processAllParam(const crow::Request& req)
    {
        if (res.resultInt() < 200 || res.resultInt() >= 400)
        {
            return false;
        }
        if (!isParsed)
        {
            if (!checkParameters(req))
            {
                return false;
            }
        }
        if (isOnly)
        {
            if (processOnly(req))
            {
                isParsed = false;
                return true;
            }
            return false;
        }
        if (isExpand)
        {
            if (processExpand(req))
            {
                return true;
            }
        }
        if (isSelect)
        {
            processSelect();
        }
        if (isSkip)
        {
            processSkip(req);
        }
        if (isTop)
        {
            processTop(req);
        }
        isParsed = false;
        return false;
    }

  private:
    bool checkParameters(const crow::Request& req)
    {
        isParsed = true;
        isOnly = false;
        auto it = req.urlParams.find("only");
        if (it != req.urlParams.end())
        {
            if (req.urlParams.size() != 1)
            {
                res.jsonValue.clear();
                messages::queryCombinationInvalid(res);
                return false;
            }
            if (!it->value().empty())
            {
                res.jsonValue.clear();
                messages::queryParameterValueFormatError(res, it->value(),
                                                         it->key());
                return false;
            }
            isOnly = true;
            return true;
        }
        isExpand = false;
        isSelect = false;
        isSkip = false;
        isTop = false;
        for (auto ite : req.urlParams)
        {
            if (ite->key() == "$expand")
            {
                std::string value = std::string(ite->value());
                if (value == "*" || value == "." || value == "~")
                {
                    isExpand = true;
                    expandType = std::move(value);
                    expandLevel = 1;
                }
                else if (value.size() >= 12 &&
                         value.substr(1, 9) == "($levels=" &&
                         (value[0] == '*' || value[0] == '.' ||
                          value[0] == '~') &&
                         value[value.size() - 1] == ')')
                {
                    isExpand = true;
                    expandType = value[0];
                    expandLevel =
                        strtoul(value.substr(10, value.size() - 2).c_str(),
                                nullptr, 10);
                    expandLevel =
                        expandLevel < maxLevel ? expandLevel : maxLevel;
                }
                else
                {
                    res.jsonValue.clear();
                    messages::queryParameterValueFormatError(res, ite->value(),
                                                             ite->key());
                    return false;
                }
            }
            else if (ite->key() == "$select")
            {
                isSelect = true;
                selectPropertyVec.clear();
                std::string value = std::string(ite->value());
                auto result = stringSplit(value, ",");
                for (auto& itResult : result)
                {
                    auto v = stringSplit(itResult, "/");
                    if (v.empty())
                    {
                        continue;
                    }
                    selectPropertyVec.push_back(std::move(v));
                }
            }
            else if (ite->key() == "$skip")
            {
                skipValue = strtoul(ite->value().c_str(), nullptr, 10);
                if (skipValue > 0)
                {
                    isSkip = true;
                }
            }
            else if (ite->key() == "$top")
            {
                topValue = strtoul(ite->value().c_str(), nullptr, 10);
                if (topValue > 0)
                {
                    isTop = true;
                }
            }
            continue;
        }
        return true;
    }

    bool processOnly(const crow::Request& req)
    {
        auto itMembers = res.jsonValue.find("Members");
        if (itMembers == res.jsonValue.end())
        {
            res.jsonValue.clear();
            messages::actionParameterNotSupported(res, "only", req.url);
            return false;
        }

        if (itMembers->size() != 1)
        {
            BMCWEB_LOG_DEBUG << "Members contains " << itMembers->size()
                             << " element, returning full collection.";
            return false;
        }
        auto itMemBegin = itMembers->begin();
        auto itUrl = itMemBegin->find("@odata.id");
        if (itUrl == itMemBegin->end())
        {
            BMCWEB_LOG_DEBUG << "No found odata.id";
            return false;
        }
        const std::string url = itUrl->get<const std::string>();
        newReq.emplace(req.req);
        newReq->session = req.session;
        newReq->setTarget(url);
        res.jsonValue.clear();
        auto asyncResp = std::make_shared<bmcweb::AsyncResp>(res);
        app.handle(*newReq, asyncResp);
        return true;
    }

    bool processExpand(const crow::Request& req)
    {
        if (pendingUrlVec.size() == 0)
        {
            if (res.jsonValue.find("isExpand") == res.jsonValue.end())
            {
                res.jsonValue["isExpand"] = expandLevel + 1;
            }
            recursiveHyperlinks(res.jsonValue);
            if (pendingUrlVec.size() == 0)
            {
                deleteExpand(res.jsonValue);
                return false;
            }
            jsonValue = res.jsonValue;
            newReq.emplace(req.req);
            newReq->session = req.session;
            newReq->setTarget(pendingUrlVec[0] + "?$expand=" + expandType +
                              "($levels=" + std::to_string(expandLevel) + ")");
            res.jsonValue.clear();
            auto asyncResp = std::make_shared<bmcweb::AsyncResp>(res);
            app.handle(*newReq, asyncResp);
            return true;
        }
        if (res.jsonValue.find("isExpand") == res.jsonValue.end())
        {
            res.jsonValue["isExpand"] = expandLevel;
        }
        insertJson(jsonValue, pendingUrlVec[0], res.jsonValue);
        pendingUrlVec.erase(pendingUrlVec.begin());
        if (pendingUrlVec.size() != 0)
        {
            newReq.emplace(req.req);
            newReq->session = req.session;
            newReq->setTarget(pendingUrlVec[0] + "?$expand=" + expandType +
                              "($levels=" + std::to_string(expandLevel) + ")");
            res.jsonValue.clear();
            auto asyncResp = std::make_shared<bmcweb::AsyncResp>(res);
            app.handle(*newReq, asyncResp);
            return true;
        }
        expandLevel -= 1;
        if (expandLevel == 0)
        {
            res.jsonValue = jsonValue;
            jsonValue.clear();
            deleteExpand(res.jsonValue);
            return false;
        }
        recursiveHyperlinks(jsonValue);
        if (pendingUrlVec.size() == 0)
        {
            deleteExpand(res.jsonValue);
            return false;
        }
        newReq.emplace(req.req);
        newReq->session = req.session;
        newReq->setTarget(pendingUrlVec[0] + "?$expand=" + expandType +
                          "($levels=" + std::to_string(expandLevel) + ")");
        res.jsonValue.clear();
        auto asyncResp = std::make_shared<bmcweb::AsyncResp>(res);
        app.handle(*newReq, asyncResp);
        return true;
    }

    void processSelect()
    {
        jsonValue = nlohmann::json::object();
        for (auto& it : selectPropertyVec)
        {
            recursiveSelect(it, res.jsonValue, jsonValue);
        }
        res.jsonValue = jsonValue;
    }

    void processSkip(const crow::Request& req)
    {
        auto it = res.jsonValue.find("Members");
        if (it == res.jsonValue.end())
        {
            res.jsonValue.clear();
            messages::actionParameterNotSupported(res, "$skip", req.url);
            return;
        }

        if (it->size() <= skipValue)
        {
            it->clear();
            return;
        }
        it->erase(it->begin(), it->begin() + int(skipValue));
    }

    void processTop(const crow::Request& req)
    {
        auto it = res.jsonValue.find("Members");
        if (it == res.jsonValue.end())
        {
            res.jsonValue.clear();
            messages::actionParameterNotSupported(res, "$top", req.url);
            return;
        }
        if (it->size() <= topValue)
        {
            return;
        }
        it->erase(it->begin() + int(topValue), it->end());
    }

    void recursiveHyperlinks(nlohmann::json& j)
    {
        auto itExpand = j.find("isExpand");
        if (itExpand == j.end())
        {
            auto it = j.find("@odata.id");
            if (it != j.end())
            {
                std::string url = it->get<std::string>();

                if (url.find('#') == std::string::npos)
                {
                    pendingUrlVec.push_back(url);
                }
            }
            else if (expandType == "*" || expandType == ".")
            {
                it = j.find("@Redfish.Settings");
                if (it != j.end())
                {
                    std::string url = it->get<std::string>();
                    if (url.find('#') == std::string::npos)
                    {
                        pendingUrlVec.push_back(url);
                    }
                }
                else
                {
                    it = j.find("@Redfish.ActionInfo");
                    if (it != j.end())
                    {
                        std::string url = it->get<std::string>();
                        if (url.find('#') == std::string::npos)
                        {
                            pendingUrlVec.push_back(url);
                        }
                    }
                    else
                    {
                        it = j.find("@Redfish.CollectionCapabilities");
                        if (it != j.end())
                        {
                            std::string url = it->get<std::string>();
                            if (url.find('#') == std::string::npos)
                            {
                                pendingUrlVec.push_back(url);
                            }
                        }
                    }
                }
            }
        }
        for (auto it = j.begin(); it != j.end(); ++it)
        {
            if (it->is_object())
            {
                if (expandType == "." && it.key() == "Links")
                {
                    continue;
                }
                recursiveHyperlinks(*it);
            }
            else if (it->is_array())
            {
                for (auto& itArray : *it)
                {
                    recursiveHyperlinks(itArray);
                }
            }
        }
    }

    bool insertJson(nlohmann::json& base, const std::string& pos,
                    const nlohmann::json& data)
    {
        auto itExpand = base.find("isExpand");
        if (itExpand != base.end())
        {
            if (itExpand->get<unsigned int>() <= expandLevel)
            {
                return false;
            }
        }
        else
        {
            auto it = base.find("@odata.id");
            if (it != base.end())
            {
                if (it->get<std::string>() == pos)
                {
                    base = data;
                    return true;
                }
            }
            else if (expandType == "*" || expandType == ".")
            {
                it = base.find("@Redfish.Settings");
                if (it != base.end())
                {
                    if (it->get<std::string>() == pos)
                    {
                        base = data;
                        return true;
                    }
                }
                else
                {
                    it = base.find("@Redfish.ActionInfo");
                    if (it != base.end())
                    {
                        if (it->get<std::string>() == pos)
                        {
                            base = data;
                            return true;
                        }
                    }
                    else
                    {
                        it = base.find("@Redfish.CollectionCapabilities");
                        if (it != base.end())
                        {
                            if (it->get<std::string>() == pos)
                            {
                                base = data;
                                return true;
                            }
                        }
                    }
                }
            }
        }
        for (auto it = base.begin(); it != base.end(); ++it)
        {
            if (it->is_object())
            {
                if (expandType == "." && it.key() == "Links")
                {
                    continue;
                }
                if (insertJson(*it, pos, data))
                {
                    return true;
                }
            }
            else if (it->is_array())
            {
                for (auto& itArray : *it)
                {
                    if (insertJson(itArray, pos, data))
                    {
                        return true;
                    }
                }
            }
        }
        return false;
    }
    void deleteExpand(nlohmann::json& j)
    {
        auto it = j.find("isExpand");
        if (it != j.end())
        {
            j.erase(it);
        }
        for (auto& it : j)
        {
            if (it.is_structured())
            {
                deleteExpand(it);
            }
        }
    }

    std::vector<std::string> stringSplit(const std::string& s,
                                         const std::string& delim)
    {
        std::vector<std::string> elems;
        size_t pos = 0;
        size_t len = s.length();
        size_t delimLen = delim.length();
        if (delimLen == 0)
        {
            return elems;
        }
        while (pos < len)
        {
            size_t findPos = s.find(delim, pos);
            if (findPos == std::string::npos)
            {
                elems.push_back(s.substr(pos, len - pos));
                break;
            }
            elems.push_back(s.substr(pos, findPos - pos));
            pos = findPos + delimLen;
        }
        return elems;
    }

    void recursiveSelect(std::vector<std::string> v, nlohmann::json& src,
                         nlohmann::json& dst)
    {
        if (v.empty())
        {
            return;
        }
        if (src.is_array())
        {
            for (auto it : src)
            {
                dst.push_back(nlohmann::json());
                recursiveSelect(v, it, dst.back());
            }
            return;
        }
        std::string key = v[0];
        auto it = src.find(key);
        if (v.size() == 1)
        {
            dst.insert(it, ++it);
        }
        else
        {
            v.erase(v.begin());
            if (it->is_array())
            {
                dst[key] = nlohmann::json::array();
            }
            else
            {
                dst[key] = nlohmann::json::object();
            }
            recursiveSelect(v, *it, dst[key]);
        }
        it = src.find("@odata.id");
        if (it != src.end())
        {
            dst.insert(it, ++it);
        }
        it = src.find("@odata.type");
        if (it != src.end())
        {
            dst.insert(it, ++it);
        }
        it = src.find("@odata.context");
        if (it != src.end())
        {
            dst.insert(it, ++it);
        }
        it = src.find("@odata.etag");
        if (it != src.end())
        {
            dst.insert(it, ++it);
        }
    }

  private:
    App& app;
    crow::Response& res;
    std::optional<crow::Request> newReq;
    bool isParsed = false;
    bool isOnly = false;
    bool isExpand = false;
    bool isSelect = false;
    bool isSkip = false;
    bool isTop = false;
    uint64_t expandLevel;
    const uint64_t maxLevel = 2;
    uint64_t skipValue = 0;
    uint64_t topValue = 0;
    std::string expandType;
    std::vector<std::string> pendingUrlVec;
    std::vector<std::vector<std::string>> selectPropertyVec;
    nlohmann::json jsonValue;
};
} // namespace query_param
} // namespace redfish
