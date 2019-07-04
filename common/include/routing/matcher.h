
#ifndef ROUTING_MATCHER_H
#define ROUTING_MATCHER_H

#include <gtest/gtest.h>
#include <routing/fmt.h>
#include <routing/logger.h>
#include <routing/stdext.h>
#include <routing/type_traits.h>

#include <algorithm>

/// use this classes only with google test
/// look at the matcher_test.cpp for examples of usage

namespace routing
{

namespace detail
{
class Expected_call_base
{
public:
    Expected_call_base(std::string const& name, int call)
        : m_name(name), m_expected_call_count(call), m_called(0)
    {
    }

    int get_called() const
    {
        return m_called;
    }

    int get_expected_call_count() const
    {
        return m_expected_call_count;
    }

    std::string const& get_name() const
    {
        return m_name;
    }

    bool had_test_error() const
    {
        return m_test_error;
    }

protected:
    std::string m_name;

    int m_expected_call_count;
    int m_called;
    bool m_test_error = false;
};


inline void check(Expected_call_base const& expected_call)
{
    EXPECT_FALSE(expected_call.had_test_error()) << fmt::format(
        "Expected call {} had an test error", expected_call.get_name());

    EXPECT_EQ(
        expected_call.get_expected_call_count(), expected_call.get_called())
        << fmt::format(
               "Expected call {} was called {} times but was supposed "
               "to be called {} times.",
               expected_call.get_name(),
               expected_call.get_called(),
               expected_call.get_expected_call_count());
}
}

template <typename T>
class Expected_call : public detail::Expected_call_base
{
public:
    Expected_call(
        std::string const& name,
        int called,
        std::function<void(T)> function)
        : Expected_call_base(name, called), m_function(function)
    {
        m_logger = routing::get_default_logger(
            fmt::format("Expected_call_{}", name));
    }

    void on_called(T arg)
    {
        if (!m_test_error)
        {
            m_function(arg);

            if (::testing::Test::HasFailure())
            {
                log_error(arg);
                m_test_error = true;
            }

            m_called++;
        }
    }

private:
    Logger_t m_logger;
    std::function<void(T)> m_function;

    void log_error(T instance)
    {
        m_logger->error(
            "FAILURE expected call failed for instance {}", 
            to_string_or_not_printable(instance));
    }

};

template <typename T>
class Sequence_matcher;

template <typename ARG>
class Filter_expected_call_builder;

template <typename ARG>
class Expected_call_builder
{
public:
    Expected_call_builder& named(std::string const& name)
    {
        m_name = name;
        return *this;
    }

    Expected_call_builder& called(int called)
    {
        m_expected_call_count = called;
        return *this;
    }

    void set_matcher(std::function<void(ARG)> const& function)
    {
        m_function = function;
        m_on_finished(*this);
    }

private:
    template <typename R>
    friend class Sequence_matcher;

    template <typename T>
    friend class Filter_expected_call_builder;

    std::string m_name = "";
    int m_expected_call_count = 1;

    std::function<void(ARG)> m_function;

    std::function<void(Expected_call_builder<ARG>&)> m_on_finished;

    Expected_call_builder(
        std::function<void(Expected_call_builder<ARG>&)> const on_finished)
        : m_on_finished(on_finished)
    {
    }
};

/// Sequence matcher is used to match sequence of calls to a 
/// function object.
template <typename R, typename ARG>
class Sequence_matcher<R(ARG)>
{
public:
    Sequence_matcher() = default;

    Sequence_matcher(Sequence_matcher const& other) = delete;
    Sequence_matcher& operator=(Sequence_matcher const& other) = delete;

    Sequence_matcher(Sequence_matcher&& other)
        : m_check_validation(other.m_check_validation),
          m_next(other.m_next),
          m_validators(std::move(other.m_validators)),
          m_return_function(std::move(other.m_return_function))
    {
        if (other.m_check_validation)
        {
            other.m_check_validation = false;
        }
    }

    Sequence_matcher& operator=(Sequence_matcher&& other) 
    {
        if (&other != this)
        {
            m_check_validation = other.m_check_validation;
            m_next             = other.m_next;
            m_validators       = std::move(other.m_validators);
            m_return_function  = std::move(other.m_return_function);

            other.m_check_validation = false;
        }

        return *this;
    }

    ~Sequence_matcher()
    {
        if (m_check_validation)
        {
            for (const auto& expected_call : m_validators)
            {
                detail::check(expected_call);
            }
        }
    }

    Sequence_matcher& return_function(
        std::function<R(ARG)> const& return_function)
    {
        m_return_function = return_function;
        return *this;
    }

    R operator() (ARG arg)
    {
        if (!m_check_validation)
        {
            return call_return_function(arg);
        }

        if (m_next >= m_validators.size())
        {
            // check this again to get nice error message
            EXPECT_LT(m_next, m_validators.size()) << fmt::format(
                "No expected call defined for arg {}",
                to_string_or_not_printable(arg));

            return call_return_function(arg);
        }

        auto& expected_call = m_validators[m_next];

        expected_call.on_called(arg);

        if (expected_call.get_called()
            >= expected_call.get_expected_call_count())
        {
            EXPECT_LT(m_next, m_validators.size()) << fmt::format(
                "No expected call defined for this arg {}",
                to_string_or_not_printable(arg));

            if (m_next < m_validators.size())
            {
                m_next++;
            }
        }

        return call_return_function(arg);
    }

    Expected_call_builder<ARG> next()
    {
        auto on_finished =
            [this](Expected_call_builder<ARG>& builder) {
                this->m_validators.emplace_back(
                    builder.m_name,
                    builder.m_expected_call_count,
                    builder.m_function);
            };

        return Expected_call_builder<ARG>(on_finished);
    }

private:
    bool m_check_validation = true;
    std::size_t m_next = 0;

    std::vector<Expected_call<ARG>> m_validators;
    std::function<R(ARG)> m_return_function;

    R call_return_function(ARG arg)
    {
        if (m_return_function)
        {
            return m_return_function(arg);
        }
        else 
        {
            return R();
        }
    }
};

template <typename T>
class Filter_expected_call : public detail::Expected_call_base
{
public:
    Filter_expected_call(
        std::string const& name,
        int count,
        std::function<bool(T)> const& filter,
        std::function<void(T)> const& function)
        : detail::Expected_call_base(name, count),
          m_filter(filter),
          m_function(function)
    {
        m_logger = routing::get_default_logger(
            fmt::format("Filter_expected_call_{}", name));
    }

    bool try_match(T arg) 
    {
        if (m_filter(arg))
        {
            if (!m_test_error)
            {
                m_function(arg);

                if (::testing::Test::HasFailure())
                {
                    m_test_error = true;
                    log_error(arg);
                }

                m_called++;
            }

            return true;
        }

        return false;
    }

private:
    Logger_t m_logger;
    std::function<bool(T)> m_filter;
    std::function<void(T)> m_function;

    void log_error(T arg)
    {
        m_logger->error(
            "Filter has failed for instance {}",
            to_string_or_not_printable(arg));
    }
};

template <typename ARG>
class Filter_expected_call_builder
{
public:
    Filter_expected_call_builder(
        std::vector<Filter_expected_call<ARG>>& validators)
        : m_validators(validators)
    {
    }

    Expected_call_builder<ARG> filter(std::function<bool(ARG)> filter)
    {
        m_filter = filter;

        auto on_finished = [this](Expected_call_builder<ARG>& builder) {
            m_validators.emplace_back(
                builder.m_name,
                builder.m_expected_call_count,
                m_filter,
                builder.m_function);
        };

        return Expected_call_builder<ARG>(on_finished);
    }

private:
    template <typename U>
    friend class Set_matcher;

    std::function<bool(ARG)> m_filter;

    std::vector<Filter_expected_call<ARG>>& m_validators;
};

template <typename T>
class Set_matcher;

/// Set matcher is used to match calls which can match
/// to multiple predefined calls. There is no ordering
/// for the calls.
template <typename R, typename ARG>
class Set_matcher<R(ARG)>
{
public:
    Set_matcher() = default;

    Set_matcher(Set_matcher const& other) = delete;
    Set_matcher& operator=(Set_matcher const& other) = delete;

    Set_matcher(Set_matcher&& other)
        : m_validators(std::move(other.m_validators)),
          m_check_validation(other.m_check_validation)
    {
        other.m_check_validation = false;
    }

    Set_matcher& operator=(Set_matcher&& other) 
    {
        if (&other != this)
        {
            m_validators = std::move(other.m_validators);
            m_check_validation = other.m_check_validation;

            other.m_check_validation = false;
        }

        return *this;
    }

    ~Set_matcher()
    {
        if (m_check_validation)
        {
            for (const auto& expected_call : m_validators)
            {
                detail::check(expected_call);
            }
        }
    }

    R operator() (ARG arg)
    {
        if (!m_check_validation)
        {
            return R();
        }

        auto it = std::find_if(m_validators.begin(), m_validators.end(), 
                [arg] (Filter_expected_call<ARG>& filter_matcher)
                {
                    return filter_matcher.try_match(arg);
                });

        EXPECT_NE(it, m_validators.end()) << fmt::format(
            "No filter matcher defined for {}",
            to_string_or_not_printable(arg));

        return R();
    }

    Filter_expected_call_builder<ARG> expect()
    {
        return Filter_expected_call_builder<ARG>(m_validators);
    }

private:
    std::vector<Filter_expected_call<ARG>> m_validators;
    bool m_check_validation = true;
};

}

#endif
