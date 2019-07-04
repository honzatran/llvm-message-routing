

#include <gtest/gtest.h>

#include <routing/string_utils.h>

using namespace routing;
using namespace std;


TEST(Hungarian_transform_test, basic)
{
    ASSERT_EQ("My_type", routing::from_hungarian_notation("MyType"));
}

TEST(Hungarian_transform_test, mix_num)
{
    ASSERT_EQ("My_type1", routing::from_hungarian_notation("MyType1"));
}

TEST(Hungarian_transform_test, two_upper_next)
{
    ASSERT_EQ(
        "My_type1_id", routing::from_hungarian_notation("MyType1ID"));
}

TEST(Hungarian_transform_test, empty)
{
    ASSERT_EQ("", routing::from_hungarian_notation(std::string()));
}

TEST(Replacer_test, basic)
{
    String_replacer replacer("%TEST%_%TEST%_%PAT1%");

    replacer.replace_with("%TEST%", "AAA")
        .replace_with<int>("%PAT1%", 1);

    ASSERT_EQ("AAA_AAA_1", replacer.get_result());
}
