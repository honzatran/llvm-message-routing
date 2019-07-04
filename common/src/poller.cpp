

#include <routing/poller.h>

using namespace routing;

void
detail::Poller_base::start(IThread_factory* thread_factory)
{
    std::lock_guard<std::mutex> lock_guard(m_mutex);

    bool stopped = false;
    if (!m_running.compare_exchange_strong(stopped, true))
    {
        return;
    }

    if (m_poller_thread.joinable())
    {
        m_poller_thread.join();
    }

    m_poller_thread
        = thread_factory->create(m_name, get_run_loop());
}

void
detail::Poller_base::stop()
{
    std::lock_guard<std::mutex> lock_guard(m_mutex);

    bool running = true;
    if (!m_running.compare_exchange_strong(running, false))
    {
        return;
    }

    if (m_poller_thread.joinable())
    {
        m_poller_thread.join();
    }
}

