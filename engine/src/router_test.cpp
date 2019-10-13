
#include "base_engine_test.h"

class Router_test : public routing::engine::Base_engine_function_test
{
public:
    Router_test() : routing::engine::Base_engine_function_test("router.cpp") {}
};

TEST_F(Router_test, router_class_detected)
{
    ASSERT_TRUE(m_jit_symbols->contains_router("Automaton"));
}


TEST_F(Router_test, entrance_method_detected)
{
    ASSERT_TRUE(m_jit_symbols->get_entrance("Automaton"));
}

TEST_F(Router_test, factory_method_detected)
{
    ASSERT_TRUE(m_jit_symbols->get_factory("Automaton"));
}
