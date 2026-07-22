# Project Status

## Current Milestone

**M1 – Platform Layer and Fixed Game Loop**

## Status

**In progress**

M1 implementation is approved and active. Do not begin M2.

## Current Branch

`m1-platform-loop`

## Repository

`WYUnknown89/Engine`

## Latest Validated Commit

- `bb6b9d07d051e514f4495b6980970f65d43c220e` – M0: Enable conforming MSVC
  `__cplusplus`

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
- Windows CI/smoke, GitHub Actions rerun, window-manager minimize/restore and
  minimized-CPU lifecycle check, and independent review: pending

### Local Linux

- GCC Debug: passed
- Clang Debug: passed
- GCC Release: passed
- `format-check`: passed
- `tidy`: passed
- All discovered tests: 4/4 passed in each validated configuration
- No bootstrap overrides used

### GitHub Actions

- Ubuntu GCC Debug: passed
- Ubuntu Clang Debug: passed
- Windows MSVC Debug: passed configure, build, and 4/4 CTest cases
- Final workflow: [M0 validation #29833829404](https://github.com/WYUnknown89/Engine/actions/runs/29833829404)

## M0 Blockers

None. Local Linux validation, the required Ubuntu/Windows CI jobs, and the
independent technical review have all passed.

## M1 Blockers

- Windows MSVC CI build/test/smoke and the GitHub Actions rerun require the
  user's manual push.
- Required window-manager-backed minimize/restore and minimized-CPU checks
  remain. Raw Xvfb has no window manager and cannot validate this lifecycle
  path; see `docs/M1_Validation.md`.
- Independent technical review remains.

## Last Issue Identified

M0 closed after resolving the Windows Vulkan SDK lookup, Catch2 C++20/string-
view mismatch, and MSVC legacy `__cplusplus` reporting. Their fixes are
recorded in `docs/M0_Validation.md`.

The final MSVC project-options fix adds `/Zc:__cplusplus` centrally without
applying project policy to fetched dependencies.

## Exact Next Action

Commit and push the targeted independent-review corrections manually, then
obtain the GitHub Actions and Windows evidence. Complete the remaining
window-manager lifecycle check and independent review before closing M1.

## Next Milestone

**M2 – Memory, Logging and Diagnostics Foundation**

M2 must not begin until every M1 validation, smoke, CI, documentation, and
independent-review gate has passed.

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
