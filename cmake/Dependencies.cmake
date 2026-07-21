include_guard(GLOBAL)

include(FetchContent)

function(arpg_configure_dependencies)
    set(FETCHCONTENT_QUIET OFF)

    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    if(UNIX AND NOT APPLE)
        if(ARPG_BOOTSTRAP_NULL_GLFW)
            message(WARNING
                "ARPG_BOOTSTRAP_NULL_GLFW is enabled: this validates M0 dependencies only, not Linux window support")
            set(GLFW_BUILD_NULL ON CACHE BOOL "" FORCE)
            set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
            set(GLFW_BUILD_X11 OFF CACHE BOOL "" FORCE)
        else()
            set(GLFW_BUILD_NULL OFF CACHE BOOL "" FORCE)
            set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
            set(GLFW_BUILD_X11 ON CACHE BOOL "" FORCE)
        endif()
    endif()

    set(GLM_BUILD_INSTALL OFF CACHE BOOL "" FORCE)
    set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(CATCH_BUILD_TESTING OFF CACHE BOOL "" FORCE)
    set(CATCH_INSTALL_DOCS OFF CACHE BOOL "" FORCE)
    set(CATCH_INSTALL_EXTRAS OFF CACHE BOOL "" FORCE)
    # Catch2 is a compiled dependency. Force its generated configuration and
    # compilation mode to match the project's C++20 consumers so the
    # std::string_view StringMaker declaration and definition cannot diverge.
    set(CATCH_CONFIG_CPP17_STRING_VIEW ON CACHE BOOL "" FORCE)
    set(VOLK_INSTALL OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        vulkan_headers
        URL https://github.com/KhronosGroup/Vulkan-Headers/archive/refs/tags/v1.4.350.tar.gz
        URL_HASH SHA256=6dd105e5cc7ddab6e7b611ae2c1872740d1727557cc8bf9daf13d6de1e4b3999
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        SYSTEM
    )
    FetchContent_Declare(
        glfw
        URL https://github.com/glfw/glfw/archive/refs/tags/3.4.tar.gz
        URL_HASH SHA256=c038d34200234d071fae9345bc455e4a8f2f544ab60150765d7704e08f3dac01
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        SYSTEM
    )
    FetchContent_Declare(
        glm
        URL https://github.com/g-truc/glm/archive/refs/tags/1.0.3.tar.gz
        URL_HASH SHA256=6775e47231a446fd086d660ecc18bcd076531cfedd912fbd66e576b118607001
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        SYSTEM
    )
    FetchContent_Declare(
        catch2
        URL https://github.com/catchorg/Catch2/archive/refs/tags/v3.15.0.tar.gz
        URL_HASH SHA256=9650c55e497759cc39b977e45524bc8acb15256061c112080916ab6cb0b1ea66
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        SYSTEM
    )
    FetchContent_Declare(
        volk
        URL https://github.com/zeux/volk/archive/refs/tags/1.4.350.tar.gz
        URL_HASH SHA256=e47c6efe5294bb03729a976b385864bba5daf46ec60f0bdea11e1d1446345f9a
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        SYSTEM
    )
    FetchContent_Declare(
        imgui
        URL https://github.com/ocornut/imgui/archive/refs/tags/v1.92.8.tar.gz
        URL_HASH SHA256=fecb33d33930e12ff53a34064e9d3a06c8f7c3e04408f14cd36c80e3faac863b
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        SYSTEM
    )

    FetchContent_MakeAvailable(vulkan_headers glfw glm catch2 volk)

    foreach(ARPG_CATCH_TARGET IN ITEMS Catch2 Catch2WithMain)
        if(NOT TARGET ${ARPG_CATCH_TARGET})
            message(FATAL_ERROR "Pinned Catch2 did not provide target ${ARPG_CATCH_TARGET}")
        endif()
        set_target_properties(
            ${ARPG_CATCH_TARGET}
            PROPERTIES
                CXX_STANDARD 20
                CXX_STANDARD_REQUIRED ON
                CXX_EXTENSIONS OFF)
        target_compile_features(${ARPG_CATCH_TARGET} PUBLIC cxx_std_20)
    endforeach()

    set(ARPG_CATCH2_EXTRAS_DIR "${catch2_SOURCE_DIR}/extras" CACHE INTERNAL
        "Path to the pinned Catch2 CMake integration modules")
    FetchContent_GetProperties(imgui)
    if(NOT imgui_POPULATED)
        FetchContent_Populate(imgui)
    endif()

    add_library(
        arpg_imgui STATIC
        "${imgui_SOURCE_DIR}/imgui.cpp"
        "${imgui_SOURCE_DIR}/imgui_draw.cpp"
        "${imgui_SOURCE_DIR}/imgui_tables.cpp"
        "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
    )
    add_library(imgui::imgui ALIAS arpg_imgui)
    target_include_directories(arpg_imgui SYSTEM PUBLIC "${imgui_SOURCE_DIR}")
    set_target_properties(arpg_imgui PROPERTIES POSITION_INDEPENDENT_CODE ON)
endfunction()
