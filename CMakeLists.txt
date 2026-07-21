cmake_minimum_required(VERSION 3.28)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not supported. Use a preset or configure under build/.")
endif()

project(
    ArpgEngine
    VERSION 0.1.0
    DESCRIPTION "Bespoke 3D isometric action RPG engine"
    LANGUAGES C CXX
)

set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(ProjectOptions)
include(CompilerWarnings)
include(ValidateToolchain)

arpg_validate_toolchain()
arpg_create_project_options()

include(Dependencies)
arpg_configure_dependencies()

include(CTest)

add_subdirectory(core-engine)
add_subdirectory(arpg-gameplay)
add_subdirectory(apps)

if(ARPG_BUILD_TOOLS)
    add_subdirectory(dev-tools)
endif()

if(BUILD_TESTING AND ARPG_BUILD_TESTS)
    add_subdirectory(tests)
endif()

include(ValidateTargetGraph)
arpg_validate_target_graph()

get_property(ARPG_FORMAT_SOURCES GLOBAL PROPERTY ARPG_FORMAT_SOURCES)
list(REMOVE_DUPLICATES ARPG_FORMAT_SOURCES)
get_property(ARPG_TIDY_SOURCES GLOBAL PROPERTY ARPG_TIDY_SOURCES)
list(REMOVE_DUPLICATES ARPG_TIDY_SOURCES)

find_program(ARPG_CLANG_FORMAT_EXECUTABLE NAMES clang-format clang-format-18)
if(ARPG_CLANG_FORMAT_EXECUTABLE)
    add_custom_target(
        format-check
        COMMAND "${ARPG_CLANG_FORMAT_EXECUTABLE}" --dry-run --Werror ${ARPG_FORMAT_SOURCES}
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        COMMENT "Checking project source formatting"
        VERBATIM
    )
else()
    add_custom_target(
        format-check
        COMMAND "${CMAKE_COMMAND}" -E echo
                "clang-format 18 is required. On Zorin/Ubuntu install package: clang-format"
        COMMAND "${CMAKE_COMMAND}" -E false
        VERBATIM
    )
endif()

find_program(ARPG_CLANG_TIDY_EXECUTABLE NAMES clang-tidy clang-tidy-18)
if(ARPG_CLANG_TIDY_EXECUTABLE)
    add_custom_target(
        tidy
        COMMAND "${ARPG_CLANG_TIDY_EXECUTABLE}" --quiet -p "${CMAKE_BINARY_DIR}" ${ARPG_TIDY_SOURCES}
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        COMMENT "Running clang-tidy on project sources"
        VERBATIM
    )
else()
    add_custom_target(
        tidy
        COMMAND "${CMAKE_COMMAND}" -E echo
                "clang-tidy 18 is required. On Zorin/Ubuntu install package: clang-tidy"
        COMMAND "${CMAKE_COMMAND}" -E false
        VERBATIM
    )
endif()
