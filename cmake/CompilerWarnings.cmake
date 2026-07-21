include_guard(GLOBAL)

function(arpg_enable_project_warnings TARGET_NAME)
    if(MSVC)
        target_compile_options(${TARGET_NAME} PRIVATE /W4)
        if(ARPG_WARNINGS_AS_ERRORS)
            target_compile_options(${TARGET_NAME} PRIVATE /WX)
        endif()
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(
            ${TARGET_NAME}
            PRIVATE
                -Wall
                -Wextra
                -Wpedantic
                -Wconversion
                -Wsign-conversion
                -Wshadow
                -Wformat=2
                -Wundef
                -Wcast-align
                -Wnon-virtual-dtor
                -Woverloaded-virtual
                -Wold-style-cast
        )
        if(ARPG_WARNINGS_AS_ERRORS)
            target_compile_options(${TARGET_NAME} PRIVATE -Werror)
        endif()
    else()
        message(FATAL_ERROR "Unsupported C++ compiler: ${CMAKE_CXX_COMPILER_ID}")
    endif()
endfunction()
