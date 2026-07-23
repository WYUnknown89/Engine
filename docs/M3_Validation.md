# M3 Validation Status

Date: 2026-07-23

Branch: `m3-ecs`

M3 is in progress. The complete corrective contract-test matrix and local
validation have passed. GitHub CI, Windows validation, and independent
committed-code review remain required before closure.

## Local validation

| Configuration or gate | Result |
| --- | --- |
| Linux GCC Debug | passed, 73/73 CTest cases |
| Linux Clang Debug | passed, 73/73 CTest cases |
| `format-check` | passed |
| `tidy` | passed |
| Linux GCC Release | passed, 72/72 CTest cases |
| Dedicated M3 allocation test | passed, transactional failure preserved state and prepared hot path allocated zero times |
| Linux GCC Headless Debug | passed, 69/69 CTest cases |
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
./build/linux-gcc-release/tests/arpg_m3_ecs_allocation_tests
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
m3_ecs_benchmark compiler=13.3.0 build=Release positions=1200 moving=1000 warmup_ticks=120 measured_ticks=600 samples=9 total_visits=5400000 expected_visits=5400000 visits_verified=true median_batch_ns=1081429 p95_batch_ns=1204471 median_ns_per_tick=1802.382 visits_per_second=554821444.589 measured_allocations=0 checksum=18642696.000 expected_checksum=18642696.000 checksum_tolerance=186.427 checksum_verified=true
```

This is baseline evidence, not a hardware-independent pass/fail timing target.
The checksum tolerance is the greater of 0.5 and 0.001% of the expected
single-precision aggregate; it accommodates compiler/platform floating-point
accumulation variation while still detecting material workload divergence.

## Corrective evidence

- The M3 Catch2 executable passed 455 assertions across 29 focused test cases,
  covering entity lifetime and retirement, sparse-set component lifetime,
  deterministic queries and nesting, and ordered deferred-command conflicts.
- A friend-only test accessor forces generation exhaustion and inspects internal
  state without exposing a runtime ECS test-control API.
- The isolated allocation executable forced failure during sparse-pool growth
  after a component pool existed, verified transactional creation recovery, and
  reported:

  ```text
  m3_ecs_allocation_tests transactional_sparse_growth_failure=true prepared_hot_path_allocations=0 zero_allocations_verified=true
  ```

- Queries resolve all requested component-pool pointers once before traversal;
  per-entity membership and retrieval use those direct pointers.
- Entity creation reserves corresponding FIFO reuse capacity, so later
  destruction does not allocate.
- Prepared deferred-add capacity is checked at enqueue and regression-tested
  after immediate capacity consumption.
- Allocation instrumentation recorded zero global allocations across prepared
  multi-component iteration, value mutation, deferred submission, and flush.
- The GLFW/Xvfb smoke run reported five ticks and `discarded=0`.

## Pending gates

- Windows MSVC Debug configure/build/test/client smoke.
- GitHub Actions validation, including the Release M3 benchmark job.
- Independent committed-code and architecture review.
- M3 closure documentation and closure commit are intentionally not created.
