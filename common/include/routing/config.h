
#ifndef ROUTING_CONFIG_H
#define ROUTING_CONFIG_H

#include <iostream>
#include <vector>

#include <boost/optional.hpp>
#include <boost/program_options.hpp>

#include "fmt.h"

namespace program_options = boost::program_options;

namespace routing
{
class Config;

Config
parse_args(
    std::vector<program_options::options_description> const& descriptions,
    int argc, char** argv, std::string const& config_path);

Config
parse_args(
    std::vector<program_options::options_description> const& descriptions,
    std::string const& path);

class Config
{
public:
    Config() = default;
    Config(program_options::variables_map vm);

    template <typename T>
    T get_option(std::string const& key) const
    {
        auto optional = get<T>(key);

        if (optional)
        {
            return *optional;
        }
        
        throw std::runtime_error(fmt::format("Missing option key {}", key));
    }

    template <typename T>
    boost::optional<T> get(std::string const& key) const
    {
        if (m_vm.count(key))
        {
            return m_vm[key].as<T>();
        }

        return {};
    }

    template <typename T>
    void put_option(std::string const& key, T option)
    {
        decltype(m_vm.begin()) it;
        bool inserted;

        std::tie(it, inserted) = m_vm.insert(std::make_pair(
            key, program_options::variable_value(option, false)));

        if (!inserted)
        {
            it->second = program_options::variable_value(option, false);
        }

        program_options::notify(m_vm);

    }

    void add_options(const program_options::variables_map& vm){
        
    }

private:
    program_options::variables_map m_vm;
};

template <typename T>
program_options::typed_value<T>*
create_option()
{
    return program_options::value<T>();
}

template <typename T>
program_options::typed_value<T>*
create_option(T const& def_value)
{
    auto option = program_options::value<T>();
    return option->default_value(def_value);
}

template <typename T>
program_options::typed_value<std::vector<T>>*
create_multi_option()
{
    return program_options::value<std::vector<T>>();
}

}  // namespace routing

#endif
