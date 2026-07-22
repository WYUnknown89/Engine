# M2 Validation Status

Date: 2026-07-22

Branch: `m2-memory-diagnostics`

M2 is complete. All required local, CI, Windows, smoke, benchmark, and
independent-review gates have passed.

## CI clang-tidy correction

The first M2 GitHub Actions run failed only at clang-tidy. The correction keeps
the deliberately ill-formed Release assertion probe as a `try_compile` negative
test but removes it from normal format/tidy registration. It replaces
same-typed constructor parameter lists with named `ConsoleSinkStreams`,
`LinearArenaConfig`, and `FixedBlockPoolConfig` records; replaces the test's
C-style address array with `std::array`; and uses documented line-local
`NOLINT(bugprone-use-after-move)` annotations only where tests verify the
explicit M2 moved-from contract. Fixed-pool metadata is bounded `std::vector`
storage allocated at construction only and released on teardown/move; no
post-construction pool metadata allocation was introduced.

The corrected Clang Debug `tidy` target and direct focused tidy invocation both
passed, as did the complete GCC Debug, GCC Release, GCC Headless, and Xvfb
validation matrix recorded below.

## Local validation

| Configuration or gate | Result | Tests / evidence |
| --- | --- | --- |
| Linux GCC Debug | passed | 43/43 CTest cases (31 unit, 12 integration) |
| Linux Clang Debug | passed | 43/43 CTest cases (31 unit, 12 integration) |
| Linux GCC Release | passed | 42/42 CTest cases (30 unit, 12 integration); the Debug assertion-abort probe is intentionally excluded |
| Linux GCC Headless Debug | passed | 39/39 CTest cases (28 unit, 11 integration) |
| `format-check` | passed | all registered project-owned C++ sources and headers |
| `tidy` | passed | Clang Debug compilation database |
| `git diff --check` | passed | no whitespace errors |

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
ctest --test-dir build/linux-gcc-release --output-on-failure \
  -R '^M2 allocator stress preserves bounded fixed storage$'
./build/linux-gcc-release/tests/arpg_m2_allocator_benchmark

cmake --preset linux-gcc-headless-debug --fresh
cmake --build --preset linux-gcc-headless-debug --parallel
ctest --preset linux-gcc-headless-debug --output-on-failure

xvfb-run -a ./build/linux-gcc-debug/apps/arpg-client/arpg_client --smoke-ticks=5
git diff --check
```

## M2-specific evidence

- The Release assertion test verified that `ARPG_ASSERT` evaluates neither its
  expression nor message in Release. A configure-time negative compilation
  check rejects a non-Boolean assertion expression in Release, preserving the
  compile-time validation contract.
- Allocator tests cover backing-derived alignments from 1 through 64 bytes,
  exact capacity, checked-overflow rejection, invalid release detection,
  reuse, reset, move construction, move assignment, self-move safety, and
  controlled moved-from failures.
- The allocation-instrumented steady-state allocator test recorded zero global
  `new` calls across arena allocation/reset and pool allocation/release/reset
  after initialization. Existing M1 fixed-loop/input allocation tests continue
  to pass.
- The allocator stress test completed 200,000 pool allocate/release operations
  and 200,000 arena allocations with bounded resets. It passed in GCC Release.
- The observed GCC Release benchmark result was
  `operations=1000000 pool_ns=5098534 arena_ns=2427237`. This is a baseline
  measurement only, not a hardware-independent performance threshold.
- Logger tests cover filtering, structured records, bounded sink registration,
  partial sink failure, fatal severity without process termination, reentrant
  dispatch rejection, and subsequent logger reuse.
- Runtime diagnostics tests show optional timing records one fixed tick and one
  frame without changing scheduling results. They remain non-authoritative.

## Desktop smoke

The following passed under Xvfb:

```bash
xvfb-run -a ./build/linux-gcc-debug/apps/arpg-client/arpg_client --smoke-ticks=5
```

The aggregate log reported `exit=1`, `ticks=5`, and `discarded=0`; `1` is the
numeric `requested_stop` exit reason for the bounded smoke option. The client
returned success.

## Final CI, Windows, and review evidence

- The corrected GitHub Actions run passed all approved M2 jobs: Linux GCC
  Debug, Linux Clang Debug with `format-check` and `tidy`, Linux GCC Release
  with allocator stress and benchmark, Linux GCC Headless Debug, and Linux
  GLFW/Xvfb smoke.
- Windows MSVC Debug configure, build, CTest, and bounded `arpg_client` smoke
  passed.
- The Independent Technical Review Board verdict is **PASS** for “M2 – Memory,
  Logging and Diagnostics Foundation.” No corrective implementation findings
  remain.

M3 remains **not started**.
