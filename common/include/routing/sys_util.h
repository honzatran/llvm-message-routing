

#ifndef ROUTING_SYS_UTIL_H
#define ROUTING_SYS_UTIL_H

#include <cstdlib>
#include <string>
#include <boost/optional.hpp>

namespace routing
{
boost::optional<std::string>
get_env_variable(std::string const& env_name)
{
    char const* env_var = std::getenv(env_name.c_str());

    if (env_var != nullptr)
    {
        return std::string(env_var);
    }

    return {};
}
};

#endif
