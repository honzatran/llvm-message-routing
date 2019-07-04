

#ifndef ROUTING_SORTED_SMALL_ARRAY_H
#define ROUTING_SORTED_SMALL_ARRAY_H

namespace routing
{

template <typename T, std::size_t CAPACITY>
class Sorted_small_array
{
public:
    constexpr static std::size_t capacity()
    {
        return CAPACITY;
    }

    Sorted_small_array(T min)
    {
        for (int i = 0; i < CAPACITY; ++i)
        {
            m_array[i] = min;
        }
    }

    T const& operator[] (std::size_t index) const
    {
        return m_array[index];
    }

    T& operator[] (std::size_t index) 
    {
        return m_array[index];
    }

    T max() const
    {
        return m_array[CAPACITY - 1];
    }

    T min() const
    {
        return m_array[0];
    }

    std::size_t lower_bound(T element) const
    {
        for (std::size_t i = 0; i < CAPACITY; i++)
        {
            if (element <= m_array[i])
            {
                return i;
            }
        }

        return CAPACITY;
    }

    std::size_t upper_bound(T element) const
    {
        for (std::size_t i = 0; i < CAPACITY; i++)
        {
            if (element < m_array[i])
            {
                return i;
            }
        }

        return CAPACITY;
    }

    T shift_and_insert(T elem)
    {
        T tmp[CAPACITY];

        T shifted_away = m_array[0];

        for (std::size_t i = 0; i < CAPACITY; ++i)
        {
            tmp[i] = m_array[i];
        }

        for (std::size_t i = 0; i < CAPACITY - 1; ++i)
        {
            m_array[i] = tmp[i + 1];
        }

        m_array[CAPACITY - 1] = elem;

        return shifted_away;
    }

private:
    T m_array[CAPACITY];
    std::size_t m_size;
};

};

#endif
