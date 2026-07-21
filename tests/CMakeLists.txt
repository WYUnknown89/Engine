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
