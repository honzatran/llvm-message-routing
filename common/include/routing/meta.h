

#ifndef ROUTING_META_H
#define ROUTING_META_H

#include <tuple>

namespace routing
{
template <typename...>
struct is_one_of;

template <typename F>
struct is_one_of<F>
{
    static constexpr bool value = false;
};

template <typename F, typename HEAD, typename... ARGS>
struct is_one_of<F, HEAD, ARGS...>
{
    static constexpr bool value
        = std::is_same<F, HEAD>::value || is_one_of<F, ARGS...>::value;
};

template <typename...>
struct is_unique;

template <>
struct is_unique<>
{
    static constexpr bool value = true;
};

template <typename HEAD, typename... ARGS>
struct is_unique<HEAD, ARGS...>
{
    static constexpr bool value
        = is_unique<ARGS...>::value && !is_one_of<HEAD, ARGS...>::value;
};

template <typename...>
struct cons;

template <typename T, typename... ARGS>
struct cons<T, std::tuple<ARGS...>>
{
    using type = std::tuple<T, ARGS...>;
};

template <template <typename> class PRED, typename...>
struct filter;

template <template <typename> class PRED>
struct filter<PRED>
{
    using type = std::tuple<>;
};

template <template <typename> class PRED, typename HEAD, typename... REST>
struct filter<PRED, HEAD, REST...>
{
    using type = typename std::
        conditional<PRED<HEAD>::value,
                    typename cons<HEAD,
                                  typename filter<PRED, REST...>::type>::type,
                    typename filter<PRED, REST...>::type>::type;
};

template <typename T, std::size_t N, typename... ARGS>
struct index_of
{
    static constexpr std::size_t value = N;
};

template <typename T, std::size_t N, typename... ARGS>
struct index_of<T, N, T, ARGS...>
{
    static constexpr std::size_t value = N;
};

template <typename T, std::size_t N, typename U, typename... ARGS>
struct index_of<T, N, U, ARGS...>
{
    static constexpr std::size_t value = index_of<T, N + 1, ARGS...>::value;
};

template <template <typename...> class T, typename...>
struct apply_types;

template <template <typename...> class T, typename... ARGS>
struct apply_types<T, std::tuple<ARGS...>>
{
    using type = T<ARGS...>;
};

using expander = int[];

#define EXPAND_FUNCTION(F) (void)expander{0, (F, 0)... };

// template <std::size_t.. Indices>
// struct index_sequence {};
//
// template <std::size_t N>
// using get_index_sequence =
}

#endif
