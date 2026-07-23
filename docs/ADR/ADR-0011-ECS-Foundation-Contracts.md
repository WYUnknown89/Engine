# ADR-0011 ECS Foundation Contracts

## Status

Accepted

## Context

M3 introduces the first ECS foundation while preserving a headless,
single-threaded core-engine boundary.

## Decision

- Registry owns generational entities, sparse-set pools, deferred commands, and
  typed staged deferred-add values.
- Entity handles use a 32-bit index and 32-bit generation. Generation zero is
  permanently invalid; slots begin at generation one, reuse FIFO free slots,
  and retire permanently at `UINT32_MAX` rather than wrapping.
- Component pools use dense entities/components plus sparse metadata, swap
  removal, and smallest-pool on-demand queries. Structural mutation is deferred
  while iteration is active and flushes only through an explicit API.
- `ComponentTypeId` is equality-only opaque process-local token identity. The
  token is an ODR-safe inline per-type object, not a numeric ID; it is never
  persisted, serialized, ordered, or included directly in state hashes.
  Registration order controls deterministic pool traversal and destruction.
- Deferred add accepts a fully constructed `T` value. A registry-owned typed
  payload slot owns it until explicit flush, discard, or teardown. No deferred
  constructor capture, `std::function`, or per-command allocation is used.
- Standard contiguous containers are used with explicit preparation APIs;
  prepared query/deferred hot paths do not allocate project-owned storage.

## Consequences

Runtime ECS handles and component type tokens are transient implementation
identities. Gameplay systems, world ownership, persistence, scheduling,
parallel execution, and renderer integration remain later milestones.
