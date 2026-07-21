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

## GitHub Actions validation

Workflow: [`.github/workflows/m0-validation.yml`](../.github/workflows/m0-validation.yml).

The first run, [M0 validation #29826526762](https://github.com/WYUnknown89/Engine/actions/runs/29826526762),
ran on commit `c81aae8ac442cc436297f99d4061e3a8d5379219` with these results:

| Job | Result | Evidence |
|---|---|---|
| Ubuntu GCC Debug | passed | configure, build, and CTest all succeeded |
| Ubuntu Clang Debug | passed | configure, build, CTest, `format-check`, and `tidy` all succeeded |
| Windows MSVC Debug | failed | Vulkan SDK installation and exposure succeeded; configure failed; build/test were skipped |

The Actions log download API is restricted for the current repository token, so
the detailed Windows configure text is unavailable locally. The repository-side
cause was nevertheless identified and fixed: host Vulkan SDK validation now
searches `%VULKAN_SDK%\\Include` and `%VULKAN_SDK%\\Lib` explicitly, rather than
relying on default CMake search paths. The repair is awaiting a workflow rerun.

## Remaining open gates

- Push commit `b3524a5` (`M0: Validate Windows Vulkan SDK location`) and obtain
  a new successful GitHub Actions run for Ubuntu GCC Debug, Ubuntu Clang Debug,
  and Windows MSVC Debug. The attempted command was:

  ```bash
  git push origin main
  ```

  It failed locally because no GitHub HTTPS credential is configured:

  ```text
  fatal: could not read Username for 'https://github.com': No such device or address
  ```

- Record that run’s job URLs and results here.

M0 remains in progress until all three required GitHub Actions jobs pass. The
local Linux validation and source-quality gates are complete; M1 has not begun.
