# ARPG Engine

A bespoke C++20 and Vulkan 1.3 engine for a 3D isometric action RPG. The engine
is intentionally genre-specific and is not a general-purpose game engine.

**M0 through M2 are complete. M3: ECS Foundation is at the closure gate on
`m3-ecs` and is not yet complete on `main`.** The repository now also has
generational entities, sparse-set component storage, deterministic queries,
ordered deferred structural changes, and prepared hot-path allocation
validation.

Validation evidence is recorded in
[`docs/M1_Validation.md`](docs/M1_Validation.md) and
[`docs/M0_Validation.md`](docs/M0_Validation.md). Final M2 validation evidence
is recorded in [`docs/M2_Validation.md`](docs/M2_Validation.md), and M3 closure
evidence is recorded in [`docs/M3_Validation.md`](docs/M3_Validation.md).

The canonical requirements are in
[`docs/Master_Implementation_Specification.md`](docs/Master_Implementation_Specification.md).
There is currently no project licence grant. Third-party components retain
their own licences, recorded in `THIRD_PARTY_NOTICES.md`.

## Supported development configurations

- 64-bit Zorin/Ubuntu Linux with GCC 13+ or Clang 18+
- 64-bit Windows with Visual Studio 2022 / MSVC 19.38+
- CMake 3.28+ and C++20 without compiler extensions

## Zorin OS 18 / Ubuntu 24.04 prerequisites

Package installation requires explicit permission to elevate privileges:

```bash
sudo apt-get update
sudo apt-get install --no-install-recommends \
  build-essential cmake ninja-build git pkg-config \
  libvulkan-dev vulkan-tools vulkan-validationlayers \
  glslc glslang-tools spirv-tools \
  libglfw3-dev libglm-dev \
  libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev \
  clang clang-format clang-tidy
```

The project uses pinned source archives for GLFW and GLM by default; the distro
packages above also make the host SDK complete and allow independent version
diagnostics. The explicit X11 development packages are required because M0
builds pinned GLFW from source with its X11 backend enabled.

Verify the host after installation:

```bash
cmake --version
ninja --version
g++ --version
clang++ --version
clang-format --version
clang-tidy --version
pkg-config --modversion vulkan glfw3 glm
glslc --version
glslangValidator --version
spirv-val --version
vulkaninfo --summary
```

`vulkaninfo` validates the runtime/driver and may report a display-specific
diagnostic on a headless host. CMake separately validates compile-time headers
and shader tools with actionable error messages.

## Configure, build, and test

```bash
cmake --preset linux-gcc-debug --fresh
cmake --build --preset linux-gcc-debug --parallel
ctest --preset linux-gcc-debug --output-on-failure
```

Other validation presets:

```bash
cmake --preset linux-clang-debug --fresh
cmake --build --preset linux-clang-debug --parallel
ctest --preset linux-clang-debug --output-on-failure

cmake --preset linux-gcc-release --fresh
cmake --build --preset linux-gcc-release --parallel
ctest --preset linux-gcc-release --output-on-failure
```

From a Visual Studio 2022 developer shell on Windows:

```powershell
cmake --preset windows-msvc-debug --fresh
cmake --build --preset windows-msvc-debug --parallel
ctest --preset windows-msvc-debug --output-on-failure
```

## Formatting and static analysis

```bash
cmake --build --preset linux-clang-debug --target format-check
cmake --build --preset linux-clang-debug --target tidy
```

These targets fail with a clear message if `clang-format` or `clang-tidy` is
missing. Fetched third-party sources are excluded.

For repository development only, `ARPG_BOOTSTRAP_NULL_GLFW=ON` permits a
dependency smoke build on a host missing Linux window-system development
headers. Normal presets never enable it, and it does not satisfy the M0 Linux
windowing prerequisite gate.

## Dependencies and offline builds

The approved dependencies are fetched from immutable versioned archives with
SHA-256 verification. After one connected configure, reuse the same build tree
or configure with the download cache populated:

```bash
cmake --preset linux-gcc-debug --fresh -DFETCHCONTENT_FULLY_DISCONNECTED=ON
```

Machine-specific preset overrides belong in ignored `CMakeUserPresets.json`.
Build output belongs in ignored `build/<preset>` directories.

## Repository boundaries

- `core-engine`: renderer-independent engine foundations and later engine
  subsystems
- `arpg-gameplay`: authoritative ARPG rules, depending on core engine
- `apps`: runtime entry points, depending on gameplay
- `dev-tools`: content tools, never a runtime dependency
- `tests`: unit, integration, determinism, and performance suites
- `assets-source`: authoring inputs
- `assets-compiled`: generated runtime assets, not committed by default
