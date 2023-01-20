#include "str_utility.hpp"

#include <string>
#include <vector>

#include <gmock/gmock.h> // IWYU pragma: keep
#include <gtest/gtest.h> // IWYU pragma: keep

// IWYU pragma: no_include <gtest/gtest-message.h>
// IWYU pragma: no_include <gtest/gtest-test-part.h>
// IWYU pragma: no_include "gtest/gtest_pred_impl.h"
// IWYU pragma: no_include <gmock/gmock-matchers.h>

namespace
{

using ::testing::ElementsAre;
using ::testing::IsEmpty;

TEST(Split, PositiveTests)
{
    using bmcweb::split;
    std::vector<std::string> vec;
    split(vec, "xx-abc-xx-abb", '-');
    EXPECT_THAT(vec, ElementsAre("xx", "abc", "xx", "abb"));
    vec.clear();
    split(vec, "", '-');
    EXPECT_THAT(vec, IsEmpty());
}

} // namespace
