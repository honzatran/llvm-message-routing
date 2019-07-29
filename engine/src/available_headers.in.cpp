
#include <string>
#include <vector>

std::string g_common_install_headers = "@COMMON_INSTALL_HEADER_DIR@";
std::string g_common_headers         = "@COMMON_HEADER_DIR@";

std::string g_3rd_party_spd_log
    = "@PROJECT_SOURCE_DIR@/3rd-party/spdlog-1.3.1/include";

std::string_view
get_common_headers_path()
{
    return g_common_install_headers;
}

namespace routing::engine
{
std::vector<std::string>
get_include_paths()
{
    return {g_common_install_headers, g_common_headers, g_3rd_party_spd_log};
}

}  // namespace routing::engine
