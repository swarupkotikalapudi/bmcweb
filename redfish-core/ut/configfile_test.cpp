#include "http_response.hpp"
#include "ibm/management_console_rest.hpp"

#include <string>

#include <gtest/gtest.h> // IWYU pragma: keep

// IWYU pragma: no_include <gtest/gtest-message.h>
// IWYU pragma: no_include <gtest/gtest-test-part.h>
// IWYU pragma: no_include "gtest/gtest_pred_impl.h"

namespace crow
{
namespace ibm_mc
{

TEST(ConfigFileTest, FileNameValidChar)
{
    crow::Response res;

    const std::string fileName = "GoodConfigFile";
    EXPECT_TRUE(isValidConfigFileName(fileName, res));
}
TEST(ConfigFileTest, FileNameInvalidChar)
{
    crow::Response res;

    const std::string fileName = "Bad@file";
    EXPECT_FALSE(isValidConfigFileName(fileName, res));
}
TEST(ConfigFileTest, FileNameInvalidPath1)
{
    crow::Response res;

    const std::string fileName = "/../../../../../etc/badpath";
    EXPECT_FALSE(isValidConfigFileName(fileName, res));
}
TEST(ConfigFileTest, FileNameInvalidPath2)
{
    crow::Response res;

    const std::string fileName = "/../../etc/badpath";
    EXPECT_FALSE(isValidConfigFileName(fileName, res));
}
TEST(ConfigFileTest, FileNameInvalidPath3)
{
    crow::Response res;

    const std::string fileName = "/mydir/configFile";
    EXPECT_FALSE(isValidConfigFileName(fileName, res));
}
TEST(ConfigFileTest, FileNameNull)
{
    crow::Response res;

    const std::string fileName;
    EXPECT_FALSE(isValidConfigFileName(fileName, res));
}
TEST(ConfigFileTest, FileNameSlash)
{
    crow::Response res;

    const std::string fileName = "/";
    EXPECT_FALSE(isValidConfigFileName(fileName, res));
}
TEST(ConfigFileTest, FileNameMorethan20Char)
{
    crow::Response res;

    const std::string fileName = "BadfileBadfileBadfile";
    EXPECT_FALSE(isValidConfigFileName(fileName, res));
}

} // namespace ibm_mc
} // namespace crow
