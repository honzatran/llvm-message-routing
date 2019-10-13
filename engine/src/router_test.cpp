
#include "base_engine_test.h"

class Router_test : public routing::engine::Base_engine_function_test
{
public:
    Router_test() : routing::engine::Base_engine_function_test("router.cpp") {}
};

TEST_F(Router_test, init) {}
