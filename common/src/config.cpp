
#include <routing/config.h>
#include <routing/sys_util.h>
#include <routing/stdext.h>

#include <fstream>
#include <utility>
#include <iostream>

namespace ps = program_options;

using namespace std;
using namespace routing;

const char* const alternative_path_env_variable = "CONFIG_FILE";

ps::options_description
create_all_option_description(
    std::vector<ps::options_description> const &descriptions)
{
    ps::options_description desc("All");

    for (auto const &subcomponent_desc : descriptions)
    {
        desc.add(subcomponent_desc);
    }

    return desc;
}

Config
routing::parse_args(
    std::vector<ps::options_description> const &descriptions,
    int argc,
    char **argv,
    std::string const &config_path)
{
    ps::variables_map vm;
    ps::options_description desc = create_all_option_description(descriptions);

    desc.add_options()("help", "show help");

    ps::store(ps::parse_command_line(argc, argv, desc), vm);

    boost::optional<std::string> alternative_config_path
        = routing::get_env_variable(alternative_path_env_variable);

    std::ifstream config_stream(
        alternative_config_path.get_value_or(config_path));
    ps::store(ps::parse_config_file(config_stream, desc, true), vm);

    ps::notify(vm);

    if (vm.count("help") != 0u)
    {
        cout << desc << endl;
        std::exit(0);
    }

    return Config{vm};
}

Config
routing::parse_args(
    std::vector<program_options::options_description> const &descriptions,
    std::string const& path)
{
    ps::options_description desc = create_all_option_description(descriptions);

    std::ifstream config_stream(path);
    ps::variables_map vm;

    ps::store(
            ps::parse_config_file(
                config_stream,
                desc,
                true),
            vm);

    ps::notify(vm);

    return Config{vm};
};

Config::Config(ps::variables_map vm)
    : m_vm(std::move(vm))
{
}

