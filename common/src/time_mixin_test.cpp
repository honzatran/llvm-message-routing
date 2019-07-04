


#include <routing/time_mixin.h>
#include <routing/stdext.h>

#include <gtest/gtest.h>
#include <spdlog/fmt/fmt.h>

#include <chrono>

using namespace routing;
using namespace std;

class Time_mixin_test : public ::testing::Test
{
public:
    class IBase
    {
    public:
        virtual ~IBase() = default;

        virtual void run() = 0;

    private:
    };

    class Derived : public IBase
    {
    public:
        Derived(Function_ref<void()> function) : m_function(function) {}

        void run() override
        {
            if (m_function)
            {
                m_function();
            }
        }

    private:
        Function_ref<void()> m_function;
    };

    class Repeat_derived : public Repeat, public Derived
    {
    public:
        Repeat_derived(
            int times,
            chrono::microseconds repeat,
            Function_ref<void()> function)
            : Repeat(times, repeat), Derived(function)
        {
        }

        void run() override
        {
            repeat([this] {
                        Derived::run();
                    });
        }
    };

private:
};

TEST_F(Time_mixin_test, DISABLED_repeat)
{
    auto f = [] { fmt::print("hitted\n"); };

    std::unique_ptr<IBase> tmp
        = make_unique<Repeat_derived>(5, chrono::milliseconds{200}, f);

    tmp->run();
}

// TEST(Test_crash, crash)
// {
//     std::unique_ptr<int> a;
//
//     int tmp = *a;
// }


