

#include <routing/engine/annotation.h>
#include <routing/message/message.h>

class ENGINE_ROUTER Automaton
{
public:
    Automaton() : m_val(42) {}

    ENGINE_FACTORY
    static Automaton create();

    ENGINE_ENTRANCE
    void enter(routing::engine::Message &m);

private:
    int m_val;
};

ENGINE_FACTORY
Automaton
create_test_automaton()
{
    fmt::print("Creating automaton 222\n");

    return Automaton();
}
