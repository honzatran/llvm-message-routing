

#include <routing/logger.h>
#include <routing/stdext.h>

#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <mutex>

#include <spdlog/fmt/fmt.h>

using namespace routing;
using namespace std;

const char* k_log_file_option = "logger.file";
const char* k_log_async_option = "logger.async";
const char* k_log_flush_on_info_option = "logger.flush_on_info";
const char* k_log_debug = "logger.debug";

const char* k_default_logger_name = "logger";

std::once_flag init_logger_flag;
std::unique_ptr<Logger_factory> g_logger_factory;

std::mutex g_logger_factory_mutex;

program_options::options_description
routing::get_logger_options_description()
{
    program_options::options_description logger("logger");

    logger.add_options()
        (k_log_file_option,
         routing::create_option<string>("../logs/sandwich.log"), 
         "output log file");

    logger.add_options()
        (k_log_async_option,
         routing::create_option<bool>(true),
         "use async logging")
        (k_log_flush_on_info_option,
         routing::create_option<bool>(true),
         "flush on info level logging")
        (k_log_debug,
         routing::create_multi_option<std::string>(),
         "list of loggers that use a debug level");

    return logger;
}

void 
routing::init_logger(routing::Config const& config) 
{
    std::string log_file = config.get_option<std::string>(k_log_file_option);

    if (g_logger_factory)
    {
        return;
    }

    bool async_logging = config.get_option<bool>(k_log_async_option);

    if (async_logging)
    {
        spdlog::set_async_mode(
                8192, 
                spdlog::async_overflow_policy::discard_log_msg);
    }

    vector<spdlog::sink_ptr> sinks = 
    { 
        std::make_shared<spdlog::sinks::stderr_sink_st>(),
        std::make_shared<spdlog::sinks::daily_file_sink_st>(log_file, 23, 59)
    };

    std::call_once(
        init_logger_flag,
        [](Config const& config) {
            if (g_logger_factory)
            {
                return;
            }

            g_logger_factory = std::make_unique<Logger_factory>(config);
            
        },
        config);

    g_logger_factory->get_default_logger(k_default_logger_name);
}

void 
init_logger(std::string const& log_file, bool async_logging)
{
    if (g_logger_factory)
    {
        return;
    }

    if (async_logging)
    {
        spdlog::set_async_mode(
                8192, 
                spdlog::async_overflow_policy::discard_log_msg);
    }

    vector<spdlog::sink_ptr> sinks = 
    { 
        std::make_shared<spdlog::sinks::stderr_sink_st>(),
        std::make_shared<spdlog::sinks::daily_file_sink_st>(log_file, 23, 59)
    };

    std::vector<std::string> debug_loggers = {};

    std::call_once(
        init_logger_flag,
        [&log_file, &async_logging, &debug_loggers]() {
            if (g_logger_factory)
            {
                return;
            }

            g_logger_factory = std::make_unique<Logger_factory>(
                log_file, async_logging, true, debug_loggers);
            
        });
}

void guard_logger_factory_initialized()
{
    if (!g_logger_factory)
    {
        throw std::runtime_error("Logger factory not initialized");
    }
}



std::shared_ptr<spdlog::logger>
routing::get_default_logger()
{
    guard_logger_factory_initialized();
    return g_logger_factory->get_default_logger(k_default_logger_name);
}

Logger_t
routing::get_default_logger(std::string const& name)
{
    guard_logger_factory_initialized();
    return g_logger_factory->get_default_logger(name);
}

Logger_t
routing::get_st_custom_logger(
    std::string const& name,
    std::string const& file)
{
    guard_logger_factory_initialized();
    return g_logger_factory->get_st_custom_logger(name, file);
}

Logger_t
routing::get_mt_custom_logger(
    std::string const& name,
    std::string const& file)
{
    guard_logger_factory_initialized();
    return g_logger_factory->get_mt_custom_logger(name, file);
}


Logger_factory::Logger_factory(routing::Config const& config)
{
    std::string log_file = config.get_option<std::string>(k_log_file_option);
    m_async_logging      = config.get_option<bool>(k_log_async_option);
    m_flush_on_info      = config.get_option<bool>(k_log_flush_on_info_option);

    auto debug_loggers
        = config.get<std::vector<std::string>>(k_log_debug);

    if (debug_loggers)
    {
        m_debug_loggers
            = std::set<std::string>(debug_loggers->begin(), debug_loggers->end());
    }
    
    if (m_async_logging)
    {
        spdlog::set_async_mode(
            8192, spdlog::async_overflow_policy::discard_log_msg);

        m_default_sinks = {std::make_shared<spdlog::sinks::stderr_sink_st>(),
                           std::make_shared<spdlog::sinks::daily_file_sink_st>(
                               log_file, 23, 59)};
    }
    else
    {
        m_default_sinks = {std::make_shared<spdlog::sinks::stderr_sink_mt>(),
                           std::make_shared<spdlog::sinks::daily_file_sink_mt>(
                               log_file, 23, 59)};
    }
}

Logger_factory::Logger_factory(
    std::string const& log_file,
    bool async_logging,
    bool flush_on_info,
    std::vector<string> const& debug_loggers)
{
    m_debug_loggers
        = std::set<std::string>(debug_loggers.begin(), debug_loggers.end());
    
    if (m_async_logging)
    {
        spdlog::set_async_mode(
            8192, spdlog::async_overflow_policy::discard_log_msg);

        m_default_sinks = {std::make_shared<spdlog::sinks::stderr_sink_st>(),
                           std::make_shared<spdlog::sinks::daily_file_sink_st>(
                               log_file, 23, 59)};
    }
    else
    {
        m_default_sinks = {std::make_shared<spdlog::sinks::stderr_sink_mt>(),
                           std::make_shared<spdlog::sinks::daily_file_sink_mt>(
                               log_file, 23, 59)};
    }
}

std::shared_ptr<spdlog::logger>
register_combined_logger(
    std::string const& name,
    std::vector<spdlog::sink_ptr> const& sinks)
{
    auto combined_logger = std::make_shared<spdlog::logger>(
             name,
             sinks.begin(),
             sinks.end());
    
    spdlog::register_logger(combined_logger);

    return spdlog::get(name);
}

Logger_t Logger_factory::get_default_logger(std::string const& name)
{
    std::lock_guard<std::mutex> guard(g_logger_factory_mutex);

    Logger_t logger = spdlog::get(name);

    if (logger)
    {
        return logger;
    }

    auto new_logger = register_combined_logger(name, m_default_sinks);

    if (m_flush_on_info)
    {
        new_logger->flush_on(spdlog::level::info);
    }

    if (m_debug_loggers.find(name) != m_debug_loggers.end())
    {
        new_logger->set_level(spdlog::level::debug);
    }

    return new_logger;
}

Logger_t
Logger_factory::get_st_custom_logger(
    std::string const& name,
    std::string const& file)
{
    std::lock_guard<std::mutex> guard(g_logger_factory_mutex);

    Logger_t logger = spdlog::get(name);

    if (logger)
    {
        return logger;
    }

    vector<spdlog::sink_ptr> sinks
        = {std::make_shared<spdlog::sinks::stderr_sink_mt>(),
           std::make_shared<spdlog::sinks::daily_file_sink_mt>(file, 23, 59)};

    return register_combined_logger(name, sinks);
}

Logger_t
Logger_factory::get_mt_custom_logger(
    std::string const& name,
    std::string const& file)
{
    std::lock_guard<std::mutex> guard(g_logger_factory_mutex);

    Logger_t logger = spdlog::get(name);

    if (logger)
    {
        return logger;
    }

    if (m_async_logging)
    {
        vector<spdlog::sink_ptr> sinks = {
            std::make_shared<spdlog::sinks::stderr_sink_st>(),
            std::make_shared<spdlog::sinks::daily_file_sink_st>(file, 23, 59)};

        return register_combined_logger(name, sinks);
    }

    vector<spdlog::sink_ptr> sinks
        = {std::make_shared<spdlog::sinks::stderr_sink_mt>(),
           std::make_shared<spdlog::sinks::daily_file_sink_mt>(file, 23, 59)};

    return register_combined_logger(name, sinks);
}
