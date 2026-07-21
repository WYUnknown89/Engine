include_guard(GLOBAL)

function(arpg_validate_toolchain)
    if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
        message(FATAL_ERROR "ARPG Engine supports only 64-bit targets; pointer size is ${CMAKE_SIZEOF_VOID_P}")
    endif()

    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 13)
        message(FATAL_ERROR "GCC 13 or newer is required; found ${CMAKE_CXX_COMPILER_VERSION}")
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 18)
        message(FATAL_ERROR "Clang 18 or newer is required; found ${CMAKE_CXX_COMPILER_VERSION}")
    elseif(MSVC AND MSVC_VERSION LESS 1938)
        message(FATAL_ERROR "MSVC 19.38 / Visual Studio 2022 17.8 or newer is required")
    elseif(NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|MSVC")
        message(FATAL_ERROR "Unsupported compiler family: ${CMAKE_CXX_COMPILER_ID}")
    endif()

    if(ARPG_VALIDATE_RUNTIME_TOOLS)
        # Do not call FindVulkan here: it creates Vulkan::Headers, which would
        # conflict with the deliberately pinned Vulkan-Headers target supplied
        # by Dependencies.cmake.  This check validates the host SDK prerequisite
        # only; project targets continue to use the pinned headers.
        find_path(
            ARPG_SYSTEM_VULKAN_INCLUDE_DIR
            NAMES vulkan/vulkan.h
            HINTS "$ENV{VULKAN_SDK}/Include")
        find_library(
            ARPG_SYSTEM_VULKAN_LOADER_LIBRARY
            NAMES vulkan vulkan-1
            HINTS "$ENV{VULKAN_SDK}/Lib" "$ENV{VULKAN_SDK}/Lib32")
        if(NOT ARPG_SYSTEM_VULKAN_INCLUDE_DIR OR NOT ARPG_SYSTEM_VULKAN_LOADER_LIBRARY)
            message(FATAL_ERROR
                "Vulkan 1.3 development headers and loader were not found. "
                "On Zorin/Ubuntu install: libvulkan-dev. On Windows install the Vulkan SDK.")
        endif()

        find_program(ARPG_GLSLC_EXECUTABLE NAMES glslc)
        find_program(ARPG_GLSLANG_VALIDATOR_EXECUTABLE NAMES glslangValidator)
        find_program(ARPG_SPIRV_VAL_EXECUTABLE NAMES spirv-val)
        find_program(ARPG_VULKANINFO_EXECUTABLE NAMES vulkaninfo vulkaninfoSDK)

        foreach(ARPG_TOOL IN ITEMS GLSLC GLSLANG_VALIDATOR SPIRV_VAL)
            if(NOT ARPG_${ARPG_TOOL}_EXECUTABLE)
                string(TOLOWER "${ARPG_TOOL}" ARPG_TOOL_LOWER)
                message(FATAL_ERROR
                    "Required shader tool '${ARPG_TOOL_LOWER}' was not found. "
                    "On Zorin/Ubuntu install: glslc glslang-tools spirv-tools")
            endif()
        endforeach()

        if(NOT ARPG_VULKANINFO_EXECUTABLE)
            message(FATAL_ERROR
                "Required Vulkan diagnostic tool 'vulkaninfo' was not found. "
                "On Zorin/Ubuntu install: vulkan-tools. On Windows install the Vulkan SDK.")
        endif()
    endif()
endfunction()
