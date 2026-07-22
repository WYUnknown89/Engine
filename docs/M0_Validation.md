# M0 Validation Status

Date: 2026-07-21

Host: Zorin OS 18.1 (`ubuntu`/`debian` family, Ubuntu codename `noble`),
64-bit x86_64.

## Host tools

The installed host tools used for this validation are:

```text
cmake 3.28.3
ninja 1.11.1
g++ 13.3.0
clang++ 18.1.3
clang-format 18.1.3
clang-tidy 18.1.3
glslc 2023.8
glslangValidator 15.1.0
spirv-val 2025.1
pkg-config: Vulkan 1.3.275, GLFW 3.3.10, GLM 0.9.9.8
```

All required X11 development packages, including `libxinerama-dev`, are
installed. No bootstrap override was used in any command below.

## Local Linux validation

### Linux GCC Debug — passed

```bash
cmake --preset linux-gcc-debug --fresh
cmake --build --preset linux-gcc-debug --parallel
ctest --preset linux-gcc-debug --output-on-failure
```

Result: configuration and build passed with `ARPG_VALIDATE_RUNTIME_TOOLS=ON`,
`ARPG_BOOTSTRAP_NULL_GLFW=OFF`, and project warnings as errors. CTest passed all
4 of 4 tests (3 `unit`, 1 `integration`).

### Linux Clang Debug — passed

```bash
cmake --preset linux-clang-debug --fresh
cmake --build --preset linux-clang-debug --parallel
ctest --preset linux-clang-debug --output-on-failure
```

Result: configuration and build passed with `ARPG_VALIDATE_RUNTIME_TOOLS=ON`,
`ARPG_BOOTSTRAP_NULL_GLFW=OFF`, and project warnings as errors. CTest passed all
4 of 4 tests (3 `unit`, 1 `integration`).

### Linux GCC Release — passed

```bash
cmake --preset linux-gcc-release --fresh
cmake --build --preset linux-gcc-release --parallel
ctest --preset linux-gcc-release --output-on-failure
```

Result: configuration and build passed with `ARPG_VALIDATE_RUNTIME_TOOLS=ON`,
`ARPG_BOOTSTRAP_NULL_GLFW=OFF`, and project warnings as errors. CTest passed all
4 of 4 tests (3 `unit`, 1 `integration`).

### Formatting and static analysis — passed

```bash
cmake --build --preset linux-clang-debug --target format-check
cmake --build --preset linux-clang-debug --target tidy
```

Result: both targets exited successfully. `clang-tidy` processed the 5 project
source files; diagnostics from external headers were suppressed as configured.

## Catch2 C++ standard consistency fix

Windows MSVC reached the build stage after the Vulkan SDK lookup repair but
failed when linking `arpg_bootstrap_tests` with:

```text
LNK2019 unresolved Catch::StringMaker<std::string_view>::convert
```

The effective configuration before this fix was inconsistent:

| Target | Effective language/configuration before the fix |
|---|---|
| `Catch2` | Catch2's upstream `PUBLIC cxx_std_14` requirement; no explicit `CXX_STANDARD`; generated `CATCH_CONFIG_CPP17_STRING_VIEW=OFF` |
| `Catch2WithMain` | inherited Catch2's C++14 baseline; no explicit `CXX_STANDARD` |
| `arpg_bootstrap_tests` | project option `cxx_std_20` (`/std:c++20` on MSVC) |

This allowed the C++20 test translation unit to declare Catch2's automatic
`std::string_view` `StringMaker`, while the separately compiled Catch2 library
was built without its matching definition.

`cmake/Dependencies.cmake` now sets the supported
`CATCH_CONFIG_CPP17_STRING_VIEW=ON` CMake cache configuration before fetching
Catch2. It also sets `CXX_STANDARD=20`, `CXX_STANDARD_REQUIRED=ON`,
`CXX_EXTENSIONS=OFF`, and a public `cxx_std_20` requirement on both `Catch2`
and `Catch2WithMain`; no fetched source was edited.

The regenerated GCC Debug build confirms all three relevant targets now use
`-std=c++20`, and Catch2's generated `catch_user_config.hpp` contains
`#define CATCH_CONFIG_CPP17_STRING_VIEW`. GCC Debug, Clang Debug, and GCC
Release each rebuilt and passed 4 of 4 CTest cases with this configuration.

## MSVC `__cplusplus` conformance fix

The subsequent Windows MSVC build succeeded, but the M0 build-contract test
reported `build.cpp_standard` as `199711` rather than at least `202002`.
`build_info.cpp` correctly reads `__cplusplus`; MSVC reports the legacy value
unless `/Zc:__cplusplus` is enabled.

`cmake/ProjectOptions.cmake` now adds `/Zc:__cplusplus` to the central
`arpg::project_options` interface target alongside the existing MSVC conformance
options. The option therefore propagates to `arpg_core_engine`,
`arpg_gameplay`, `arpg_tools`, the `arpg_apps` application facade, and the
project test targets. Fetched third-party targets do not consume this interface
and receive no project warning or compiler-option policy.

## GitHub Actions validation

The historical M0 runs used `.github/workflows/m0-validation.yml`. That file
was replaced during M1 by the milestone-neutral
[`.github/workflows/ci.yml`](../.github/workflows/ci.yml); this does not alter
the historical M0 results recorded below.

The initial run, [M0 validation #29826526762](https://github.com/WYUnknown89/Engine/actions/runs/29826526762),
failed Windows configuration on commit `c81aae8ac442cc436297f99d4061e3a8d5379219`.
After the Vulkan SDK lookup repair, [M0 validation #29832328672](https://github.com/WYUnknown89/Engine/actions/runs/29832328672)
ran on commit `ca65a04fdb897b3eb8568c491ba4c102004f5475` with these exact results:

| Job | Result | Evidence |
|---|---|---|
| [Ubuntu GCC Debug](https://github.com/WYUnknown89/Engine/actions/runs/29832328672/job/88639888780) | passed | configure, build, and CTest all succeeded |
| [Ubuntu Clang Debug](https://github.com/WYUnknown89/Engine/actions/runs/29832328672/job/88639888789) | passed | configure, build, CTest, `format-check`, and `tidy` all succeeded |
| [Windows MSVC Debug](https://github.com/WYUnknown89/Engine/actions/runs/29832328672/job/88639888765) | failed | configure succeeded; build failed linking `arpg_bootstrap_tests` with the Catch2 `std::string_view` unresolved external above; test was skipped |

The earlier failures were corrected in sequence: host Vulkan SDK lookup now
searches `%VULKAN_SDK%\\Include` and `%VULKAN_SDK%\\Lib`; Catch2 uses a consistent
C++20/string-view configuration; and project MSVC targets enable
`/Zc:__cplusplus`.

### Final successful run

[M0 validation #29833829404](https://github.com/WYUnknown89/Engine/actions/runs/29833829404)
completed successfully on commit `bb6b9d07d051e514f4495b6980970f65d43c220e`:

| Job | Result | Evidence |
|---|---|---|
| [Ubuntu GCC Debug](https://github.com/WYUnknown89/Engine/actions/runs/29833829404/job/88644965559) | passed | configure, build, and CTest succeeded |
| [Ubuntu Clang Debug](https://github.com/WYUnknown89/Engine/actions/runs/29833829404/job/88644965601) | passed | configure, build, CTest, `format-check`, and `tidy` succeeded |
| [Windows MSVC Debug](https://github.com/WYUnknown89/Engine/actions/runs/29833829404/job/88644965593) | passed | configure, build, and all 4 of 4 CTest cases succeeded |

## M0 closure

All documented local Linux validations, source-quality checks, and required
GitHub Actions jobs have passed. The independent technical review approved the
M0 implementation. There are no remaining M0 gates or blockers.

M0 is complete. At the time of this M0 closure, M1 had not begun and required
explicit approval before implementation work started.
