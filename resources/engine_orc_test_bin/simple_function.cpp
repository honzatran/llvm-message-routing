
int internal_function(int value)
{
    return value;
}

/// function defined inside the unit test
extern "C" int external_unit_test_function(int value);

extern "C" int simple_function(int value);

/// function called inside the simple_function unit test
int simple_function(int value)
{
    return internal_function(value);
}

extern "C" int calling_external_function(int value);

/// function called inside the external_function unit test
int calling_external_function(int value)
{
    return external_unit_test_function(value);
}
