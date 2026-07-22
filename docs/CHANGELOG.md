# Changelog

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
