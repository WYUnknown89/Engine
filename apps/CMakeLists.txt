add_library(arpg_apps INTERFACE)
add_library(arpg::apps ALIAS arpg_apps)
target_link_libraries(arpg_apps INTERFACE arpg::gameplay arpg::project_options)

if(ARPG_BUILD_DESKTOP_CLIENT)
    add_subdirectory(arpg-client)
endif()
