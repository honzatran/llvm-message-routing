

#ifndef ROUTING_TESTING_H
#define ROUTING_TESTING_H

#define TEST_CONFIG_PATH "../config/test_application.properties"

#include <gtest/gtest.h>
#include <spdlog/fmt/ostr.h>

#include <ostream>
#include <system_error>

// std::string const Config_constant::k_config_path = 
//
//
namespace routing
{

#define EXPECT_ERROR_CODE(ec, msg) EXPECT_FALSE(ec) << msg \
    << " failed due to: " << ec.message()

inline void 
expect_error_code(std::error_code ec, std::string const& msg)
{
    EXPECT_FALSE(ec) << msg << "failed due to : " <<  ec.message();
}


struct Source_location
{
     char const* file;
     char const* function;
     int line;
};

#define SOURCE_LOCATION() \
    Source_location{__FILE__, __PRETTY_FUNCTION__, __LINE__}

#define DEFINED(name) \
    fmt::format("{} defined at {}", name, SOURCE_LOCATION())

inline std::ostream& 
operator<<(std::ostream& oss, Source_location source_location)
{
    oss << source_location.file << ":" << source_location.function << ":"
        << source_location.line;

    return oss;
}




}

#endif
