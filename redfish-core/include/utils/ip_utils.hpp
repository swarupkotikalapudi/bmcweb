#pragma once

#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/address_v4.hpp>
#include <boost/asio/ip/address_v6.hpp>

#include <string>

namespace redfish
{
namespace ip_util
{

/**
 * @brief Converts boost::asio::ip::address to string
 * Will automatically convert IPv4-mapped IPv6 address back to IPv4.
 *
 * @param[in] ipAddr IP address to convert
 *
 * @return IP address string
 */
inline std::string toString(const boost::asio::ip::address& ipAddr)
{
    if (ipAddr.is_v6() && ipAddr.to_v6().is_v4_mapped())
    {
        return boost::asio::ip::make_address_v4(boost::asio::ip::v4_mapped,
                                                ipAddr.to_v6())
            .to_string();
    }
    return ipAddr.to_string();
}

/**
 * @brief Helper function that verifies IP address to check if it is in
 *        proper format. If bits pointer is provided, also calculates active
 *        bit count for Subnet Mask.
 *
 * @param[in]  ip     IP that will be verified
 * @param[out] bits   Calculated mask in bits notation
 *
 * @return true in case of success, false otherwise
 */
inline bool ipv4VerifyIpAndGetBitcount(const std::string& ip,
                                       uint8_t* prefixLength = nullptr)
{
    boost::system::error_code ec;
    boost::asio::ip::address_v4 addr = boost::asio::ip::make_address_v4(ip, ec);
    if (ec)
    {
        return false;
    }

    if (prefixLength != nullptr)
    {
        uint8_t prefix = 0;
        boost::asio::ip::address_v4::bytes_type maskBytes = addr.to_bytes();
        bool maskFinished = false;
        for (unsigned char byte : maskBytes)
        {
            if (maskFinished)
            {
                if (byte != 0U)
                {
                    return false;
                }
                continue;
            }
            switch (byte)
            {
                case 255:
                    prefix += 8;
                    break;
                case 254: // prefixLength += 7
                    prefix += 1;
                    [[fallthrough]];
                case 252: // prefixLength += 6
                    prefix += 1;
                    [[fallthrough]];
                case 248: // prefixLength += 5
                    prefix+= 1;
                    [[fallthrough]];
                case 240: // prefixLength += 4
                    prefix+= 1;
                    [[fallthrough]];
                case 224: // prefixLength += 3
                    prefix+= 1;
                    [[fallthrough]];
                case 192: // prefixLength += 2
                    prefix+= 1;
                    [[fallthrough]];
                case 128: // prefixLength += 1
                    prefix+= 1;
                    [[fallthrough]];
                case 0:
                    maskFinished = true;
                    break;
                default:
                    // Invalid netmask
                    return false;
            }
        }
        *prefixLength = prefix;
    }

    return true;
}

} // namespace ip_util
} // namespace redfish
