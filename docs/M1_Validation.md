# M1 Validation Status

Date: 2026-07-22

Branch: `m1-platform-loop`

M1 is complete. All required automated, CI, architecture, smoke, manual, and
independent-review gates have passed.

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

## Final Zorin OS desktop lifecycle validation

Final manual validation was completed on Zorin OS with the desktop window
manager active:

- The ARPG Engine window opened and minimized normally.
- During active execution, `arpg_client` used approximately 99–100% of one CPU
  thread, as expected for M1 before rendering or frame limiting exists.
- Once minimized, the process changed from `R` (running) to `S` (sleeping), and
  repeated one-second samples reported 0.0% CPU.
- The window restored normally, with no visible catch-up burst.
- Escape closed the application cleanly; the prior runtime result reported
  `discarded=0`.

## Independent technical review

The Independent Technical Review Board verdict is **PASS** for “M1 – Platform
Layer and Fixed Game Loop.” All required M1 automated, CI, architecture, smoke,
and manual validation gates are satisfied.
