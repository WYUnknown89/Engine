# ADR-0008 Target Topology

## Status

Accepted

## Context

Repository folders alone do not prevent reverse dependencies.

## Decision

The link direction is core engine to gameplay to applications. Tools depend on
core engine and shared asset contracts, not runtime applications. Public include
directories and links are target-scoped and validated during configuration.

## Consequences

Core engine cannot link gameplay or tools. Gameplay public headers cannot
expose Vulkan. Runtime targets cannot link developer tools.
