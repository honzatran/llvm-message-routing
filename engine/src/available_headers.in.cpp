
#include <string>

std::string g_common_install_headers = "@COMMON_INSTALL_HEADER_DIR@";
std::string g_common_headers = "@COMMON_HEADER_DIR@";

std::string_view
get_common_headers_path()
{
    return g_common_install_headers;
}
