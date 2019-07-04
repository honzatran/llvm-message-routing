

#include <routing/overload.h>
#include <routing/call_counter.h>
#include <routing/testing.h>
#include <boost/variant.hpp>

#include <gtest/gtest.h>

using namespace routing;
using namespace std;

TEST(Variant_overload_test, test)
{
    boost::variant<int, double, std::string> variant;

    variant = 5;

    auto int_update_called_once
        = Called_exectly_void<>(1, DEFINED("Int_visited"));

    auto visitor = overload<void>(
        [&int_update_called_once](int i) {
            EXPECT_EQ(5, i);
            int_update_called_once();
        },
        [](double d) { FAIL(); },
        [](std::string const& s) { FAIL(); });

    boost::apply_visitor(visitor, variant);

    auto string_update_called_once
        = Called_exectly_void<>(1, DEFINED("String_visited"));

    variant = "AAA";

    auto string_visitor = overload<void>(
        [](int i) { FAIL(); },
        [](double d) { FAIL(); },
        [&string_update_called_once](std::string const& s) {
            EXPECT_EQ(s, "AAA");
            string_update_called_once();
        });

    boost::apply_visitor(string_visitor, variant);
}
