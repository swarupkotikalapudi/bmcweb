#pragma once

#include <boost/beast/http/verb.hpp>

#include <array>

namespace http
{

// Note, this is an imperfect abstraction.  There are a lot of verbs that we
// use memory for, but are basically unused by most implementations.
// Ideally we would have a list of verbs that we do use, and only index in
// to a smaller array of those, but that would require a translation from
// boost::beast::http::verb, to the bmcweb index.
inline constexpr size_t maxVerbIndex =
    static_cast<size_t>(boost::beast::http::verb::patch);

// MaxVerb + 1 is designated as the "not found" verb.  It is done this way
// to keep the BaseRule as a single bitfield (thus keeping the struct small)
// while still having a way to declare a route a "not found" route.
inline constexpr size_t notFoundIndex = maxVerbIndex + 1;
inline constexpr size_t methodNotAllowedIndex = notFoundIndex + 1;
inline constexpr size_t maxNumMethods = methodNotAllowedIndex + 1;

inline constexpr std::array<boost::beast::http::verb, 6> allRedfishMethods = {
    boost::beast::http::verb::delete_, boost::beast::http::verb::get,
    boost::beast::http::verb::head,    boost::beast::http::verb::post,
    boost::beast::http::verb::put,     boost::beast::http::verb::patch,
};

inline constexpr std::array<const char*, 6> allRedfishMethodsStr = {
    "DELETE", "GET", "HEAD", "POST", "PUT", "PATCH",
};

namespace details
{

constexpr std::array<size_t, static_cast<size_t>(maxNumMethods)>
    getBoostVerbIndex()
{
    std::array<size_t, static_cast<size_t>(maxNumMethods)> mapping{};
    // By default set index to the length of all Redfish methods
    for (size_t& i : mapping)
    {
        i = allRedfishMethods.size();
    }
    // Then index used HTTP methods one by one
    size_t index = 0;
    for (boost::beast::http::verb method : allRedfishMethods)
    {
        mapping[static_cast<size_t>(method)] = index++;
    }

    return mapping;
}

} // namespace details

inline constexpr std::array<size_t, static_cast<size_t>(maxNumMethods)>
    boostVerbIndex = details::getBoostVerbIndex();

} // namespace http