

#ifndef ROUTING_LOGGER_H
#define ROUTING_LOGGER_H


#include "config.h"
#include <spdlog/spdlog.h>

#include <set>
#include <string>

namespace routing
{

program_options::options_description
get_logger_options_description();

void 
init_logger(routing::Config const& config);

void 
init_logger(std::string const& log_file, bool async_logging);

using Logger_t = std::shared_ptr<spdlog::logger>;

class Logger_factory
{
public:
    Logger_factory(routing::Config const& config);

    Logger_factory(
        std::string const& log_file,
        bool async_logging,
        bool flush_on_info,
        std::vector<std::string> const& debug_loggers);

    Logger_t get_default_logger(std::string const& name);

    Logger_t get_st_custom_logger(
        std::string const& name,
        std::string const& file);

    Logger_t get_mt_custom_logger(
        std::string const& name,
        std::string const& file);
private:
    std::vector<spdlog::sink_ptr> m_default_sinks;
    bool m_async_logging;
    bool m_flush_on_info;

    std::set<std::string> m_debug_loggers;
};

// std::shared_ptr<spdlog::logger>
// create_logger(routing::Config const& config, std::string const& name);
//
Logger_t
get_default_logger();

Logger_t
get_default_logger(std::string const& name);

Logger_t
get_st_custom_logger(std::string const& name, std::string const& file);

Logger_t
get_mt_custom_logger(std::string const& name, std::string const& file);


};


#endif
