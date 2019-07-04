

#ifndef ROUTING_LATCHES_H
#define ROUTING_LATCHES_H

#include <mutex>
#include <condition_variable>

namespace routing
{

namespace detail
{

class Latch_base
{
public:
    virtual ~Latch_base() = default;
    virtual bool can_return() = 0;

    template <typename REP, typename PERIOD>
    std::cv_status wait_for(std::chrono::duration<REP, PERIOD> const& rel_time)
    {
        std::unique_lock<std::mutex> lock(m_mtx);

        if (can_return())
        {
            return std::cv_status::no_timeout;
        }

        return m_cv.wait_for(lock, rel_time);
    }

    void wait()
    {
        std::unique_lock<std::mutex> lock(m_mtx);

        if (can_return())
        {
            return;
        }

        m_cv.wait(lock);
    }

protected:
    std::mutex m_mtx;
    std::condition_variable m_cv;
};

}

class Latch final : public detail::Latch_base
{
public:
    Latch(std::size_t count) 
        : m_count(count)
    {
    }

    void count_down()
    {
        {
            std::lock_guard<std::mutex> guard(m_mtx);

            if (m_count == 0)
            {
                return;
            }

            m_count--;
        }

        if (m_count == 0)
        {
            m_cv.notify_all();
        }
    }

    bool can_return() override
    {
        return m_count == 0;
    }

    std::size_t get_count() const
    {
        return m_count;
    }

private:
    std::size_t m_count;
};

class Event_latch final : public detail::Latch_base
{
public:
    Event_latch() = default;
         
    void signal()
    {
        {
            std::lock_guard<std::mutex> guard(m_mtx);

            if (m_signaled)
            {
                return;
            }

            m_signaled = true;
        }
        
        m_cv.notify_all();
    }

    bool can_return() override
    {
        return m_signaled;
    }

private:
    bool m_signaled{false};
};

}

#endif
