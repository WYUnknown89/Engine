add_library(arpg_apps INTERFACE)
add_library(arpg::apps ALIAS arpg_apps)
target_link_libraries(arpg_apps INTERFACE arpg::gameplay arpg::project_options)
