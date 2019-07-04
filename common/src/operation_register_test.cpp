

#include <routing/operation_register.h>

#include <gtest/gtest.h>
#include <spdlog/fmt/fmt.h>

using namespace routing;
using namespace std;


class Operation_register_test : public ::testing::Test
{
public:

protected:
    class Test_operation : public routing::IOperation
    {
    public:

        int key() override
        {
            return 0;
        }

        Buffer execute(Buffer_view operation_id) override
        {
            Buffer output(4);
            return output;
        }

    private:
    };
};


TEST_F(Operation_register_test, basic_test)
{
    std::shared_ptr<IOperation> test = std::make_shared<Test_operation>();

    Operation_register operation_register;

    operation_register.add(test);

    operation_register.execute(0, Buffer_view());

}
