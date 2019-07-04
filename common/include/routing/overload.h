

#ifndef ROUTING_OVERLOAD_H
#define ROUTING_OVERLOAD_H

#include <boost/variant/static_visitor.hpp>

namespace routing
{
template <typename...>
struct Overload;

template <typename R, typename HEAD_LAMBDA, typename... TAIL_LAMBDAS>
struct Overload<R, HEAD_LAMBDA, TAIL_LAMBDAS...>
    : public HEAD_LAMBDA, public Overload<R, TAIL_LAMBDAS...>
{
    using HEAD_LAMBDA::operator();
    using Overload<R, TAIL_LAMBDAS...>::operator();

    Overload(HEAD_LAMBDA head_lambda, TAIL_LAMBDAS... tail_lambdas)
        : HEAD_LAMBDA(head_lambda),
          Overload<R, TAIL_LAMBDAS...>(tail_lambdas...)
    {
    }
};

template <typename R, typename LAMBDA>
struct Overload<R, LAMBDA> : public LAMBDA, public boost::static_visitor<R>
{
    using LAMBDA::operator();

    Overload(LAMBDA lambda) : LAMBDA(lambda), boost::static_visitor<R>() {}
};

template <typename R>
struct Overload<R> : public boost::static_visitor<R>
{
    Overload() : boost::static_visitor<R>() {}
};

template <typename R, typename... LAMBDAS>
Overload<R, LAMBDAS...> overload(LAMBDAS... lambdas)
{
    return Overload<R, LAMBDAS...>(lambdas...);
}
};

#endif
