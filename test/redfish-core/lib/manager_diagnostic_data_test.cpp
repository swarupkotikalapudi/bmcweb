#include "async_resp.hpp"
#include "manager_diagnostic_data.hpp"

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

namespace redfish
{
namespace
{

void verifyManagerDataGetFailure(crow::Response& res)
{
    EXPECT_EQ(res.result(), boost::beast::http::status::internal_server_error);
    res.clear();
}

TEST(ManagerDiagnosticDataTest, ManagerDataGetFailure)
{
    auto asyncResp = std::make_shared<bmcweb::AsyncResp>();
    boost::system::error_code ec = boost::asio::error::operation_aborted;

    afterGetFreeStorageStatistics(asyncResp, ec, 0);
    verifyManagerDataGetFailure(asyncResp->res);

    afterGetProcessorKernelStatistics(asyncResp, ec, 0);
    verifyManagerDataGetFailure(asyncResp->res);

    afterGetProcessorUserStatistics(asyncResp, ec, 0);
    verifyManagerDataGetFailure(asyncResp->res);

    afterGetAvailableMemoryStatistics(asyncResp, ec, 0);
    verifyManagerDataGetFailure(asyncResp->res);

    afterGetBufferedAndCachedMemoryStatistics(asyncResp, ec, 0);
    verifyManagerDataGetFailure(asyncResp->res);

    afterGetFreeMemoryStatistics(asyncResp, ec, 0);
    verifyManagerDataGetFailure(asyncResp->res);

    afterGetSharedMemoryStatistics(asyncResp, ec, 0);
    verifyManagerDataGetFailure(asyncResp->res);

    afterGetTotalMemoryStatistics(asyncResp, ec, 0);
    verifyManagerDataGetFailure(asyncResp->res);
}

TEST(ManagerDiagnosticDataTest, ManagerDataGetNullPtr)
{
    auto asyncResp = std::make_shared<bmcweb::AsyncResp>();

    afterGetFreeStorageStatistics(asyncResp, {},
                                  std::numeric_limits<double>::quiet_NaN());
    EXPECT_EQ(asyncResp->res.jsonValue["FreeStorageSpaceKiB"], nullptr);
    asyncResp->res.clear();

    afterGetProcessorKernelStatistics(asyncResp, {},
                                      std::numeric_limits<double>::quiet_NaN());
    EXPECT_EQ(asyncResp->res.jsonValue["ProcessorStatistics"]["KernelPercent"],
              nullptr);
    asyncResp->res.clear();

    afterGetProcessorUserStatistics(asyncResp, {},
                                    std::numeric_limits<double>::quiet_NaN());
    EXPECT_EQ(asyncResp->res.jsonValue["ProcessorStatistics"]["UserPercent"],
              nullptr);
    asyncResp->res.clear();

    afterGetAvailableMemoryStatistics(asyncResp, {},
                                      std::numeric_limits<double>::quiet_NaN());
    EXPECT_EQ(asyncResp->res.jsonValue["MemoryStatistics"]["AvailableBytes"],
              nullptr);
    asyncResp->res.clear();

    afterGetBufferedAndCachedMemoryStatistics(
        asyncResp, {}, std::numeric_limits<double>::quiet_NaN());
    EXPECT_EQ(
        asyncResp->res.jsonValue["MemoryStatistics"]["BuffersAndCacheBytes"],
        nullptr);
    asyncResp->res.clear();

    afterGetFreeMemoryStatistics(asyncResp, {},
                                 std::numeric_limits<double>::quiet_NaN());
    EXPECT_EQ(asyncResp->res.jsonValue["MemoryStatistics"]["FreeBytes"],
              nullptr);
    asyncResp->res.clear();

    afterGetSharedMemoryStatistics(asyncResp, {},
                                   std::numeric_limits<double>::quiet_NaN());
    EXPECT_EQ(asyncResp->res.jsonValue["MemoryStatistics"]["SharedBytes"],
              nullptr);
    asyncResp->res.clear();

    afterGetTotalMemoryStatistics(asyncResp, {},
                                  std::numeric_limits<double>::quiet_NaN());
    EXPECT_EQ(asyncResp->res.jsonValue["MemoryStatistics"]["TotalBytes"],
              nullptr);
    asyncResp->res.clear();
}

TEST(ManagerDiagnosticDataTest, ManagerDataGetSuccess)
{
    auto asyncResp = std::make_shared<bmcweb::AsyncResp>();

    afterGetFreeStorageStatistics(asyncResp, {}, 204800.0);
    EXPECT_EQ(asyncResp->res.jsonValue["FreeStorageSpaceKiB"], 200);
    asyncResp->res.clear();

    afterGetProcessorKernelStatistics(asyncResp, {}, 50.23456789);
    EXPECT_EQ(asyncResp->res.jsonValue["ProcessorStatistics"]["KernelPercent"],
              50.2346);
    asyncResp->res.clear();

    afterGetProcessorUserStatistics(asyncResp, {}, 60.23456789);
    EXPECT_EQ(asyncResp->res.jsonValue["ProcessorStatistics"]["UserPercent"],
              60.2346);
    asyncResp->res.clear();

    afterGetAvailableMemoryStatistics(asyncResp, {}, 2048.7);
    EXPECT_EQ(asyncResp->res.jsonValue["MemoryStatistics"]["AvailableBytes"],
              2048);
    asyncResp->res.clear();

    afterGetBufferedAndCachedMemoryStatistics(asyncResp, {}, 2048.7);
    EXPECT_EQ(
        asyncResp->res.jsonValue["MemoryStatistics"]["BuffersAndCacheBytes"],
        2048);
    asyncResp->res.clear();

    afterGetFreeMemoryStatistics(asyncResp, {}, 2048.7);
    EXPECT_EQ(asyncResp->res.jsonValue["MemoryStatistics"]["FreeBytes"], 2048);
    asyncResp->res.clear();

    afterGetSharedMemoryStatistics(asyncResp, {}, 2048.7);
    EXPECT_EQ(asyncResp->res.jsonValue["MemoryStatistics"]["SharedBytes"],
              2048);
    asyncResp->res.clear();

    afterGetTotalMemoryStatistics(asyncResp, {}, 2048.7);
    EXPECT_EQ(asyncResp->res.jsonValue["MemoryStatistics"]["TotalBytes"], 2048);
    asyncResp->res.clear();
}

} // namespace
} // namespace redfish
