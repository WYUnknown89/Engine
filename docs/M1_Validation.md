# M1 Validation Status

Date: 2026-07-22

Branch: `m1-platform-loop`

M1 remains in progress. This document records only validation that has occurred
locally; it does not claim GitHub Actions, Windows, or independent-review
success.

## Formatting correction

The independent review identified a GitHub Actions `format-check` failure.
The root cause was M1 source and test files that had not been passed through the
repository clang-format configuration. The correction applies clang-format to
every project-owned source/header file named by the CMake `format-check` target,
including newly created (previously untracked) files. The final local
`format-check` passes.

GitHub Actions logs could not be fetched locally: the configured GitHub CLI
account is authenticated to `tecman.ghe.com`, while this repository remote uses
`github.com`.

## Local validation

| Configuration | Result | Tests |
| --- | --- | --- |
| Linux GCC Debug | passed | 25/25 (14 unit, 11 integration) |
| Linux Clang Debug | passed | 25/25 (14 unit, 11 integration) |
| Linux GCC Release | passed | 25/25 (14 unit, 11 integration) |
| Linux GCC Headless Debug | passed | 21/21 (11 unit, 10 integration) |
| `format-check` | passed | all registered project sources |
| `tidy` | passed | 14 project source files |

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

## Desktop smoke, timing, and lifecycle checks

The following GLFW smoke commands passed under Xvfb:

```bash
xvfb-run -a ./build/linux-gcc-debug/apps/arpg-client/arpg_client --smoke-ticks=5
xvfb-run -a ./build/linux-gcc-release/apps/arpg-client/arpg_client --smoke-ticks=600
```

The Debug smoke completed five ticks without discarded backlog. The Release
600-tick Xvfb run completed in 12 seconds with zero discarded ticks. Xvfb is
therefore retained as the desktop smoke environment, not as a timing benchmark.
On the active desktop display, the following measured timing check passed:

```bash
/usr/bin/time -f 'elapsed_seconds=%e' \
  ./build/linux-gcc-release/apps/arpg-client/arpg_client --smoke-ticks=600
```

It completed 600 ticks in 10.01 seconds with zero discarded backlog, within the
approved 9.8–10.2 second idle-host tolerance.

Additional Xvfb interaction checks passed:

- Four successive X11 resizes completed without a runtime failure; the client
  then stopped through a synthetic Escape press/release (`requested_stop`, 20
  ticks, zero discarded backlog).
- A synthetic quick Escape press/release stopped the client cleanly
  (`requested_stop`, two ticks, zero discarded backlog).

The raw Xvfb server has no window manager. An attempted ten-second
`XIconifyWindow` check did not deliver an iconify state to GLFW: the process
continued at 100% CPU and completed 622 ticks. This is a limitation of that
manual-test harness, not evidence of a busy-wait defect in the minimized path.
The genuine window-manager minimize/restore check remains required: minimize
for at least ten seconds, confirm event-wait suspension and low CPU, restore
without a catch-up burst, and close the window normally. The deterministic
runtime integration tests cover the equivalent suspended/wait/reset behaviour.

## Remaining gates

- Windows MSVC configure/build/test and desktop smoke in CI.
- GitHub Actions run of the milestone-neutral workflow after the correction is
  manually pushed.
- Window-manager-backed ten-second minimize/restore and minimized-CPU check.
- Independent technical review.

M1 must not be marked complete until every remaining gate passes.
