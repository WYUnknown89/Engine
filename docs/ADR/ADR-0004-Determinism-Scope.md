# ADR-0004 Determinism Scope

## Status

Accepted

## Context

Deterministic replay needs a precise initial guarantee without blocking the
vertical slice on cross-platform floating-point identity.

## Decision

Identical initial state, explicit random seeds, and a tick-indexed command
stream must produce identical simulation results for the same engine version,
build configuration, target platform, and CPU architecture. Replays are
versioned; incompatible versions fail safely. Cross-platform bit identity and
cross-version migration are not initial requirements.

## Consequences

Simulation avoids unnecessary platform-dependent behavior, uses explicit PRNG
algorithms, and hashes canonical authoritative fields rather than object memory.
