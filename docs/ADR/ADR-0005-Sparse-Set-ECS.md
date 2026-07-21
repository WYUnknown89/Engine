# ADR-0005 Sparse-set ECS

## Status

Accepted

## Context

The first ECS needs contiguous component iteration and generational entity
lifetime without the complexity of a complete archetype implementation.

## Decision

Use 64-bit generational entity handles and sparse-set component pools. Apply
structural changes at explicit, deterministically ordered phase barriers.

## Consequences

Component references are invalidated by structural mutation of their pool.
Runtime entity handles are never persisted as stable gameplay identifiers.
