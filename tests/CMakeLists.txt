include("${ARPG_CATCH2_EXTRAS_DIR}/Catch.cmake")

set(ARPG_BOOTSTRAP_TEST_SOURCES
    unit/bootstrap_tests.cpp
)
add_executable(arpg_bootstrap_tests ${ARPG_BOOTSTRAP_TEST_SOURCES})
target_link_libraries(
    arpg_bootstrap_tests
    PRIVATE
        arpg::project_options
        arpg::core_engine
        arpg::gameplay
        Catch2::Catch2WithMain
)
if(ARPG_BUILD_TOOLS)
    target_link_libraries(arpg_bootstrap_tests PRIVATE arpg::tools)
    target_compile_definitions(arpg_bootstrap_tests PRIVATE ARPG_TEST_TOOLS=1)
endif()
arpg_enable_project_warnings(arpg_bootstrap_tests)
arpg_register_format_sources(${ARPG_BOOTSTRAP_TEST_SOURCES})
catch_discover_tests(arpg_bootstrap_tests PROPERTIES LABELS "unit;bootstrap")

set(ARPG_DEPENDENCY_SMOKE_SOURCES
    integration/dependency_smoke.cpp
)
if(ARPG_BUILD_DEPENDENCY_SMOKE)
    add_executable(arpg_dependency_smoke ${ARPG_DEPENDENCY_SMOKE_SOURCES})
    target_compile_definitions(arpg_dependency_smoke PRIVATE GLFW_INCLUDE_NONE)
    target_link_libraries(
        arpg_dependency_smoke
        PRIVATE
            arpg::project_options
            Catch2::Catch2WithMain
            glfw
            glm::glm
            imgui::imgui
            volk::volk
            Vulkan::Headers
    )
    arpg_enable_project_warnings(arpg_dependency_smoke)
    arpg_register_format_sources(${ARPG_DEPENDENCY_SMOKE_SOURCES})
    catch_discover_tests(arpg_dependency_smoke PROPERTIES LABELS "integration;dependencies")
endif()

set(ARPG_M1_UNIT_SOURCES
    unit/m1_runtime_tests.cpp
)
add_executable(arpg_m1_unit_tests ${ARPG_M1_UNIT_SOURCES})
target_link_libraries(arpg_m1_unit_tests PRIVATE arpg::project_options arpg::core_engine Catch2::Catch2WithMain)
arpg_enable_project_warnings(arpg_m1_unit_tests)
arpg_register_format_sources(${ARPG_M1_UNIT_SOURCES})
catch_discover_tests(arpg_m1_unit_tests PROPERTIES LABELS "unit;m1")

set(ARPG_M1_INTEGRATION_SOURCES
    integration/m1_runtime_integration.cpp
)
add_executable(arpg_m1_integration_tests ${ARPG_M1_INTEGRATION_SOURCES})
target_link_libraries(arpg_m1_integration_tests PRIVATE arpg::project_options arpg::core_engine Catch2::Catch2WithMain)
arpg_enable_project_warnings(arpg_m1_integration_tests)
arpg_register_format_sources(${ARPG_M1_INTEGRATION_SOURCES})
catch_discover_tests(arpg_m1_integration_tests PROPERTIES LABELS "integration;m1")

if(ARPG_BUILD_DESKTOP_CLIENT)
    set(ARPG_GLFW_PLATFORM_TEST_SOURCES
        unit/glfw_error_latch_tests.cpp
    )
    add_executable(arpg_glfw_platform_tests ${ARPG_GLFW_PLATFORM_TEST_SOURCES})
    target_link_libraries(arpg_glfw_platform_tests PRIVATE arpg::project_options arpg::platform_glfw Catch2::Catch2WithMain)
    arpg_enable_project_warnings(arpg_glfw_platform_tests)
    arpg_register_format_sources(${ARPG_GLFW_PLATFORM_TEST_SOURCES})
    catch_discover_tests(arpg_glfw_platform_tests PROPERTIES LABELS "unit;platform;m1")
endif()

set(ARPG_M2_MEMORY_SOURCES
    unit/m2_memory_tests.cpp
)
add_executable(arpg_m2_memory_tests ${ARPG_M2_MEMORY_SOURCES})
target_link_libraries(arpg_m2_memory_tests PRIVATE arpg::project_options arpg::core_engine Catch2::Catch2WithMain)
arpg_enable_project_warnings(arpg_m2_memory_tests)
arpg_register_format_sources(${ARPG_M2_MEMORY_SOURCES})
catch_discover_tests(arpg_m2_memory_tests PROPERTIES LABELS "unit;m2;memory")

set(ARPG_M2_ASSERTION_SOURCES
    unit/m2_assertion_tests.cpp
)
add_executable(arpg_m2_assertion_tests ${ARPG_M2_ASSERTION_SOURCES})
target_link_libraries(arpg_m2_assertion_tests PRIVATE arpg::project_options arpg::core_engine Catch2::Catch2WithMain)
arpg_enable_project_warnings(arpg_m2_assertion_tests)
arpg_register_format_sources(${ARPG_M2_ASSERTION_SOURCES})
catch_discover_tests(arpg_m2_assertion_tests PROPERTIES LABELS "unit;m2;assert")

try_compile(
    ARPG_M2_INVALID_RELEASE_ASSERT_COMPILED
    "${CMAKE_BINARY_DIR}/m2-invalid-release-assert"
    SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/compile/m2_invalid_release_assert.cpp"
    CMAKE_FLAGS "-DCMAKE_CXX_STANDARD=20" "-DCMAKE_CXX_STANDARD_REQUIRED=ON"
)
if(ARPG_M2_INVALID_RELEASE_ASSERT_COMPILED)
    message(FATAL_ERROR "Release ARPG_ASSERT expressions must remain compile-time type-checked.")
endif()

add_executable(arpg_m2_assertion_probe support/m2_assertion_probe.cpp)
target_link_libraries(arpg_m2_assertion_probe PRIVATE arpg::project_options arpg::core_engine)
arpg_enable_project_warnings(arpg_m2_assertion_probe)
arpg_register_format_sources(support/m2_assertion_probe.cpp)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_test(
        NAME "M2 Debug assertion probe emits diagnostic and terminates"
        COMMAND "${CMAKE_COMMAND}" -DPROBE=$<TARGET_FILE:arpg_m2_assertion_probe>
                -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/verify_assertion_probe.cmake"
    )
    set_tests_properties("M2 Debug assertion probe emits diagnostic and terminates" PROPERTIES LABELS "unit;m2;assert")
endif()

set(ARPG_M2_LOGGING_SOURCES
    unit/m2_logging_tests.cpp
)
add_executable(arpg_m2_logging_tests ${ARPG_M2_LOGGING_SOURCES})
target_link_libraries(arpg_m2_logging_tests PRIVATE arpg::project_options arpg::core_engine Catch2::Catch2WithMain)
arpg_enable_project_warnings(arpg_m2_logging_tests)
arpg_register_format_sources(${ARPG_M2_LOGGING_SOURCES})
catch_discover_tests(arpg_m2_logging_tests PROPERTIES LABELS "unit;m2;logging")

set(ARPG_M2_METRICS_SOURCES
    unit/m2_metrics_tests.cpp
)
add_executable(arpg_m2_metrics_tests ${ARPG_M2_METRICS_SOURCES})
target_link_libraries(arpg_m2_metrics_tests PRIVATE arpg::project_options arpg::core_engine Catch2::Catch2WithMain)
arpg_enable_project_warnings(arpg_m2_metrics_tests)
arpg_register_format_sources(${ARPG_M2_METRICS_SOURCES})
catch_discover_tests(arpg_m2_metrics_tests PROPERTIES LABELS "unit;m2;metrics")

set(ARPG_M2_ALLOCATOR_STRESS_SOURCES
    integration/m2_allocator_stress.cpp
)
add_executable(arpg_m2_allocator_stress_tests ${ARPG_M2_ALLOCATOR_STRESS_SOURCES})
target_link_libraries(arpg_m2_allocator_stress_tests PRIVATE arpg::project_options arpg::core_engine Catch2::Catch2WithMain)
arpg_enable_project_warnings(arpg_m2_allocator_stress_tests)
arpg_register_format_sources(${ARPG_M2_ALLOCATOR_STRESS_SOURCES})
catch_discover_tests(arpg_m2_allocator_stress_tests PROPERTIES LABELS "integration;m2;memory;stress")

set(ARPG_M2_ALLOCATOR_BENCHMARK_SOURCES
    performance/m2_allocator_benchmark.cpp
)
add_executable(arpg_m2_allocator_benchmark ${ARPG_M2_ALLOCATOR_BENCHMARK_SOURCES})
target_link_libraries(arpg_m2_allocator_benchmark PRIVATE arpg::project_options arpg::core_engine)
arpg_enable_project_warnings(arpg_m2_allocator_benchmark)
arpg_register_format_sources(${ARPG_M2_ALLOCATOR_BENCHMARK_SOURCES})

set(ARPG_M3_ECS_SOURCES
    unit/m3_ecs_tests.cpp
    support/m3_component_type_identity_tu.cpp
)
add_executable(arpg_m3_ecs_tests ${ARPG_M3_ECS_SOURCES})
target_include_directories(arpg_m3_ecs_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/support")
target_link_libraries(arpg_m3_ecs_tests PRIVATE arpg::project_options arpg::core_engine Catch2::Catch2WithMain)
arpg_enable_project_warnings(arpg_m3_ecs_tests)
arpg_register_format_sources(${ARPG_M3_ECS_SOURCES} support/m3_component_type_identity_shared.hpp)
catch_discover_tests(arpg_m3_ecs_tests PROPERTIES LABELS "unit;m3;ecs")

add_executable(arpg_m3_ecs_allocation_tests unit/m3_ecs_allocation_tests.cpp)
target_link_libraries(arpg_m3_ecs_allocation_tests PRIVATE arpg::project_options arpg::core_engine)
arpg_enable_project_warnings(arpg_m3_ecs_allocation_tests)
arpg_register_format_sources(unit/m3_ecs_allocation_tests.cpp)
add_test(NAME "M3 prepared ECS operations allocate no storage" COMMAND arpg_m3_ecs_allocation_tests)
set_tests_properties("M3 prepared ECS operations allocate no storage" PROPERTIES LABELS "unit;m3;ecs;allocation")

set(ARPG_M3_ECS_BENCHMARK_SOURCES
    performance/m3_ecs_benchmark.cpp
)
add_executable(arpg_m3_ecs_benchmark ${ARPG_M3_ECS_BENCHMARK_SOURCES})
target_link_libraries(arpg_m3_ecs_benchmark PRIVATE arpg::project_options arpg::core_engine)
arpg_enable_project_warnings(arpg_m3_ecs_benchmark)
arpg_register_format_sources(${ARPG_M3_ECS_BENCHMARK_SOURCES})
