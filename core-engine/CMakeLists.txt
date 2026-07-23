set(ARPG_CORE_ENGINE_SOURCES
    src/core/assert.cpp
    src/core/build_info.cpp
    src/ecs/registry.cpp
    src/diagnostics/timing_metrics.cpp
    src/input/input_buffer.cpp
    src/logging/console_sink.cpp
    src/logging/logger.cpp
    src/memory/fixed_block_pool.cpp
    src/memory/linear_arena.cpp
    src/runtime/fixed_step.cpp
    src/runtime/runtime_loop.cpp
    src/time/monotonic_clock.cpp
)
set(ARPG_CORE_ENGINE_HEADERS
    include/arpg/core/assert.hpp
    include/arpg/core/build_info.hpp
    include/arpg/ecs/component_type_id.hpp
    include/arpg/ecs/entity.hpp
    include/arpg/ecs/registry.hpp
    include/arpg/diagnostics/timing_metrics.hpp
    include/arpg/input/input_buffer.hpp
    include/arpg/platform/platform.hpp
    include/arpg/runtime/fixed_step.hpp
    include/arpg/runtime/runtime_loop.hpp
    include/arpg/time/monotonic_clock.hpp
    include/arpg/memory/allocation.hpp
    include/arpg/memory/detail/checked_arithmetic.hpp
    include/arpg/memory/fixed_block_pool.hpp
    include/arpg/memory/linear_arena.hpp
    include/arpg/logging/console_sink.hpp
    include/arpg/logging/logger.hpp
)

add_library(arpg_core_engine STATIC ${ARPG_CORE_ENGINE_SOURCES} ${ARPG_CORE_ENGINE_HEADERS})
add_library(arpg::core_engine ALIAS arpg_core_engine)

target_include_directories(
    arpg_core_engine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(arpg_core_engine PUBLIC arpg::project_options)
arpg_enable_project_warnings(arpg_core_engine)
arpg_register_format_sources(${ARPG_CORE_ENGINE_SOURCES} ${ARPG_CORE_ENGINE_HEADERS})

set(ARPG_PLATFORM_GLFW_SOURCES
    src/platform/glfw_error_latch.cpp
    src/platform/glfw_platform.cpp
)
set(ARPG_PLATFORM_GLFW_HEADERS
    include/arpg/platform/detail/glfw_error_latch.hpp
    include/arpg/platform/desktop_platform.hpp
)

if(ARPG_BUILD_DESKTOP_CLIENT)
    add_library(arpg_platform_glfw STATIC ${ARPG_PLATFORM_GLFW_SOURCES} ${ARPG_PLATFORM_GLFW_HEADERS})
    add_library(arpg::platform_glfw ALIAS arpg_platform_glfw)
    target_include_directories(
        arpg_platform_glfw
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    )
    target_compile_definitions(arpg_platform_glfw PRIVATE GLFW_INCLUDE_NONE)
    target_link_libraries(arpg_platform_glfw PUBLIC arpg::core_engine PRIVATE glfw arpg::project_options)
    arpg_enable_project_warnings(arpg_platform_glfw)
    arpg_register_format_sources(${ARPG_PLATFORM_GLFW_SOURCES} ${ARPG_PLATFORM_GLFW_HEADERS})
endif()
