# ADR-0003 Authority and Networking Direction

## Status

Accepted

## Context

The initial game is single-player, but authoritative gameplay must not become
coupled to local presentation.

## Decision

Gameplay simulation owns authoritative state. Player intentions cross into the
simulation as tick-indexed commands. Rendering, UI, audio, and VFX are
non-authoritative consumers. No networking is implemented in the current
roadmap, but serializable state and command boundaries preserve a future
authoritative-server path.

## Consequences

The engine is not designed around deterministic peer-to-peer lockstep, and
presentation code cannot directly mutate authoritative gameplay state.
