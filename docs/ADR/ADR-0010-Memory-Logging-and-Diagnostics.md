# ADR-0010 Memory, Logging, and Diagnostics Contracts

## Status

Accepted

## Context

M2 provides bounded allocator foundations, assertions, structured logging, and
timing diagnostics without altering the authoritative M1 simulation contract.
These facilities must be usable by headless applications and future tools while
remaining independent of gameplay, GLFW, Vulkan, and dev-tools.

## Decision

- `LinearArena` and `FixedBlockPool` are move-only RAII owners with fixed,
  initialization-time backing allocations. They never grow or fall back to the
  heap after construction.
- Allocator construction uses named configuration records so capacity, alignment,
  and block-count arguments cannot be accidentally reordered. Fixed-pool slot
  metadata is bounded and allocated only during construction; reset, allocate,
  and release never grow it.
- Arena backing alignment is at least its configured maximum alignment. Pool
  backing alignment is at least its block alignment, and checked stride
  round-up ensures every block address is aligned.
- Allocator arithmetic is checked. Runtime allocation and release operations
  return explicit status rather than throwing. Raw storage does not own object
  destructors; callers destroy non-trivial objects before reset or destruction.
- Moved-from allocators are empty, non-owning, safely destructible, safely
  assignable, and return an unavailable status from allocation or release.
- Assertions abort only in Debug. Release assertions do not evaluate their
  expression or message, but retain both in an unevaluated type-checking
  context so ill-formed assertions cannot be hidden by a Release build.
- Logging is synchronous, single-threaded, structured, and non-throwing.
  Sinks return explicit write status. Sink failures neither recurse through the
  logger nor terminate the process; fatal is a severity, not control flow.
- Assertions remain independent of logging and use an emergency stderr path.
- Timing diagnostics use monotonic clocks, are optional and non-authoritative,
  and must not affect scheduling, input, tick indexes, or simulation results.
- M2 adds no asynchronous logging, global allocator replacement, PMR adapters,
  typed allocator helpers, canaries, poisoning, renderer diagnostics, or ECS.

## Consequences

Applications own clocks, sinks, loggers, metrics, and allocators according to
their required lifetime. Runtime borrows optional diagnostics only. The
foundation remains headless-compatible and establishes explicit allocation,
failure, and diagnostic contracts for future measured consumers.
