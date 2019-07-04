
#ifndef ROUTING_REMOTE_OPERATION_H
#define ROUTING_REMOTE_OPERATION_H

#include "buffer.h"

/**
 * Remote operation interface, works like RPC operation. This is used
 * mainly when we need to communicate between java and c using the 
 * language extensions and operation framework.
 */

namespace routing
{

class IOperation
{
public:
    virtual ~IOperation() = default;
    
    virtual int key() = 0;

    virtual Buffer execute(Buffer_view metadata) = 0;
};

}  // namespace routing

#endif
