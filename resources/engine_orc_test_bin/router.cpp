

#include <routing/message/message.h>
#include <routing/engine/annotation.h>


class ENGINE_ROUTER Automaton 
{
public:

    ENGINE_FACTORY
    static Automaton create();

    ENGINE_ENTRANCE
    void enter(routing::engine::Message &m);

private:
};
