
#include "utility.hpp"

#include <filesystem>
#include <fstream>
#include <thread>

#include "gtest/gtest.h"
namespace
{

inline std::string generateBigdata()
{
    std::string result;
    size_t i = 0;
    while (i < 10000)
    {
        result += "sample text";
        i += std::string("sample text").length();
    }
    return result;
}
std::string testString(std::string_view input)
{
    std::string encoded;
    crow::utility::Base64Encoder encoder;
    encoder.encode(input, encoded);
    encoder.finalize(encoded);
    std::string decoded;
    crow::utility::base64Decode(encoded, decoded);
    EXPECT_EQ(decoded, input);
    return encoded;
}
void testBigData(std::string_view input)
{
    std::string encoded;
    crow::utility::Base64Encoder encoder;
    size_t size = input.size();
    size_t chunksize = 100;
    size_t chunks = size / chunksize;
    size_t i = 0;
    while (i < chunks)
    {
        encoder.encode(input.substr(i * chunksize, chunksize), encoded);
        i++;
    }
    if (size % chunksize != 0)
    {
        encoder.encode(input.substr(i * chunksize, size % chunksize), encoded);
    }
    encoder.finalize(encoded);
    std::string decoded;
    crow::utility::base64Decode(encoded, decoded);
    EXPECT_EQ(decoded, input);
    return;
}
TEST(Base64Encoder, SmallString)
{
    EXPECT_EQ(testString("sample text"), "c2FtcGxlIHRleHQ=");
    EXPECT_EQ(testString("a"), "YQ==");
    EXPECT_EQ(testString("ab"), "YWI=");
    EXPECT_EQ(testString("abc"), "YWJj");
    EXPECT_EQ(testString("abcd"), "YWJjZA==");
    EXPECT_EQ(testString("124"), "MTI0");
    EXPECT_EQ(testString("1245"), "MTI0NQ==");
    EXPECT_EQ(testString("&-?"), "Ji0/");
    EXPECT_EQ(testString("&-?="), "Ji0/PQ==");
    EXPECT_EQ(testString("&-?=="), "Ji0/PT0=");
}
TEST(Base64Encoder, LargeString)
{
    std::string bigdata = generateBigdata();
    testBigData(bigdata);
    bigdata += "a";
    testBigData(bigdata);
    bigdata += "b";
    testBigData(bigdata);
    bigdata += "c";
    testBigData(bigdata);
}
} // namespace
