set(ARPG_TOOLS_SOURCES
    src/module_info.cpp
)
set(ARPG_TOOLS_HEADERS
    include/arpg/tools/module_info.hpp
)

add_library(arpg_tools STATIC ${ARPG_TOOLS_SOURCES} ${ARPG_TOOLS_HEADERS})
add_library(arpg::tools ALIAS arpg_tools)

target_include_directories(
    arpg_tools
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(arpg_tools PUBLIC arpg::core_engine PRIVATE arpg::project_options)
arpg_enable_project_warnings(arpg_tools)
arpg_register_format_sources(${ARPG_TOOLS_SOURCES} ${ARPG_TOOLS_HEADERS})
