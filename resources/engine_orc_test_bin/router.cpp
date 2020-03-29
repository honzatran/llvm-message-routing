

#include <routing/engine/annotation.h>
#include <routing/message/message.h>
#include <tuple>
#include "routing/stdext.h"


class ENGINE_ROUTER Automaton
{
public:
    Automaton() : m_val(42) { fmt::print("Creating automaton {}\n", m_val); }

    ENGINE_FACTORY
    static Automaton create();

    ENGINE_ENTRANCE
    void enter(routing::engine::Message const& m)
    {
        fmt::print("AAA message\n");

        ENGINE_OUTPUT_CODE("OUT", m);
    }

private:
    int m_val;
};

ENGINE_FUNCTION
void
create_test_automaton()
{
    fmt::print("AA\n");
}
