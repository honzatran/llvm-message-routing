

#ifndef ROUTING_DETECT_H
#define ROUTING_DETECT_H

#include <type_traits>

namespace routing
{

namespace detail
{
template <typename...>
struct make_void
{
    using type = void;
};
}

template <typename...ARGS>
using void_t = typename detail::make_void<ARGS...>::type;

struct nonesuch
{
    nonesuch() = delete;
    ~nonesuch() = delete;

    nonesuch(nonesuch const& other) = delete;
    void operator=(nonesuch const& other) = delete;
};

template <
    class DEFAULT, 
    class ALWAYS_VOID, 
    template <class...> class OP,
    class...ARGS>
struct DETECTOR 
{
    using value_t = std::false_type;
    using type = DEFAULT;
};

template <
    class DEFAULT, 
    template <class...> class OP,
    class... ARGS>
struct DETECTOR<DEFAULT, void_t<OP<ARGS...>>, OP, ARGS...>
{
    using value_t = std::true_type;
    using type = OP<ARGS...>;
};


template <template <class...> class OP, class...ARGS>
using is_detected = typename DETECTOR<
    nonesuch, 
    void, 
    OP,
    ARGS...>::value_t;

template <template <class...> class OP, class...ARGS>
using detected_t = typename DETECTOR<
    nonesuch, 
    void, 
    OP,
    ARGS...>::type;

template <class DEFAULT, template <class...> class OP, class ...ARGS>
using detected_or_t = typename DETECTOR<
    DEFAULT, 
    void, 
    OP,
    ARGS...>::type;


};


#endif
