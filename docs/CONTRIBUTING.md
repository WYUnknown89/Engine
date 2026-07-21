# CONTRIBUTING.md

# AI & Contributor Workflow

This repository is developed primarily with AI assistance under human
review.

This document defines **how work is performed**. It complements, but
never overrides:

1.  `docs/Master_Implementation_Specification.md`
2.  `docs/ENGINE_BIBLE.md`
3.  `docs/PROJECT_RULES.md`
4.  `docs/Architecture_Decisions.md`

------------------------------------------------------------------------

# Mission

Deliver a high-quality custom C++20/Vulkan engine through disciplined,
incremental milestones.

Correctness comes before speed.

------------------------------------------------------------------------

# Before Writing Code

Every session must:

-   Read the Master Specification.
-   Read the Engine Bible.
-   Read Project Rules.
-   Read Architecture Decisions.
-   Read the current Pre-Implementation Audit.
-   Identify the active milestone.

Never assume previous context if it is not documented.

------------------------------------------------------------------------

# Standard Workflow

For every task:

1.  Inspect existing code.
2.  Understand the specification.
3.  Produce a short implementation plan.
4.  Implement only the approved scope.
5.  Build.
6.  Run tests.
7.  Fix regressions.
8.  Update documentation if architecture changed.
9.  Report results.

Never skip build or test unless impossible.

------------------------------------------------------------------------

# Milestone Discipline

Only work on the currently approved milestone.

Do not begin another milestone because it "looks easy".

If the current milestone gate has not passed, remain in the current
milestone.

------------------------------------------------------------------------

# Code Quality

Prefer:

-   Small commits
-   Clear naming
-   Explicit ownership
-   RAII
-   Measurable performance

Avoid:

-   Premature optimisation
-   Hidden allocations
-   Clever but unreadable code
-   Global mutable state

------------------------------------------------------------------------

# Architecture Rules

Never:

-   Introduce reverse dependencies.
-   Leak Vulkan into gameplay.
-   Mix rendering and simulation.
-   Invent undocumented file formats.
-   Break determinism without approval.

If a better architecture is discovered:

1.  Stop.
2.  Explain it.
3.  Show trade-offs.
4.  Wait for approval.

------------------------------------------------------------------------

# Dependencies

Do not add libraries because they are convenient.

Every dependency must include:

-   justification
-   licence compatibility
-   maintenance status
-   architectural impact

------------------------------------------------------------------------

# Build Rules

Every meaningful change should end with:

-   Configure
-   Build
-   Test

Record the exact commands used.

------------------------------------------------------------------------

# Testing Rules

Never delete or weaken tests to obtain a passing build.

If a test fails:

-   determine root cause
-   fix code or update the specification if the behaviour intentionally
    changed

------------------------------------------------------------------------

# Progress Reports

Every completed task should include:

## Summary

What changed.

## Files

Files created or modified.

## Validation

Build command.

Test command.

Results.

## Risks

Known issues.

## Next Step

The single recommended next task.

------------------------------------------------------------------------

# Blockers

Stop and ask only when:

-   architecture is genuinely ambiguous
-   external credentials are required
-   destructive migration is required
-   specification conflict exists
-   approval is explicitly required

Do **not** stop for routine implementation decisions that can be
inferred safely.

------------------------------------------------------------------------

# Documentation

Whenever architecture changes:

-   Update Architecture Decisions.
-   Update the Engine Bible if philosophy changes.
-   Update the Master Specification only when requirements change.
-   Update the Changelog.

------------------------------------------------------------------------

# Git

Recommended commit style:

M0: Bootstrap repository

M3: Implement sparse-set ECS registry

M8: Add deterministic A\* navigation

Use concise, imperative commit messages.

------------------------------------------------------------------------

# Definition of Success

A task is complete only when:

-   code builds
-   relevant tests pass
-   acceptance criteria pass
-   documentation is current
-   no known regression has been introduced

Working code is not enough.

The implementation must remain understandable, maintainable and aligned
with the long-term vision of the engine.
