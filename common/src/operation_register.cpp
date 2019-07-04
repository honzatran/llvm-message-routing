

#include <routing/operation_register.h>

using namespace routing;

void
Operation_register::add(std::shared_ptr<IOperation> const& operation)
{
    m_operations.insert(std::make_pair(operation->key(), operation));
}

boost::optional<Buffer>
Operation_register::execute(int operation_id, Buffer_view metadata)
{
    auto it = m_operations.find(operation_id);

    if (it != m_operations.end())
    {
        return it->second->execute(metadata);
    }

    return {};
}

