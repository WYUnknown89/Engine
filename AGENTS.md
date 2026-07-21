# Agent Instructions

The canonical contract is `docs/Master_Implementation_Specification.md`,
version 1.1. The PDF is secondary. Also read `docs/PROJECT_RULES.md`,
`docs/ENGINE_BIBLE.md`, `docs/Architecture_Decisions.md`, and the current audit
before changing code.

## Milestone discipline

- Work only on the explicitly approved milestone.
- Inspect, plan, implement, format/lint, build, test, smoke test where relevant,
  verify acceptance criteria, document, and report.
- Never claim placeholders, empty behavior, or unverified functionality as
  complete.
- Do not advance while the current milestone gate is open.

## Architecture

- Dependency direction is core engine -> gameplay -> applications.
- Tools may use core engine/shared format contracts but never become runtime
  dependencies.
- Gameplay owns authoritative state and must not expose Vulkan types.
- Rendering, UI, audio, and VFX are non-authoritative consumers.
- Persisted formats are explicit, versioned, validated, and never native-memory
  dumps.
- Add only dependencies approved by the specification or an accepted ADR.

## Validation

```bash
cmake --preset linux-gcc-debug --fresh
cmake --build --preset linux-gcc-debug --parallel
ctest --preset linux-gcc-debug --output-on-failure

cmake --preset linux-clang-debug --fresh
cmake --build --preset linux-clang-debug --parallel
ctest --preset linux-clang-debug --output-on-failure

cmake --build --preset linux-clang-debug --target format-check
cmake --build --preset linux-clang-debug --target tidy
```

Windows validation uses the `windows-msvc-debug` configure, build, and test
presets from a Visual Studio 2022 developer environment.
