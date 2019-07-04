

#include <routing/config.h>

#include <spdlog/fmt/fmt.h>

#include "gtest/gtest.h"

using namespace routing;

TEST(Config_test, put)
{
    Config config;

    std::string name = fmt::format("{}.host", "socket_name");

    config.put_option<std::uint16_t>(name, 1);

    EXPECT_EQ((std::uint16_t) 1, config.get<std::uint16_t>(name));
}


