# Project Status

## Current Milestone

**M4 – Vulkan Renderer**

## Status

**Not started / planning gate**

M0 through M3 are complete. M3 implementation, validation, independent review,
closure-branch CI, manual merge, and final `main` CI have passed. `main` is the
current known-good M3-complete baseline. M4 implementation is not authorized.

## Current Branch

`main`

## Repository

`WYUnknown89/Engine`

## Latest Validated Commit

- `678c24692cb23a6ad890adca3724750d1696e3fc` – Merge branch `m3-ecs`; the
  M3-complete `main` baseline passed final
  [Engine validation run 30014888615](https://github.com/WYUnknown89/Engine/actions/runs/30014888615).

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

### M3 local validation

- GCC Debug: passed, 73/73 CTest cases
- Clang Debug: passed, 73/73 CTest cases
- GCC Release: passed, 72/72 CTest cases
- GCC Headless Debug: passed, 69/69 CTest cases
- `format-check`, `tidy`, and `git diff --check`: passed
- Dedicated prepared-hot-path allocation test: passed with zero measured
  allocations and transactional sparse-growth failure recovery
- Release benchmark: expected 5,400,000 visits and deterministic checksum
  verified; full timing evidence is recorded in `docs/M3_Validation.md`
- Linux GLFW/Xvfb bounded smoke: passed, five ticks and zero discarded backlog

### GitHub Actions

- Ubuntu GCC Debug: passed
- Ubuntu Clang Debug: passed
- Windows MSVC Debug: passed configure, build, and 4/4 CTest cases
- Final workflow: [M0 validation #29833829404](https://github.com/WYUnknown89/Engine/actions/runs/29833829404)

### M3 final validation

- [Engine validation run 30012594722](https://github.com/WYUnknown89/Engine/actions/runs/30012594722):
  passed Ubuntu GCC Debug, Ubuntu Clang Debug, Ubuntu GCC Release, Ubuntu GCC
  Headless Debug, Linux GLFW Client Smoke, and Windows MSVC Debug
- Windows MSVC Debug: passed configure, build, CTest, and bounded client smoke
- Independent final committed-code/architecture review: passed for commit
  `31cc4d54480af261685766c3ecd4385f3d5c2ef5`
- Closure-branch
  [Engine validation run 30014140525](https://github.com/WYUnknown89/Engine/actions/runs/30014140525):
  passed all six jobs for commit
  `96b240890f99d318b4fabc1aafbc5d830abf822c`
- M3 was manually merged to `main` by merge commit
  `678c24692cb23a6ad890adca3724750d1696e3fc`
- Final `main`
  [Engine validation run 30014888615](https://github.com/WYUnknown89/Engine/actions/runs/30014888615):
  passed all six jobs

### M2 final validation

- Corrected GitHub Actions matrix: passed, including Linux GCC Debug, Linux
  Clang Debug with format/tidy, Linux GCC Release with allocator stress and
  benchmark, Linux GCC Headless Debug, Linux GLFW/Xvfb smoke, and Windows MSVC
  Debug build/test/client smoke
- Independent Technical Review Board: passed

## M0 Blockers

None. Local Linux validation, the required Ubuntu/Windows CI jobs, and the
independent technical review have all passed.

## M1 Blockers

None. M1 automated, CI, architecture, smoke, manual, and independent-review
gates have passed.

## M2 Blockers

None. M2 local, CI, Windows, smoke, benchmark, and independent-review gates
have passed.

## M3 Blockers

None. M3 implementation, local validation, branch CI, Windows validation,
independent review, closure-branch CI, manual merge, and final `main` CI passed.

## Last Issue Identified

Independent M3 review identified transactional entity creation, const-query
iteration protection, and contract-test coverage gaps. Commits through
`31cc4d54480af261685766c3ecd4385f3d5c2ef5` corrected them, and the independent
final review passed with no remaining implementation findings.

## Exact Next Action

Prepare the M4 plan, obtain independent architecture review, and secure explicit
approval before creating an M4 branch or beginning implementation.

## Next Milestone

**M4 – Vulkan Renderer**

**Not started / planning gate.** M4 implementation must not begin until its
planning and independent architecture-review gates pass and the plan is
explicitly approved.

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
