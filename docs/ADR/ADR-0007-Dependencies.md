# ADR-0007 Dependency Acquisition

## Status

Accepted

## Context

Linux and Windows need reproducible versions of the approved utility stack.

## Decision

Use CMake FetchContent with release archives and SHA-256 hashes for GLFW, GLM,
Dear ImGui, Catch2, volk, and Vulkan-Headers. Compilers, the Vulkan runtime, and
shader tools remain system prerequisites. No vcpkg, Conan, or CPM dependency is
introduced in M0.

## Consequences

The first configure needs network access or a populated CMake download cache.
Disconnected configuration is supported after dependencies have been cached.
