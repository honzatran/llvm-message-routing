

#ifndef ROUTING_SPAN_H
#define ROUTING_SPAN_H

#define span_CONFIG_PROVIDE_WITH_CONTAINER_TO_STD 11
#define span_CONFIG_CONTRACT_VIOLATION_THROWS 1

#include <nonstd/span.hpp>


namespace routing
{
    using nonstd::span;
    using nonstd::operator==;
    using nonstd::operator!=;
    using nonstd::operator<;
    using nonstd::operator<=;
    using nonstd::operator>;
    using nonstd::operator>=;

    // using nonstd::span_lite::make_span;
}

#endif
