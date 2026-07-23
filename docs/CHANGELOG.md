# Changelog

## M3 – ECS Foundation Complete

### Completed

-   Added 64-bit generational entities with FIFO reuse and permanent generation
    exhaustion retirement, sparse-set component pools, deterministic
    smallest-pool queries, and ordered explicit deferred structural changes.
-   Added ODR-safe opaque process-local component type identity, test-only
    generation-exhaustion and allocation-failure controls, and prepared
    zero-allocation hot-path validation.
-   Added complete entity, component, query, deferred-command, cross-translation
    unit identity, allocation, and Release benchmark evidence.
-   Local GCC/Clang/Release/headless validation, Linux client smoke, all six
    GitHub Actions jobs in run 30012594722, Windows MSVC build/test/client
    smoke, and independent final implementation/architecture review passed.
-   M3 was merged to `main` by merge commit
    `678c24692cb23a6ad890adca3724750d1696e3fc`; final `main` CI passed and M3 is
    complete.
-   M4 implementation has not begun and is not authorized.

## M2 – Memory, Logging and Diagnostics Foundation Complete

### Completed

-   Added fixed-capacity, aligned `LinearArena` and `FixedBlockPool` allocator
    foundations with checked arithmetic, controlled failures, and deterministic
    moved-from ownership behavior.
-   Added Debug assertions with Release unevaluated compile-time validation,
    synchronous non-throwing structured logging, and optional
    non-authoritative runtime timing metrics.
-   Added allocator allocation-discipline tests, stress coverage, benchmark,
    and explicit Release allocator CI gates.
-   All required local, GitHub Actions, Windows MSVC, Xvfb smoke, benchmark,
    and independent technical-review gates passed.

## M1 – Platform Layer and Fixed Game Loop Complete

### Completed

-   Implemented the platform abstraction, GLFW desktop lifecycle, fixed 60 Hz
    simulation loop, bounded input handling, render interpolation, and
    headless-compatible runtime boundary.
-   Added bounded catch-up, overflow, timing, lifecycle, input-transition, and
    allocation-discipline coverage.
-   Added milestone-neutral CI including Linux headless and GLFW/Xvfb smoke
    gates plus Windows client smoke.
-   All required local, CI, smoke, manual Zorin OS lifecycle, and independent
    technical-review gates passed.

## M0 – Repository Bootstrap Complete

### Completed

-   Reproducible CMake bootstrap, pinned dependency contracts, target
    boundaries, and CTest integration established.
-   Linux GCC Debug, Linux Clang Debug, Linux GCC Release, formatting, and
    static-analysis validation passed.
-   GitHub Actions Ubuntu GCC Debug, Ubuntu Clang Debug, and Windows MSVC
    Debug jobs passed; Windows completed configure, build, and 4/4 CTest cases.
-   Independent technical review approved the M0 implementation.

## Unreleased

### Added

-   Master implementation specification.
-   Pre-implementation audit.
-   M0 CMake bootstrap, dependency contracts, target boundaries, and tests.
-   Contributor and agent workflow guidance.

### Changed

-   Corrected the canonical specification version to v1.1.
-   Accepted the M0 architecture decisions from the pre-implementation audit.

### Fixed

### Removed
