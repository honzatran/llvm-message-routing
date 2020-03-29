
#include "base_engine_test.h"
#include "routing/message/message.h"

class Router_test : public routing::engine::Base_engine_full_compile_test
{
public:
    Router_test() : routing::engine::Base_engine_full_compile_test("router.cpp")
    {
    }
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
    auto factory_symbol = m_jit_symbols->get_symbols("enter");

    auto output_setter_symbol = m_jit_symbols->get_symbols("set_OUT");

    // ASSERT_TRUE(factory_symbol);

    // llvm::StringRef mangled_name(
    //     factory_symbol->get_mangled_symbol().begin(),
    //     factory_symbol->get_mangled_symbol().length());
    //
    auto mangled_name = factory_symbol[0].get_mangled_symbol();

    llvm::StringRef symbol_name(&mangled_name[0], mangled_name.size());
    auto evaluated_factory = m_jit->lookup(symbol_name);

    auto output_mangled_name = output_setter_symbol[0].get_mangled_symbol();
    llvm::StringRef output_symbol_name(
        &output_mangled_name[0], output_mangled_name.size());
    auto output_setter = m_jit->lookup(output_symbol_name);

    if (auto err = evaluated_factory.takeError())
    {
        FAIL() << routing::engine::get_error_msg(std::move(err));
    }

    if (auto err = output_setter.takeError())
    {
        FAIL() << routing::engine::get_error_msg(std::move(err));
    }

    void (*factory)(routing::engine::Message &) = (void (*)(
        routing::engine::Message &))(evaluated_factory->getAddress());

    void (*output_setter_ref)(
        routing::Function_ref<void(routing::engine::Message &)>)
        = (void (*)(
            routing::Function_ref<void(routing::engine::Message &)>))(
            output_setter->getAddress());

    auto tmp
        = [](routing::engine::Message &m) -> void { fmt::print("OUTPUT\n"); };

    routing::Function_ref<void(routing::engine::Message &)> tmp_fn = tmp;

    output_setter_ref(tmp_fn);

    routing::engine::Message m;
    factory(m);

    auto tmp1
        = [](routing::engine::Message &m) -> void { fmt::print("OUTPUT1\n"); };

    output_setter_ref(tmp1);
    factory(m);
}
