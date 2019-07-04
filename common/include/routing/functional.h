

#ifndef ROUTING_FUNCTIONAL_H
#define ROUTING_FUNCTIONAL_H

#include <mutex>
#include "stdext.h"

namespace routing
{
template <typename... ARGS>
class Call_once
{
public:
    Call_once(Function_ref<void(ARGS...)> function_ref)
        : m_function_ref(function_ref)
    {
    }

    void operator() (ARGS... args)
    {
        if (m_function_ref)
        {
            std::call_once(m_flag, m_function_ref, args...);
        }
    }

private:
    std::once_flag m_flag;
    Function_ref<void(ARGS...)> m_function_ref;
};


};

#endif
