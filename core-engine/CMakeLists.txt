set(ARPG_CORE_ENGINE_SOURCES
    src/core/build_info.cpp
)
set(ARPG_CORE_ENGINE_HEADERS
    include/arpg/core/build_info.hpp
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
