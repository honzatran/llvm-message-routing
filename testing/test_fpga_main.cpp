
#include <rapid_addition/application.h>
#include <rapid_addition/config.h>
#include <rapid_addition/fatal_signal_handlers.h>
#include <rapid_addition/fpga/cam_memory.h>
#include <rapid_addition/fpga/dma_socket.h>
#include <rapid_addition/fpga/fpga_global_options.h>
#include <rapid_addition/logger.h>
#include <rapid_addition/portability.h>
#include <rapid_addition/stdext.h>
#include <rapid_addition/testing.h>

#include <rapid_addition/fastlane_test/fpga_test.h>
#include <rapid_addition/fastlane_test/global_test_setting.h>
#include <rapid_addition/fastlane_test/test_config.h>
#include <rapid_addition/application.h>

#include <iostream>
#include <vector>

#include <gtest/gtest.h>

#include <rapid_addition/fastlane_test/test_options.h>

using namespace std;
using namespace rapid_addition;

// HResource BW_TCP_FIX_BAR_RES;
// std::unique_ptr<fpga::Hil_device> g_hil_device;
//

RA_ATTR_WEAK int main(int argc, char** argv);

bool
run_additional_fpga_initialization(Config const& config)
{
    if (test::additional_fpga_initialization)
    {
        return test::additional_fpga_initialization(config);
    }

    return true;
}

vector<program_options::options_description>
get_all_subcomponent_desc()
{
    vector<program_options::options_description> options
        = {get_logger_options_description(),
           fpga::get_fpga_global_option_description()};

    auto global_test_options = test::get_global_test_options();

    for (auto&& test_option : global_test_options)
    {
        options.emplace_back(std::move(test_option));
    }

    if (test::get_test_configurations)
    {
        for (auto&& test_option : test::get_test_configurations())
        {
            options.emplace_back(std::move(test_option));
        }
    }

    return options;
}

int
main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    rapid_addition::Config config = rapid_addition::parse_args(
        get_all_subcomponent_desc(), argc, argv, TEST_CONFIG_PATH);

    rapid_addition::init_logger(config);
    rapid_addition::test::init_global_settings(config);

    rapid_addition::install_fatal_signal_handler();
    rapid_addition::init_interrupt_handler();

    Logger_t logger
        = rapid_addition::get_default_logger("FPGA_main_logger");

    auto test_setting = rapid_addition::test::get_global_setting();

    if (test_setting->test_fpga() && !test::init_fpga(config)) 
    {
        logger->info("Initializing the hil device has failed");
        return -1;
    }

    if (test_setting->test_fpga())
    {
        run_additional_fpga_initialization(config);
    }

    rapid_addition::test::set_config(config);

    return RUN_ALL_TESTS();
}


