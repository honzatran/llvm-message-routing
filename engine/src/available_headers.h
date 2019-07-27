
#ifndef ROUTING_ENGINE_AVAILABLE_HEADERS_H
#define ROUTING_ENGINE_AVAILABLE_HEADERS_H

#include <vector>
#include <string>

namespace routing::engine
{

/// Returns the include files for the jit compilation
std::vector<std::string>
get_include_paths();

}

#endif
