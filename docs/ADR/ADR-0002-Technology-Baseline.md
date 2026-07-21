# ADR-0002 Technology Baseline

## Status

Accepted

## Context

The engine requires a portable, explicit stack that does not delegate engine
ownership to a third-party game engine.

## Decision

Use 64-bit C++20, CMake 3.28 or newer, and Vulkan 1.3. Linux GCC/Clang and
Windows MSVC are first-class compiler families. C++ compiler extensions are
disabled.

## Consequences

Changing language, build system, graphics API, or the 64-bit platform baseline
requires a project-level decision.
