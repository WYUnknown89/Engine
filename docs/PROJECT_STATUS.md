# Project Status

## Current Milestone

**M2 – Memory, Logging and Diagnostics Foundation**

## Status

**In progress**

M2 implementation is authorized. Do not begin M3.

## Current Branch

`m2-memory-diagnostics`

## Repository

`WYUnknown89/Engine`

## Latest Validated Commit

- `22b34d5dea1cadca758fa887cc866296b0cc065d` – M2: Implement memory
  diagnostics foundation; local validation has passed, while M2 CI, Windows,
  and independent-review closure gates remain open.

## Validation Status

### M1 local validation

- GCC Debug: passed, 25/25 tests (14 unit, 11 integration)
- Clang Debug: passed, 25/25 tests (14 unit, 11 integration)
- GCC Release: passed, 25/25 tests (14 unit, 11 integration)
- GCC Headless Debug: passed, 21/21 tests (11 unit, 10 integration)
- `format-check`: passed after applying the repository clang-format profile to
  every project-owned file in the target, including new files
- `tidy`: passed for 14 project source files
- Linux GLFW/Xvfb smoke: passed for 5 Debug ticks
- Active-display Release 600-tick timing: passed in 10.01 seconds with zero
  discarded backlog (within the 9.8–10.2 second tolerance); the Xvfb timing
  run is retained only as smoke evidence
- Xvfb repeated-resize and synthetic quick Escape press/release checks: passed
- GitHub Actions/Windows MSVC build, test, and smoke: passed
- Final Zorin OS window-manager minimize/restore lifecycle validation: passed;
  minimized process state was `S` with repeated 0.0% CPU samples
- Independent Technical Review Board: passed

### Local Linux

- M2 GCC Debug: passed, 43/43 CTest cases (31 unit, 12 integration)
- M2 Clang Debug: passed, 43/43 CTest cases (31 unit, 12 integration)
- M2 GCC Release: passed, 42/42 CTest cases (30 unit, 12 integration)
- M2 GCC Headless Debug: passed, 39/39 CTest cases (28 unit, 11 integration)
- M2 `format-check`, `tidy`, and `git diff --check`: passed
- M2 GCC Release allocator stress: passed; benchmark observed and recorded in
  `docs/M2_Validation.md`
- M2 Linux GLFW/Xvfb bounded smoke: passed, five ticks and zero discarded
  backlog

### GitHub Actions

- Ubuntu GCC Debug: passed
- Ubuntu Clang Debug: passed
- Windows MSVC Debug: passed configure, build, and 4/4 CTest cases
- Final workflow: [M0 validation #29833829404](https://github.com/WYUnknown89/Engine/actions/runs/29833829404)

## M0 Blockers

None. Local Linux validation, the required Ubuntu/Windows CI jobs, and the
independent technical review have all passed.

## M1 Blockers

None. M1 automated, CI, architecture, smoke, manual, and independent-review
gates have passed.

## M2 Blockers

The first M2 CI run failed only at clang-tidy. The targeted correction and its
complete local validation have passed; do not mark M2 complete until the
corrected GitHub Actions run, Windows MSVC build/test/client smoke, independent
review, and any resulting corrective work pass.

## Last Issue Identified

M0 closed after resolving the Windows Vulkan SDK lookup, Catch2 C++20/string-
view mismatch, and MSVC legacy `__cplusplus` reporting. Their fixes are
recorded in `docs/M0_Validation.md`.

The final MSVC project-options fix adds `/Zc:__cplusplus` centrally without
applying project policy to fetched dependencies.

## Exact Next Action

Commit the targeted M2 clang-tidy correction on `m2-memory-diagnostics`, then
await the corrected GitHub Actions run and independent review. Do not begin M3.

## Next Milestone

**M2 – Memory, Logging and Diagnostics Foundation**

M2 is in progress and awaits its closure gates. M3 must not begin without
explicit approval after M2 closes.

## Agent Handover Instructions

At every meaningful stopping point, update this file so it accurately reflects:

- active milestone
- committed repository state
- validation results
- current blockers
- exact next action

Do not record uncommitted work as completed.
Do not claim CI success until GitHub Actions confirms it.
Do not advance to the next milestone without explicit approval.
