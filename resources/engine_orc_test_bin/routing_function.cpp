

#include <routing/message/message.h>
#include <routing/engine/annotation.h>

ENGINE_FUNCTION
int route(routing::engine::Message& m)
{
    return m.get_int(42) + m.get_int(31) + m.get_int(20);
}
