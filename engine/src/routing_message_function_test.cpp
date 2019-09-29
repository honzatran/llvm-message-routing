
#include <gtest/gtest.h>

#include <routing/message/message.h>

#include "base_engine_test.h"
#include "routing/slab_allocator.h"
#include "routing/stdext.h"

using namespace routing;

class Routing_message_function_tests
    : public routing::engine::Base_engine_function_test
{
public:
    Routing_message_function_tests()
        : routing::engine::Base_engine_function_test("routing_function.cpp")
    {
    }
};

TEST_F(Routing_message_function_tests, route)
{
    auto route_message_function = m_jit_symbols->get_symbols("route");

    ASSERT_EQ(1, route_message_function.size());

    auto mangled_name = route_message_function[0].get_mangled_symbol();
    llvm::StringRef symbol_name(&mangled_name[0], mangled_name.size());

    auto external_bits_function = m_jit->lookup(symbol_name);

    if (auto err = external_bits_function.takeError())
    {
        FAIL() << routing::engine::get_error_msg(std::move(err));
    }


    auto slab_allocator
        = std::make_unique<Slab_allocator_t>(Malloc_allocator());
    routing::engine::Message message(std::move(slab_allocator));

    message.set_int(42, 1);
    message.set_int(31, 2);
    message.set_int(20, 3);

    int (*route_function)(routing::engine::Message &) = (int (*)(
        routing::engine::Message &))(external_bits_function->getAddress());

    ASSERT_EQ(6, route_function(message));
}
