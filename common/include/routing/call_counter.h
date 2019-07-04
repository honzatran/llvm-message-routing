
#ifndef ROUTING_CALL_COUNTER_H
#define ROUTING_CALL_COUNTER_H

#include <routing/stdext.h>

#include <vector>

#include <gtest/gtest.h>
#include <spdlog/fmt/fmt.h>

namespace routing
{

namespace detail
{
class Called_exectly_base
{
public:
    Called_exectly_base(int required_invocation, std::string const& name)
        : m_required_invocation(required_invocation), m_name(name), m_called(0)
    {
    }

    Called_exectly_base(Called_exectly_base const& other) = delete;
    Called_exectly_base& operator=(Called_exectly_base const& other) = delete;

    Called_exectly_base(Called_exectly_base && other) = default;
    Called_exectly_base& operator=(Called_exectly_base && other) = default;

    ~Called_exectly_base()
    {
        EXPECT_EQ(m_called, m_required_invocation) << m_name;
    }

protected:
    int m_required_invocation;
    std::string m_name;
    int m_called;


    void on_called()
    {
        m_called++;
    }
};

struct Argument_matcher_base
{
    Argument_matcher_base(std::string const& name)
        : m_name(name)
    {
    }

    std::string m_name;
};

template <typename... ARGS>
class Argument_matcher : protected Argument_matcher_base
{
    static_assert(sizeof...(ARGS) > 0, "Cannot match on empty arguments");
public:
    Argument_matcher(std::string const& name)
        : Argument_matcher_base(name)
    {
    }

    ~Argument_matcher()
    {
        EXPECT_TRUE(m_arguments.empty()) << fmt::format(
            "Caller {} was not invoked with all predefined counters", m_name);
    }

    void expect(int required_call_count, ARGS... args)
    {
        m_arguments.push_back(
            Arguments(required_call_count, std::make_tuple(args...)));
    }

protected:

    void check_argument(std::tuple<ARGS...> const& tuple_args)
    {
        auto it = std::find_if(
            m_arguments.begin(),
            m_arguments.end(),
            [&tuple_args](Arguments const& arguments) {
                return tuple_args == arguments.m_parameter_value;
            });

        EXPECT_TRUE(it != m_arguments.end()) << fmt::format(
            "mock {} called with invalid arguments", m_name);

        if (it != m_arguments.end())
        {
            it->m_required_count--;

            if (it->m_required_count == 0)
            {
                m_arguments.erase(it);
            }
        }
    } 

private:

    struct Arguments
    {
        Arguments(int required_call_count, std::tuple<ARGS...> arguments)
            : m_required_count(required_call_count),
              m_parameter_value(arguments)
        {
        }

        int m_required_count;
        std::tuple<ARGS...> m_parameter_value;
    };

    std::vector<Arguments> m_arguments;
};

}

template <typename R, typename... ARGS>
class Called_exectly : protected detail::Called_exectly_base,
                       public detail::Argument_matcher<ARGS...>
{
public:
    Called_exectly(int required_invocation, std::string const& name)
        : Called_exectly_base(required_invocation, name),
          detail::Argument_matcher<ARGS...>(name)
    {
    }

    R operator() (ARGS... args)
    {
        on_called();
        check_argument(std::make_tuple(args...));
        return m_return_value;
    }

    int called() const
    {
        return m_called;
    }

private:
    R m_return_value;
};

template <typename... ARGS>
class Called_exectly_void : protected detail::Called_exectly_base,
                            public detail::Argument_matcher<ARGS...>
{
public:
    Called_exectly_void(int required_invocation, std::string const& name)
        : Called_exectly_base(required_invocation, name),
          detail::Argument_matcher<ARGS...>(name)
    {
    }

    void operator() (ARGS... args)
    {
        on_called();

        detail::Argument_matcher<ARGS...>::check_argument(
            std::make_tuple(args...));
    }

    int called() const
    {
        return m_called;
    }

private:
    std::vector<std::tuple<ARGS...>> m_arguments;
};

template <>
class Called_exectly_void<> : protected detail::Called_exectly_base
{
public:
    Called_exectly_void(int required_invocation, std::string const& name)
        : Called_exectly_base(required_invocation, name)
    {
    }

    void operator() ()
    {
        on_called();
    }

    int called() const
    {
        return m_called;
    }

private:
};

template <typename... ARGS>
Called_exectly_void<ARGS...>
define_void_mock(int count, std::string const& name)
{
    return Called_exectly_void<ARGS...>(count, name);
}

};

#endif
