

#include <gtest/gtest.h>
#include <routing/matcher.h>
#include <routing/fmt.h>

using namespace routing;

TEST(Sequence_matcher_test, sequence)
{
    Sequence_matcher<void(int)> matcher;

    matcher.next()
        .named("TEST")
        .called(2)
        .set_matcher([](int i) { EXPECT_EQ(10, i); });

    matcher(10);
    matcher(10);
}

TEST(Sequence_matcher_test, two_expected_call)
{
    Sequence_matcher<void(int)> matcher;

    matcher.next()
        .named("TEST")
        .set_matcher([](int i) { EXPECT_EQ(10, i); });

    matcher.next()
        .named("TEST1")
        .set_matcher([](int i) { EXPECT_EQ(15, i); });

    matcher(10);
    matcher(15);
}

TEST(Sequence_matcher_test, move)
{
    Sequence_matcher<void(int)> matcher;

    matcher.next()
        .called(2)
        .named("TEST")
        .set_matcher([](int i) { EXPECT_EQ(10, i); });

    matcher.next()
        .named("TEST1")
        .set_matcher([](int i) { EXPECT_EQ(15, i); });

    matcher(10);

    Sequence_matcher<void(int)> move_matcher(std::move(matcher));

    move_matcher(10);
    move_matcher(15);
}

TEST(Sequence_matcher_test, move_assign)
{
    Sequence_matcher<void(int)> matcher;

    matcher.next()
        .called(2)
        .named("TEST")
        .set_matcher([](int i) { EXPECT_EQ(10, i); });

    matcher.next()
        .named("TEST1")
        .set_matcher([](int i) { EXPECT_EQ(15, i); });

    matcher(10);

    Sequence_matcher<void(int)> move_matcher = std::move(matcher);

    move_matcher(10);
    move_matcher(15);
}

TEST(Sequence_matcher_test, sequence_return)
{
    Sequence_matcher<int(int)> matcher;

    matcher.return_function([](int i) { return 42; })
        .next()
        .named("TEST")
        .called(1)
        .set_matcher([](int i) { EXPECT_EQ(10, i); });

    ASSERT_EQ(42, matcher(10));
}

TEST(Set_matcher_test, set)
{
    Set_matcher<void(int)> matcher;

    matcher.expect()
        .filter([](int i) { return i == 10; })
        .named("TEST")
        .called(1)
        .set_matcher( [](int i) { ASSERT_EQ(10, i); });

    matcher.expect()
        .filter([] (int i) { return i == 15; })
        .named("TEST1")
        .called(1)
        .set_matcher([](int i) { ASSERT_EQ(15, i); });

    matcher(10);
    matcher(15);
}

TEST(Set_matcher_test, move)
{
    Set_matcher<void(int)> matcher;

    matcher.expect()
        .filter([](int i) { return i == 10; })
        .named("TEST")
        .called(2)
        .set_matcher( [](int i) { ASSERT_EQ(10, i); });

    matcher.expect()
        .filter([] (int i) { return i == 15; })
        .named("TEST1")
        .called(1)
        .set_matcher([](int i) { ASSERT_EQ(15, i); });

    matcher(10);

    Set_matcher<void(int)> move_matcher(std::move(matcher));

    move_matcher(15);
    move_matcher(10);
}

TEST(Set_matcher_test, move_assign)
{
    Set_matcher<void(int)> matcher;

    matcher.expect()
        .filter([](int i) { return i == 10; })
        .named("TEST")
        .called(2)
        .set_matcher( [](int i) { ASSERT_EQ(10, i); });

    matcher.expect()
        .filter([] (int i) { return i == 15; })
        .named("TEST1")
        .called(1)
        .set_matcher([](int i) { ASSERT_EQ(15, i); });

    matcher(10);

    Set_matcher<void(int)> move_matcher = std::move(matcher);

    move_matcher(15);
    move_matcher(10);
}

