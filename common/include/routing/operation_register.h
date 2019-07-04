
#ifndef ROUTING_OPERATION_REGISTER_H
#define ROUTING_OPERATION_REGISTER_H

#include <routing/operation.h>

#include <unordered_map>
#include <memory>
#include <boost/optional.hpp>

namespace routing
{

class Operation_register
{
public:

    void add(std::shared_ptr<IOperation> const& operation);

    boost::optional<Buffer> execute(
        int operation_id,
        Buffer_view metadata);

private:
    std::unordered_map<int, std::shared_ptr<IOperation>> m_operations;
};

}  // namespace routing

#endif
