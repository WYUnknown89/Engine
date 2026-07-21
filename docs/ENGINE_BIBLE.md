# ENGINE_BIBLE.md

# Engine Bible

Version: 1.0

## Purpose

This document is the long-term engineering philosophy for the ARPG
Engine.

Unlike the Master Implementation Specification, this document explains
*why* decisions are made rather than *what* must be built.

If a future implementation decision is technically valid but contradicts
the principles in this document, the principles take precedence until
deliberately revised.

------------------------------------------------------------------------

# Vision

Build a bespoke, high-performance C++20/Vulkan engine designed
specifically for one genre:

**3D Isometric Action RPGs.**

This is **not** intended to become a general-purpose engine.

Every subsystem exists only if it improves the intended game.

------------------------------------------------------------------------

# Core Philosophy

## Simplicity over cleverness

Prefer code that is easy to understand and debug.

Avoid unnecessary abstraction.

If two solutions have similar performance, choose the simpler one.

------------------------------------------------------------------------

## Measure before optimising

Never optimise because something "might" be slow.

Profile first.

Optimise only after evidence exists.

------------------------------------------------------------------------

## Determinism

Gameplay must be deterministic within the supported replay scope.

Rendering is never authoritative.

Simulation owns the truth.

------------------------------------------------------------------------

## Separation of Responsibilities

Renderer - Draws. - Never owns gameplay.

Gameplay - Owns simulation. - Never depends on Vulkan.

Tools - Produce content. - Never become runtime dependencies.

------------------------------------------------------------------------

# Performance Philosophy

Optimise for:

-   predictable frame times
-   cache locality
-   contiguous memory
-   low allocations
-   simple ownership

Avoid:

-   unnecessary virtual dispatch
-   hidden allocations
-   global mutable state

------------------------------------------------------------------------

# Memory Philosophy

Prefer:

-   RAII
-   std::unique_ptr
-   std::vector
-   std::pmr where justified

Avoid:

-   raw owning pointers
-   manual lifetime unless required
-   premature custom allocators

------------------------------------------------------------------------

# ECS Philosophy

Components contain data.

Systems contain behaviour.

Entities are identifiers only.

Systems communicate using explicit state or events.

------------------------------------------------------------------------

# Renderer Philosophy

Gameplay never owns Vulkan objects.

Renderer resources are referenced through engine handles.

Swapchain recreation must never leak resources.

Correctness before optimisation.

------------------------------------------------------------------------

# Asset Philosophy

Every asset has:

-   stable ID
-   version
-   validation
-   deterministic loading

Never depend on absolute paths.

------------------------------------------------------------------------

# Testing Philosophy

Every major subsystem should be testable independently.

Regression tests are never removed merely to obtain a passing build.

Performance benchmarks are separate from correctness tests.

------------------------------------------------------------------------

# AI Collaboration Rules

Every AI session must:

1.  Read the Master Specification.
2.  Read PROJECT_RULES.md.
3.  Read Architecture Decisions.
4.  Read this Engine Bible.
5.  Work only on the approved milestone.

Never silently change architecture.

Never invent completed functionality.

Always explain assumptions.

------------------------------------------------------------------------

# Decision Process

When multiple valid implementations exist:

1.  Correctness
2.  Maintainability
3.  Determinism
4.  Performance
5.  Complexity

Performance never overrides correctness.

------------------------------------------------------------------------

# Long-Term Goals

The engine should remain:

-   understandable
-   maintainable
-   deterministic
-   performant
-   enjoyable to extend

It is acceptable for the engine to support only the intended ARPG genre
if that produces a significantly better architecture.

Generality is not a goal.
