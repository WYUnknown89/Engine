# M0 Validation Status

Date: 2026-07-21

Host: Zorin OS 18.1 (`ubuntu`/`debian` family, Ubuntu codename `noble`),
64-bit x86_64.

## Current host tools

The installed host tools were verified before validation:

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

## Normal validation attempted

No `ARPG_VALIDATE_RUNTIME_TOOLS=OFF` or `ARPG_BOOTSTRAP_NULL_GLFW=ON`
override was used.

### Linux GCC Debug

Command run:

```bash
cmake --preset linux-gcc-debug --fresh
```

Result: failed during configuration; build and CTest were not run. The host has
`libvulkan-dev`, shader tools, and the other X11 development headers, but does
not have `libxinerama-dev`. The pinned GLFW 3.4 source correctly stops with:

```text
Xinerama headers not found; install libxinerama development package
```

The command did first reveal a project CMake issue: the host SDK validation
created `Vulkan::Headers`, conflicting with the pinned Vulkan-Headers target.
`cmake/ValidateToolchain.cmake` now validates the host header and loader by
path/library lookup instead, leaving the pinned target as the only
`Vulkan::Headers` provider. A repeat of the exact command above reached the
GLFW prerequisite diagnostic shown here.

Install the missing prerequisite, then resume the normal sequence:

```bash
sudo apt-get install --no-install-recommends libxinerama-dev
```

### Pending normal commands

These commands have not been run because the first required normal configure
cannot complete. They must be run without overrides after the prerequisite is
installed:

```bash
cmake --preset linux-gcc-debug --fresh
cmake --build --preset linux-gcc-debug --parallel
ctest --preset linux-gcc-debug --output-on-failure

cmake --preset linux-clang-debug --fresh
cmake --build --preset linux-clang-debug --parallel
ctest --preset linux-clang-debug --output-on-failure

cmake --preset linux-gcc-release --fresh
cmake --build --preset linux-gcc-release --parallel
ctest --preset linux-gcc-release --output-on-failure

cmake --build --preset linux-clang-debug --target format-check
cmake --build --preset linux-clang-debug --target tidy
```

## CI

`.github/workflows/m0-validation.yml` defines the required GitHub Actions jobs:

- Ubuntu GCC Debug
- Ubuntu Clang Debug, including `format-check` and `tidy`
- Windows MSVC Debug

The workflow has not run from this local repository state, so no CI result is
claimed.

## M0 status

M0 remains in progress. Completion requires all three normal local Linux
validation configurations, local formatting/static-analysis checks, and all
three GitHub Actions jobs to pass. Windows remains unvalidated until the
workflow runs successfully on a Windows runner.
