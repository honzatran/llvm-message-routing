
#ifndef ROUTING_THREAD_FACTORY_H
#define ROUTING_THREAD_FACTORY_H

#include "logger.h"
#include "config.h"

#include <thread>

#if __linux__
#include <sched.h>
#endif

namespace routing
{

class IThread_factory
{
public:
    virtual ~IThread_factory() = default;

    virtual std::thread create(
        std::string const& thread_name, std::function<void()> function)
        = 0;
};

class Basic_thread_factory : public IThread_factory
{
public:
    Basic_thread_factory()
    {
        m_logger = routing::get_default_logger("Basic_thread_factory");
    }

    std::thread create(
        std::string const& thread_name, std::function<void()> function) override
    {
        Logger_t logger = m_logger;
        return std::thread([logger, function, thread_name] {
            logger->info("Starting thread {}", thread_name);
            function();
        });
    }

private:
    Logger_t m_logger;
};

class Affinity_guard
{
#if __linux__
public:
    Affinity_guard(std::uint32_t core)
    {
        pthread_getaffinity_np(
            pthread_self(), sizeof(cpu_set_t), &m_old_set);

        cpu_set_t new_set;
        CPU_ZERO(&new_set);
        CPU_SET(core, &new_set);

        pthread_setaffinity_np(
            pthread_self(), sizeof(cpu_set_t), &new_set);
    }

    ~Affinity_guard()
    {
        pthread_setaffinity_np(
            pthread_self(), sizeof(cpu_set_t), &m_old_set);
    }

private:
    cpu_set_t m_old_set;

#else
    Affinity_guard(std::uint32_t core) {}
#endif
};

program_options::options_description
get_thread_factory_desc();


class Thread_pinning_factory : public Basic_thread_factory
{
public:
    Thread_pinning_factory(Config const& config)
    {
        m_logger = routing::get_default_logger("Thread_pinning_factory");

        load_pinned_threads(config);
    }

    std::thread create(
        std::string const& thread_name, std::function<void()> function) override;
private:
    Logger_t m_logger;

    std::unordered_map<std::string, int> m_cpu_ids;

    void load_pinned_threads(Config const& config);
    int get_cpu_id(std::string const& thread_name);

    bool is_valid_config_line(std::string const& thread_name);
    void store_pinning(
        std::string const& thread_name,
        std::string& cpu_str);
};

void init_thread_factory(Config const& config);
std::shared_ptr<IThread_factory> get_thread_factory();

}


#endif
