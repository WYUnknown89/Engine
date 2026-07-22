# M1 Validation Status

Date: 2026-07-22

Branch: `m1-platform-loop`

M1 remains in progress. This document records only validation that has occurred
locally; it does not claim GitHub Actions, Windows, Xvfb, manual lifecycle, or
independent-review success.

## Formatting correction

The M1 GitHub Actions format-check gate was reported as failed. Local
`format-check` reproduced violations in newly added M1 C++ headers, sources,
the client, and tests. The repository clang-format configuration was applied to
all tracked project C++ source/header files. The final local format-check passes.

GitHub Actions logs could not be fetched locally: the configured GitHub CLI
account is authenticated to `tecman.ghe.com`, while this repository remote uses
`github.com`.

## Local validation

| Configuration | Result | Tests |
| --- | --- | --- |
| Linux GCC Debug | passed | 14/14 |
| Linux Clang Debug | passed | 14/14 |
| Linux GCC Release | passed | 14/14 |
| Linux GCC Headless Debug | passed | 12/12 |
| `format-check` | passed | all registered project sources |
| `tidy` | passed | 12 project source files |

Commands used:

```bash
cmake --preset linux-gcc-debug --fresh
cmake --build --preset linux-gcc-debug --parallel
ctest --preset linux-gcc-debug --output-on-failure

cmake --preset linux-clang-debug --fresh
cmake --build --preset linux-clang-debug --parallel
ctest --preset linux-clang-debug --output-on-failure
cmake --build --preset linux-clang-debug --target format-check
cmake --build --preset linux-clang-debug --target tidy

cmake --preset linux-gcc-release --fresh
cmake --build --preset linux-gcc-release --parallel
ctest --preset linux-gcc-release --output-on-failure

cmake --preset linux-gcc-headless-debug --fresh
cmake --build --preset linux-gcc-headless-debug --parallel
ctest --preset linux-gcc-headless-debug --output-on-failure
```

## Desktop smoke and timing

The host has an active `DISPLAY=:0`; the following GLFW smoke commands passed:

```bash
./build/linux-gcc-debug/apps/arpg-client/arpg_client --smoke-ticks=5
./build/linux-gcc-release/apps/arpg-client/arpg_client --smoke-ticks=600
```

The Debug smoke completed five ticks without discarded backlog. The Release
600-tick run completed in approximately 10.1 seconds with zero discarded ticks,
within the approved 9.8–10.2 second idle-host tolerance.

`xvfb-run` is not installed on this host. Installing `xvfb` requires elevation,
and non-interactive sudo is unavailable, so the required Linux Xvfb smoke has
not been performed.

## Remaining gates

- Linux GLFW/Xvfb smoke.
- Windows MSVC configure/build/test and desktop smoke in CI.
- GitHub Actions rerun after the formatting correction is manually pushed.
- Required manual minimize/restore, input, and minimized-CPU lifecycle checks.
- Independent technical review.

M1 must not be marked complete until every remaining gate passes.
