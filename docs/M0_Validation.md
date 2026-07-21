# M0 Validation Status

Date: 2026-07-21

Host: Zorin OS 18.1 (`ubuntu`/`debian` family, Ubuntu codename `noble`),
64-bit x86_64.

## Passed locally

The repository configured, built with project warnings as errors, and passed all
four discovered CTest cases in these configurations:

- GCC 13.3 Debug
- Clang 18.1 Debug
- GCC 13.3 Release

Because elevated package installation was unavailable, these local executions
used:

```bash
-DARPG_VALIDATE_RUNTIME_TOOLS=OFF -DARPG_BOOTSTRAP_NULL_GLFW=ON
```

That combination is deliberately marked as bootstrap-only and does not satisfy
the Linux window/toolchain gate. Dependency archives and hashes were validated,
and GCC Debug also passed a fully disconnected reconfigure after its dependency
cache was populated.

The project sources pass clang-format 18 and clang-tidy 18. The tools were
downloaded and extracted to a temporary directory without installing system
packages.

## Expected strict diagnostic

The unmodified `linux-gcc-debug` preset fails early and clearly because the host
lacks Vulkan development headers:

```text
Vulkan 1.3 development headers and loader were not found.
On Zorin/Ubuntu install: libvulkan-dev.
```

After that package is installed, the same validation will check `glslc`,
`glslangValidator`, `spirv-val`, `vulkaninfo`, and the GLFW X11 development
headers.

## Open gates

- Run the documented APT installation command with elevated permission.
- Run normal GCC Debug/Release and Clang Debug presets without bootstrap
  overrides.
- Run format and tidy through the installed system tools.
- Run the Windows MSVC Debug configure/build/test preset on a Windows runner.

Until these pass, `docs/ROADMAP.md` correctly reports M0 as in progress.
