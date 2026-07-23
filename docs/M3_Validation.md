# M3 Validation Status

Date: 2026-07-23

Branch: `m3-ecs`

M3 is in progress. This document records local implementation evidence only;
GitHub CI, Windows validation, and independent committed-code review remain
required before closure.

## Local validation

| Configuration or gate | Result |
| --- | --- |
| Linux GCC Debug | passed, 49/49 CTest cases |
| Linux Clang Debug | passed, 49/49 CTest cases |
| `format-check` | passed |
| `tidy` | passed |
| Linux GCC Release | passed, 48/48 CTest cases |
| Linux GCC Headless Debug | passed, 45/45 CTest cases |
| Linux GLFW/Xvfb smoke | passed, five ticks and zero discarded backlog |
| `git diff --check` | passed |

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
./build/linux-gcc-release/tests/arpg_m3_ecs_benchmark

cmake --preset linux-gcc-headless-debug --fresh
cmake --build --preset linux-gcc-headless-debug --parallel
ctest --preset linux-gcc-headless-debug --output-on-failure

xvfb-run -a ./build/linux-gcc-debug/apps/arpg-client/arpg_client --smoke-ticks=5
git diff --check
```

## M3 benchmark

GCC Release result:

```text
m3_ecs_benchmark positions=1200 moving=1000 warmup_ticks=120 measured_ticks=600 samples=9 median_ns=6778921 p95_ns=7078007 checksum=18642696.000
```

This is baseline evidence, not a hardware-independent pass/fail timing target.

## Pending gates

- Windows MSVC Debug configure/build/test/client smoke.
- GitHub Actions validation, including the Release M3 benchmark job.
- Independent committed-code and architecture review.
- M3 closure documentation and closure commit are intentionally not created.
