

#ifndef ROUTING_DECIMAL_H
#define ROUTING_DECIMAL_H

#include <istream>
#include <ostream>

#include <limits>

namespace routing
{

template <std::int64_t SCALE>
class Decimal
{
public:
    Decimal() = default;

    explicit Decimal(int64_t value) : m_value(value) {}

    std::int64_t to_int64() const { return m_value; }

    friend Decimal operator+(Decimal t1, Decimal t2)
    {
        return Decimal(t1.m_value + t2.m_value);
    }

    friend Decimal operator-(Decimal t1, Decimal t2)
    {
        return Decimal(t1.m_value - t2.m_value);
    }

    friend Decimal operator*(Decimal t1, Decimal t2)
    {
        return Decimal((t1.m_value / ((double)(SCALE))) * t2.m_value);
    }

    friend Decimal operator/(Decimal t1, Decimal t2)
    {
        return Decimal(t1.m_value / t2.m_value);
    }

    friend bool operator<(Decimal t1, Decimal t2)
    {
        return t1.m_value < t2.m_value;
    }

    friend bool operator<=(Decimal t1, Decimal t2)
    {
        return t1.m_value <= t2.m_value;
    }

    friend bool operator>=(Decimal t1, Decimal t2)
    {
        return t1.m_value >= t2.m_value;
    }

    friend bool operator>(Decimal t1, Decimal t2)
    {
        return t1.m_value > t2.m_value;
    }

    friend bool operator==(Decimal t1, Decimal t2)
    {
        return t1.m_value == t2.m_value;
    }

    friend Decimal& operator+=(Decimal& t1, Decimal t2)
    {
        t1.m_value += t2.m_value;

        return t1;
    }

    friend Decimal operator*(int quantity, Decimal t1)
    {
        return Decimal(quantity * t1.m_value);
    }

    friend inline Decimal
    abs(Decimal t)
    {
        return Decimal(std::abs(t.m_value));
    }

    friend std::ostream& operator<<(std::ostream& os, Decimal t)
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

    friend std::istream& operator>>(std::istream& is, Decimal& t)
    {
        float f;
        is >> f;

        t.m_value = f * SCALE;

        return is;
    }

    static Decimal from(std::int64_t decimal_value)
    {
        return Decimal(decimal_value);
    }

    static Decimal from_int64(std::int64_t value)
    {
        return Decimal(value* SCALE);
    }

    static Decimal max()
    {
        return Decimal(std::numeric_limits<std::int64_t>::max());
    }

    std::int64_t get_underlying_value() const { return m_value; }

private:
    std::int64_t m_value = 0;
};

template <std::int64_t SCALE>
Decimal<SCALE> convert_from(std::int64_t value)
{
    return Decimal<SCALE>(value * SCALE);
}

template <std::int64_t SCALE>
Decimal<SCALE> convert_from(double value)
{
    return Decimal<SCALE>(value * SCALE);
}


}  // namespace routing

#endif
