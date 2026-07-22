# ADR-0009 Platform Loop, Input, and Lifetime Contracts

## Status

Accepted

## Context

M1 establishes the desktop platform boundary and the first authoritative game
loop. These contracts must remain suitable for deterministic simulation and a
future headless application without allowing platform or presentation concerns
to leak into gameplay.

## Decision

- The authoritative simulation frequency is permanently fixed at 60 Hz. It is
  compile-time metadata, never application or runtime configuration. Future
  gameplay speed changes use game-time scaling above the scheduler.
- Tick index is authoritative. Wall time is used only by the outer accumulator
  and render interpolation.
- The single-threaded M1 loop uses a bounded accumulator with a default maximum
  of eight catch-up ticks and a 250 ms input wall-time clamp. Excess whole-tick
  backlog is discarded while preserving the fractional remainder.
- Project-owned fixed-loop and input infrastructure performs no unbounded heap
  allocation in normal steady-state execution after initialization. Input
  transitions use fixed, preallocated storage; overflow is a controlled error.
  Initialization and exceptional diagnostic paths may allocate.
- GLFW is isolated in a dedicated platform target. Core and gameplay public
  contracts expose no GLFW or Vulkan types or headers.
- The desktop client pauses simulation/render callbacks while iconified or when
  the framebuffer extent is zero, waits for events without busy-waiting, and
  resets wall-time scheduling state on restoration. Headless operation is not
  affected by this desktop policy.
- Runtime ownership is explicit. Construction is clock, GLFW platform, client,
  then loop; destruction is the reverse. The loop becomes quiescent before
  borrowed client or platform objects are destroyed. GLFW window destruction
  always precedes GLFW termination.

## Consequences

Application configuration cannot alter the authoritative tick rate. Gameplay
must receive platform-neutral tick-boundary input/commands rather than polling
GLFW. The core loop can be tested with fake clocks and platforms without opening
a window or initializing Vulkan. Shutdown and partial-construction paths are
tested as first-class behavior.
