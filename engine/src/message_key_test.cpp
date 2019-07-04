

#include <emmintrin.h>
#include <gtest/gtest-param-test.h>
#include <gtest/gtest.h>

#include <routing/engine/message_key.h>
#include <routing/fmt.h>

using namespace routing::engine;

namespace
{

class Combined_message_key_test : public ::testing::TestWithParam<Field_type>
{
};
}

TEST_P(Combined_message_key_test, basic_initilization)
{
    int const test_key = 42;
    
    Combined_message_key key(test_key, Field_type::MESSAGE_ARRAY);

    ASSERT_EQ(test_key, key.key());
    ASSERT_EQ(Field_type::MESSAGE_ARRAY, key.field_type());
}

INSTANTIATE_TEST_SUITE_P(
    Type_keys,
    Combined_message_key_test,
    ::testing::Values(
        Field_type::INT,
        Field_type::LONG,
        Field_type::DOUBLE,
        Field_type::DECIMAL,
        Field_type::TIME,
        Field_type::DATE,
        Field_type::STRING,
        Field_type::CUSTOM_DATA,
        Field_type::MESSAGE_ARRAY));


TEST(Message_group_value_types_test, set)
{
    Message_value_group_types types;

    types.set_type(1, Field_type::INT);

    ASSERT_TRUE(types.contains(Field_type::INT));
}

TEST(Message_group_value_types_test, mask_for_set_type)
{
    Message_value_group_types types;

    types.set_type(1, Field_type::INT);
    types.set_type(9, Field_type::INT);

    auto mask = types.get_mask(Field_type::INT);

    ASSERT_EQ((1 << 1) | (1 << 9), mask);
}

TEST(Message_group_value_types_test, mask_for_non_set_type)
{
    Message_value_group_types types;

    auto mask = types.get_mask(Field_type::INT);

    ASSERT_EQ(0, mask);
}

TEST(Message_group_value_keys_test, set)
{
    Message_value_group_keys keys;
    ASSERT_TRUE(keys.all_zeros());

    ASSERT_EQ(-1, keys.contains(42));

    keys.set_key(7, 42);


    ASSERT_EQ(7, keys.contains(42));
}
