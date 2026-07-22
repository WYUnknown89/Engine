# Project Status

## Current Milestone

**M1 – Platform Layer and Fixed Game Loop**

## Status

**In progress**

M1 implementation is approved and active. Do not begin M2.

## Current Branch

`main`

## Repository

`WYUnknown89/Engine`

## Latest Validated Commit

- `bb6b9d07d051e514f4495b6980970f65d43c220e` – M0: Enable conforming MSVC
  `__cplusplus`

## Validation Status

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

## Last Issue Identified

M0 closed after resolving the Windows Vulkan SDK lookup, Catch2 C++20/string-
view mismatch, and MSVC legacy `__cplusplus` reporting. Their fixes are
recorded in `docs/M0_Validation.md`.

The final MSVC project-options fix adds `/Zc:__cplusplus` centrally without
applying project policy to fetched dependencies.

## Exact Next Action

Implement M1 platform boundaries, GLFW lifecycle, bounded input capture, and
the fixed 60 Hz loop. Keep `docs/M1_Validation.md` and this status current.

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
