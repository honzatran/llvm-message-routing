
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
    auto factory_symbol = m_jit_symbols->get_factory("Automaton");

    ASSERT_TRUE(factory_symbol);

    llvm::StringRef mangled_name(
        factory_symbol->get_mangled_symbol().begin(),
        factory_symbol->get_mangled_symbol().length());

    auto evaluated_factory = m_jit->lookup(mangled_name);

    if (auto err = evaluated_factory.takeError())
    {
        FAIL() << routing::engine::get_error_msg(std::move(err));
    }

    void (*factory)() = (void (*)(void))(evaluated_factory->getAddress());

    factory();

    fmt::print("AA\n");
}