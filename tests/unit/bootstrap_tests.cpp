#include "arpg/core/build_info.hpp"
#include "arpg/gameplay/module_info.hpp"

#if defined(ARPG_TEST_TOOLS)
#include "arpg/tools/module_info.hpp"
#endif

#include <catch2/catch_test_macros.hpp>
#include <string_view>

TEST_CASE("M0 build contract identifies the canonical specification", "[unit][bootstrap]") {
    const auto build = arpg::core::build_info();

    CHECK(build.project_name == std::string_view{"ARPG Engine"});
    CHECK(build.project_version == std::string_view{"0.1.0"});
    CHECK(build.specification_version == std::string_view{"1.1"});
    CHECK(build.pointer_size == 8U);
    CHECK(build.cpp_standard >= 202002L);
}

TEST_CASE("gameplay module is linked through the core engine", "[unit][bootstrap]") {
    const auto module = arpg::gameplay::module_info();

    CHECK(module.name == std::string_view{"arpg-gameplay"});
    CHECK(module.required_specification_version == std::string_view{"1.1"});
}

#if defined(ARPG_TEST_TOOLS)
TEST_CASE("tool module depends on core without gameplay authority", "[unit][bootstrap]") {
    const auto module = arpg::tools::module_info();

    CHECK(module.name == std::string_view{"arpg-tools"});
    CHECK(module.required_specification_version == std::string_view{"1.1"});
}
#endif
