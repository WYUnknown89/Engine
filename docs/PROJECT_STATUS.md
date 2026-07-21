# Project Status

## Current Milestone

**M0 – Repository and Build Bootstrap**

## Status

**In progress**

M1 has not started and is not yet approved.

## Current Branch

`main`

## Repository

`WYUnknown89/Engine`

## Latest Known Commits

- `d897b0f` – M0: Align Catch2 with C++20 consumers
- `74ee001` – M0: Record Catch2 CI rerun gate

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
- Windows MSVC Debug: builds successfully after the Catch2 C++20 configuration
  fix, but its build-contract test reports MSVC's legacy `__cplusplus` value
  (`199711`)

## Current Blocker

The final M0 gate remains open until the Windows MSVC Debug GitHub Actions job
passes on the latest pushed commit.

## Last Issue Identified

Windows MSVC's `__cplusplus` reports `199711` unless `/Zc:__cplusplus` is
enabled. This caused the M0 build-contract test to fail after the Catch2 link
issue was corrected.

The central project-options fix:

- adds `/Zc:__cplusplus` to `arpg::project_options`
- propagates to core engine, gameplay, tools, application facade, and tests
- leaves `build_info.cpp` reading `__cplusplus`
- does not apply project options to fetched dependencies
- does not weaken or remove the existing tests

## Exact Next Action

1. Commit the central MSVC conformance fix locally and push it manually.
2. Confirm that all three jobs pass:
   - Ubuntu GCC Debug
   - Ubuntu Clang Debug
   - Windows MSVC Debug
3. If Windows fails, inspect and fix only the first genuine error.
4. If all jobs pass:
   - update `docs/M0_Validation.md`
   - update `docs/ROADMAP.md` to mark M0 complete
   - update this file
   - commit and push the final M0 closure
   - request independent technical review before starting M1

## Next Milestone

**M1 – Platform Layer and Fixed Game Loop**

M1 must not begin until:

- M0 is marked complete in the repository
- all required CI jobs pass
- the independent technical review approves M0

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
