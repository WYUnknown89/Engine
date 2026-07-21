# ADR-0006 Catch2 and CTest

## Status

Accepted

## Context

The specification requires one C++ test framework to be selected during M0.

## Decision

Use Catch2 v3 for C++ assertions and CTest for discovery, labels, and the
portable test command.

## Consequences

Unit, integration, determinism, and performance tests share CTest orchestration.
Performance executables remain separate from ordinary correctness timing gates.
