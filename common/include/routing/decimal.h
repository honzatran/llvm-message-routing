

#ifndef ROUTING_DECIMAL_H
#define ROUTING_DECIMAL_H

#include <istream>
#include <ostream>

#include <limits>

namespace routing
{
template <std::int64_t SCALE>
class Generic_decimal
{
public:
    Generic_decimal() = default;

    explicit Generic_decimal(int64_t value) : m_value(value) {}

    std::int64_t to_int64() const { return m_value; }

    friend Generic_decimal operator+(Generic_decimal t1, Generic_decimal t2)
    {
        return Generic_decimal(t1.m_value + t2.m_value);
    }

    friend Generic_decimal operator-(Generic_decimal t1, Generic_decimal t2)
    {
        return Generic_decimal(t1.m_value - t2.m_value);
    }

    friend Generic_decimal operator*(Generic_decimal t1, Generic_decimal t2)
    {
        return Generic_decimal((t1.m_value / ((double)(SCALE))) * t2.m_value);
    }

    friend Generic_decimal operator/(Generic_decimal t1, Generic_decimal t2)
    {
        return Generic_decimal(t1.m_value / t2.m_value);
    }

    friend bool operator<(Generic_decimal t1, Generic_decimal t2)
    {
        return t1.m_value < t2.m_value;
    }

    friend bool operator<=(Generic_decimal t1, Generic_decimal t2)
    {
        return t1.m_value <= t2.m_value;
    }

    friend bool operator>=(Generic_decimal t1, Generic_decimal t2)
    {
        return t1.m_value >= t2.m_value;
    }

    friend bool operator>(Generic_decimal t1, Generic_decimal t2)
    {
        return t1.m_value > t2.m_value;
    }

    friend bool operator==(Generic_decimal t1, Generic_decimal t2)
    {
        return t1.m_value == t2.m_value;
    }

    friend Generic_decimal& operator+=(Generic_decimal& t1, Generic_decimal t2)
    {
        t1.m_value += t2.m_value;

        return t1;
    }

    friend Generic_decimal operator*(int quantity, Generic_decimal t1)
    {
        return Generic_decimal(quantity * t1.m_value);
    }

    friend inline Generic_decimal abs(Generic_decimal t)
    {
        return Generic_decimal(std::abs(t.m_value));
    }

    friend std::ostream& operator<<(std::ostream& os, Generic_decimal t)
    {
        double value           = (double)t.m_value / SCALE;
        std::int64_t int_value = static_cast<std::int64_t>(value);

        if (value == int_value)
        {
            os << int_value;
        }
        else
        {
            os << value;
        }

        return os;
    }

    friend std::istream& operator>>(std::istream& is, Generic_decimal& t)
    {
        float f;
        is >> f;

        t.m_value = f * SCALE;

        return is;
    }

    static Generic_decimal from(std::int64_t decimal_value)
    {
        return Generic_decimal(decimal_value);
    }

    static Generic_decimal from_int64(std::int64_t value)
    {
        return Generic_decimal(value * SCALE);
    }

    static Generic_decimal max()
    {
        return Generic_decimal(std::numeric_limits<std::int64_t>::max());
    }

    std::int64_t get_underlying_value() const { return m_value; }

private:
    std::int64_t m_value = 0;
};

template <std::int64_t SCALE>
Generic_decimal<SCALE>
convert_from(std::int64_t value)
{
    return Generic_decimal<SCALE>(value * SCALE);
}

template <std::int64_t SCALE>
Generic_decimal<SCALE>
convert_from(double value)
{
    return Generic_decimal<SCALE>(value * SCALE);
}

class Decimal : public Generic_decimal<100'000'000>
{
public:
    Decimal(std::int64_t value) : Generic_decimal<100'000'000>(value) {}

private:
};

}  // namespace routing

#endif
