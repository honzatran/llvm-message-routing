
#ifndef ROUTING_STDEXT_H
#define ROUTING_STDEXT_H

#include <type_traits>
#include <utility>
#include <memory>

#include <stdint.h>

namespace routing
{

template <typename Fn>
class Function_ref;

template <typename RET, typename... PARAMS>
class Function_ref<RET(PARAMS...)>
{
private:
    RET (*m_callback)(intptr_t callable, PARAMS... params) = nullptr;
    intptr_t m_callable;

    template <typename CALLABLE>
    static RET callback_fn(intptr_t callable, PARAMS... params)
    {
        return (*reinterpret_cast<CALLABLE*>(callable))(
            std::forward<PARAMS>(params)...);
    }

public:
    Function_ref() = default;

    template <typename CALLABLE>
    Function_ref(
        CALLABLE&& callable,
        typename std::enable_if<!std::is_same<
            typename std::remove_reference<CALLABLE>::type,
            Function_ref>::value>::type*  /*unused*/= nullptr)
        : m_callback(
              callback_fn<typename std::remove_reference<CALLABLE>::type>),
          m_callable(reinterpret_cast<intptr_t>(&callable))
    {
    }

    RET operator()(PARAMS... params) const
    {
        return m_callback(m_callable, std::forward<PARAMS>(params)...);
    }

    operator bool() const { return m_callback; }
};

template <typename T, typename ...ARGS>
std::unique_ptr<T> 
make_unique(ARGS&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<ARGS>(args)...));
}


template <bool B, typename T = void>
using enable_if_t = typename std::enable_if<B, T>::type;

template <typename T>
using decay_t = typename std::decay<T>::type;

}

#endif
