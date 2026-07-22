add_executable(arpg_client main.cpp)
target_link_libraries(arpg_client PRIVATE arpg::apps arpg::platform_glfw arpg::project_options)
arpg_enable_project_warnings(arpg_client)
arpg_register_format_sources(main.cpp)
