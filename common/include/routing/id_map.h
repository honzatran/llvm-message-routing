

#ifndef ROUTING_ID_MAP_H
#define ROUTING_ID_MAP_H

#include <unordered_map>
#include <type_traits>

#include <boost/optional.hpp>

namespace routing
{

template <typename ID_TYPE, typename MAP_TYPE>
class Id_map
{
    static_assert(
        std::is_integral<MAP_TYPE>::value,
        "MAP TYPE must be integral");

public:
    MAP_TYPE get_or_create_id(ID_TYPE const& id)
    {
        auto it = m_id_map.find(id);

        if (it != m_id_map.end())
        {
            return it->second;
        }

        MAP_TYPE value = m_counter++;

        m_id_map.insert(std::make_pair(id, value));

        return value;
    }

    boost::optional<MAP_TYPE> get_id(ID_TYPE const& id)
    {
        auto it = m_id_map.find(id);

        if (it != m_id_map.end())
        {
            return it->second;
        }

        return {};
    }

private:
    std::unordered_map<MAP_TYPE, ID_TYPE> m_id_map;

    MAP_TYPE m_counter = 0;
};


}

#endif
