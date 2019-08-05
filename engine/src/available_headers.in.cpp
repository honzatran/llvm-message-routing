
#include <string>
#include <vector>

std::string g_common_install_headers = "../include";
std::string g_common_headers         = "@PROJECT_SOURCE_DIR@/common/include";

std::string g_message_headers = "@PROJECT_SOURCE_DIR@/message/include";

std::string g_3rd_party_spd_log
    = "@PROJECT_SOURCE_DIR@/3rd-party/spdlog-1.3.1/include";

std::string g_3rd_party_abseil = "@PROJECT_SOURCE_DIR@/3rd-party/google/abseil";

std::string g_engine_headers = "@ENGINE_HEADERS_DIR@";

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
    return {g_common_install_headers,
            g_common_headers,
            g_message_headers,
            g_3rd_party_abseil,
            g_3rd_party_spd_log,
            g_engine_headers};
}

}  // namespace routing::engine
