

#include "../include/routing/thread_factory.h"
#include <fstream>
#include <vector>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/trim.hpp>

using namespace std;
using namespace routing;


const char* const k_thread_pinning_config = "thread.pinning.config";
const char* const k_thread_pinning_enabled = "thread.pinning.enabled";
const char* const k_native_prefix = "native.";

std::shared_ptr<IThread_factory> g_thread_factory;

program_options::options_description
routing::get_thread_factory_desc()
{
    program_options::options_description options("Thread factory");

    options.add_options()(
            k_thread_pinning_enabled,
            create_option<bool>(false),
            "thread pinning enabled")(
            k_thread_pinning_config,
            create_option<std::string>(),
            "config to the thread factory");

    return options;
}

void Thread_pinning_factory::load_pinned_threads(Config const& config)
{
    boost::optional<std::string> thread_pinning_config_path
        = config.get<std::string>(k_thread_pinning_config);

    if (!thread_pinning_config_path)
    {
        m_logger->warn("No thread affinitity config specified");
        return;
    }

    std::ifstream config_stream(*thread_pinning_config_path);

    std::string line;

    while (std::getline(config_stream, line) != nullptr)
    {
        boost::algorithm::trim(line);

        if (line.empty())
        {
            continue;
        }

        vector<std::string> split_tokens;
        boost::split(split_tokens, line, boost::is_any_of("="));

        m_logger->debug("config line {}", line);

        if (split_tokens.size() != 2)
        {
            m_logger->error("wrong format {}", line);
        }
        else
        {
            if (is_valid_config_line(split_tokens[0]))
            {
                store_pinning(split_tokens[0], split_tokens[1]);
            }
        }
    }
}

bool
Thread_pinning_factory::is_valid_config_line(std::string const& thread_name)
{
    return boost::starts_with(thread_name, k_native_prefix);
}

void
Thread_pinning_factory::store_pinning(
    std::string const& thread_name,
    std::string& cpu_str)
{
    std::string real_thread_name = thread_name.substr(7);
    boost::algorithm::trim(cpu_str);

    m_logger->debug("cpu {} length {}", cpu_str, cpu_str.size());
    int cpu_id = boost::lexical_cast<int>(cpu_str);

    if (cpu_id < 0)
    {
        m_logger->error(
            "thread {} will be pinned to invalid cpu {}", thread_name, cpu_id);
    }

    m_logger->debug(
        "thread {} will be pinned to cpu {}", real_thread_name, cpu_id);

    m_cpu_ids.insert(std::make_pair(real_thread_name, cpu_id));
}



int Thread_pinning_factory::get_cpu_id(std::string const& thread_name)
{
    auto it = m_cpu_ids.find(thread_name);

    if (it != m_cpu_ids.end())
    {
        return it->second;
    }

    return -1;
}

std::thread Thread_pinning_factory::create(
    std::string const& thread_name, std::function<void()> function) 
{
    Logger_t logger = m_logger;
    int cpu_id = get_cpu_id(thread_name);

    return std::thread([logger, cpu_id, function, thread_name]() {
        if (cpu_id < 0)
        {
            logger->warn(
                "Starting unpinned thread {}", thread_name);

            function();
        }
        else 
        {
            logger->info(
                "Starting thread {} pinned to cpu {}", thread_name, cpu_id);

            Affinity_guard affinity_guard(cpu_id);
            function();
        }
    });
}

void routing::init_thread_factory(Config const& config)
{
    if (config.get_option<bool>(k_thread_pinning_enabled))
    {
        g_thread_factory = std::make_shared<Thread_pinning_factory>(config);
    }
    else
    {
        g_thread_factory = std::make_shared<Basic_thread_factory>();
    }
}

std::shared_ptr<IThread_factory> routing::get_thread_factory()
{
    return g_thread_factory;
}


