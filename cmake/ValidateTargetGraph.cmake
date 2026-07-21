include_guard(GLOBAL)

function(arpg_assert_target_does_not_link TARGET_NAME)
    get_target_property(ARPG_LINKS ${TARGET_NAME} LINK_LIBRARIES)
    if(NOT ARPG_LINKS)
        return()
    endif()

    foreach(ARPG_FORBIDDEN IN LISTS ARGN)
        if(ARPG_FORBIDDEN IN_LIST ARPG_LINKS)
            message(FATAL_ERROR "Target ${TARGET_NAME} must not link forbidden target ${ARPG_FORBIDDEN}")
        endif()
    endforeach()
endfunction()

function(arpg_validate_target_graph)
    arpg_assert_target_does_not_link(arpg_core_engine arpg_gameplay arpg_tools arpg_apps)
    arpg_assert_target_does_not_link(arpg_gameplay arpg_tools arpg_apps)

    if(TARGET arpg_tools)
        arpg_assert_target_does_not_link(arpg_tools arpg_gameplay arpg_apps)
    endif()
endfunction()
