

#include <gtest/gtest.h>
#include <routing/type_traits.h>
#include <routing/stdext.h>

#include <iostream>

using namespace routing;

namespace
{
template <typename T>
enable_if_t<Is_printable<T>::value, bool>
can_be_printed()
{
    return true;
}

template <typename T>
enable_if_t<!Is_printable<T>::value, bool>
can_be_printed()
{
    return false;
}

class Not_printable
{
};

class Printable
{
public:
    friend std::ostream& operator<<(
        std::ostream& os,
        Printable const& printable)
    {
        return os;
    }
};
}

TEST(Is_printable_test, int_is_printable)
{
    ASSERT_TRUE(can_be_printed<int>());
}

TEST(Is_printable_test, not_printable)
{
    ASSERT_FALSE(can_be_printed<::Not_printable>());
}

TEST(Is_printable_test, printable)
{
    ASSERT_TRUE(can_be_printed<::Printable>());

    std::cout << Printable();
}

TEST(Is_printable_test, do_print_printable)
{
    ASSERT_EQ("1", to_string_or_not_printable(1));
}

TEST(Is_printable_test, do_print_notprintable)
{
    ASSERT_EQ("NOT PRINTABLE", to_string_or_not_printable(Not_printable()));
}

// TEST(Is_printable_test, do_print_not_printable)
// {
//     bool called = false;
//
//     do_print(
//         [](::Not_printable a) { fmt::format("{}", a); FAIL(); },
//         [&called](::Not_printable a) { called = true; },
//         ::Not_printable());
//
//     ASSERT_TRUE(called);
// }

