set(ARPG_GAMEPLAY_SOURCES
    src/module_info.cpp
)
set(ARPG_GAMEPLAY_HEADERS
    include/arpg/gameplay/module_info.hpp
)

add_library(arpg_gameplay STATIC ${ARPG_GAMEPLAY_SOURCES} ${ARPG_GAMEPLAY_HEADERS})
add_library(arpg::gameplay ALIAS arpg_gameplay)

target_include_directories(
    arpg_gameplay
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(arpg_gameplay PUBLIC arpg::core_engine PRIVATE arpg::project_options)
arpg_enable_project_warnings(arpg_gameplay)
arpg_register_format_sources(${ARPG_GAMEPLAY_SOURCES} ${ARPG_GAMEPLAY_HEADERS})
