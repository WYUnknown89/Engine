# Pre-implementation Audit and M0 Execution Plan

## Audit basis

- Canonical source reviewed in full: [Master_Implementation_Specification.md](/home/joel/Documents/Games/Engine/docs/Master_Implementation_Specification.md).
- Secondary reference inspected: [v1.1 PDF](/home/joel/Documents/Games/Engine/docs/ARPG_Engine_Master_Implementation_Specification_v1.1.pdf).
- Every repository file was inspected. These are currently the only two files; no implementation, build files, Git metadata, or hidden project configuration exists.
- The Markdown contains all 29 specification sections but still displays “Version 1.0” on line 3. The agreed canonical version is v1.1.
- Host: 64-bit Zorin OS 18.1, Ubuntu 24.04 “noble” base, using APT.

# A. Specification Audit

| ID | Relevant section / severity | Exact problem | Engineering consequence | Recommended correction |
|---|---|---|---|---|
| A01 | Header; **high** | Canonical Markdown and PDF body display v1.0 despite the confirmed v1.1 contract. | Reports and agents can apply the wrong revision. | Change the Markdown label to v1.1 and regenerate/correct the secondary PDF so both visible labels agree. |
| A02 | §§3, 24; **high** | The repository topology omits an application directory even though dependency rules mention applications and M1 requires a runnable application. | The executable may be placed in gameplay or engine code, weakening boundaries. | Add `apps/arpg-client` and later `apps/arpg-headless`; applications depend on gameplay/engine facades, never the reverse. |
| A03 | §3; **medium** | “Tools may depend on shared asset-format libraries,” but no shared-format ownership location is defined. | Tools may duplicate schemas or force runtime dependencies on tool code. | Place wire-format declarations and validation in a renderer-independent `core-engine/assets` library consumable by runtime and tools. |
| A04 | §§1, 2, 26; **high** | Third-party libraries are restricted, but dependency acquisition, pinning, licence records, update policy, and offline behaviour are unspecified. | Builds can drift or silently acquire incompatible versions. | Use CMake `FetchContent` with immutable release archives and SHA-256 hashes for approved source dependencies; retain system Vulkan/toolchain prerequisites and a third-party notices file. |
| A05 | §2; **medium** | Catch2 versus GoogleTest is unresolved. | Test layout and discovery remain ambiguous. | Lock Catch2 v3 for unit/integration tests and CTest as the universal runner. |
| A06 | §§2, 25; **high** | Supported compiler versions, CMake minimum, generators, warning profiles, 64-bit enforcement, and build configurations are unspecified. | “Builds on Linux and Windows” is not reproducible or measurable. | Lock CMake 3.28+, GCC 13+, Clang 18+, MSVC 19.38+/VS 2022, C++ extensions off, and fail configuration on non-64-bit targets. |
| A07 | §§3, 24; **high** | Dependency direction is prose only and no target topology or namespace policy enforces it. | Include-path leakage and reverse links can introduce architectural cycles. | Use target-scoped include directories and links, with `arpg_core_engine`, `arpg_gameplay`, application, and tool facades; add configure-time graph assertions. |
| A08 | §§4, 25; **medium** | “Warnings treated seriously” does not define warning-as-error policy or third-party warning treatment. | Builds may pass with new project warnings or fail on external-library warnings. | Enable strict warnings for project targets, suppress warnings only for imported third-party targets, and use warnings-as-errors in CI/presets. |
| A09 | §§6, 22; **high** | Headless simulation is required “eventually” but has no milestone assignment. | ECS, replay, and determinism tests could accidentally depend on GLFW or Vulkan. | Establish headless-compatible engine/application boundaries in M1; no engine simulation target may link the renderer or window layer transitively. |
| A10 | §§5, 6 plus confirmed decision; **critical** | The original text did not define the scope of determinism or authoritative/presentation boundaries. | Numeric, replay, and event designs could be incompatible with the intended future server model. | Adopt the confirmed same-version/build/config/platform/CPU guarantee and authoritative-server-compatible command/state boundaries. |
| A11 | §6; **high** | “Exactly 1/60 second” does not identify the authoritative time representation. Binary floating point cannot represent that rational exactly. | Different systems may accumulate time differently. | Make tick index the authoritative simulation clock. Express rates per tick or from a single rational `1/60`; use floating-point wall time only in the outer accumulator and interpolation. |
| A12 | §6; **high** | The maximum catch-up steps and backlog-discard policy are configurable but have no defaults or diagnostics. | Long stalls can cause spirals, permanent latency, or inconsistent command scheduling. | Default to eight catch-up ticks, cap incoming wall-time delta, discard excess accumulated wall time after the cap, and emit an overload metric/log. Replay remains command/tick driven. |
| A13 | §§6, 29.1; **high** | Simulation behaviour while minimized, paused, or unfocused is explicitly required but undefined. | Restore can trigger large catch-up bursts or unintended gameplay progression. | For the single-player client, pause wall-clock-driven simulation while minimized, wait for events without busy-looping, and reset the accumulator timestamp on restore. Headless simulation remains unaffected. |
| A14 | §29.2; **high** | Event-to-tick assignment, ordering of same-tick events, continuous sampling, and behaviour across multiple catch-up ticks are not exact. | One-shot inputs may duplicate, disappear, or replay differently. | Sequence platform events monotonically, translate them once into the next produced tick snapshot, retain held state across ticks, and consume edge events once. Tick-indexed gameplay commands are the replay boundary. |
| A15 | §§5, 22, 29.2; **high** | Replay format, compatibility rejection, command ordering, and canonical state hashing are absent. | Corrupt or old recordings may be accepted, and hashes may include padding or unstable iteration order. | Define a versioned replay envelope with magic, version, engine compatibility ID, initial seed/state ID, tick-indexed ordered commands, and checked lengths. Hash explicitly serialized authoritative fields in stable entity/component order. |
| A16 | §§5, 15, 16; **high** | A seeded PRNG is required but algorithm, state ownership, and stream partitioning are unspecified. | `std::random` implementations or call-order coupling can alter gameplay. | Use a specified engine-owned PRNG such as PCG32 with named simulation streams; never use standard-library distributions for authoritative results without defining their algorithm. |
| A17 | §7; **high** | Contiguous/chunk storage leaves sparse-set versus archetype ECS unresolved. | Later selection would rewrite component APIs and structural-change behaviour. | Use sparse-set component pools initially: dense component/entity arrays, generation-checked entity registry, and smallest-pool multi-component iteration. |
| A18 | §7; **high** | Entity index width, generation width, null value, generation wrap, reuse, and stale-reference rules are missing. | Long sessions can resurrect stale handles or make serialized IDs unsafe. | Use an opaque 64-bit runtime handle split into 32-bit index and generation, reserve an invalid value, increment on destruction, retire a slot on generation exhaustion, and never persist raw runtime handles. |
| A19 | §7; **high** | Deferred structural changes have no ordering or conflict semantics. | Destroy/add/remove commands in the same tick can become order-dependent or unsafe. | Record commands in deterministic submission order, apply them at explicit phase barriers, make destruction dominate later operations for that entity, and report invalid/conflicting operations in debug builds. |
| A20 | §7; **medium** | Pointer/reference stability during pool growth, swap erase, and structural changes is not stated. | Systems may retain invalid component addresses. | Component references are valid only until the next structural mutation of that pool; systems retain entity handles, not component pointers, across phase boundaries. |
| A21 | §§1, 5, 21, 23; **high** | “Hot loop” and “routine allocation” are not defined or instrumented. | Allocation claims cannot be tested, and custom allocators may be built speculatively. | Use standard allocation for setup/ownership, `std::pmr`-compatible arenas/pools for bounded frame/tick scratch and hot collections, allocator counters, warm-up phases, and zero-allocation assertions in established benchmark loops. |
| A22 | §§5, 8; **critical** | Vulkan ownership is described generally but no destruction graph is fixed. | Device children can outlive the device, or resources can be freed while referenced by submitted work. | Renderer owns instance→surface→device/queues→resource managers→swapchain/resources; RAII wrappers are non-copyable; deferred destruction is tied to completed frame fences; renderer handles never expose raw Vulkan ownership to gameplay. |
| A23 | §§8, 29.1; **high** | Device feature, queue, extension, and surface capability requirements are unspecified. | Physical-device selection may succeed on an unusable device or differ unpredictably. | Require Vulkan 1.3, graphics and presentation support, swapchain extension, dynamic rendering and synchronization2; score optional features separately and produce a rejection diagnostic per device. |
| A24 | §§8, 29.1; **high** | Swapchain recreation does not define image index versus frame index, format changes, old-swapchain handoff, or exactly which dependent resources are rebuilt. | Repeated resize can use mismatched framebuffers, depth resources, or synchronization objects. | Keep frames-in-flight independent from swapchain image count, rebuild all extent/format-dependent resources, pass `oldSwapchain`, and initially call `vkDeviceWaitIdle` before destroying the old dependency set. |
| A25 | §8; **high** | GPU memory allocation policy is absent, and Vulkan Memory Allocator is not approved by the specification. | Either excessive dedicated allocations or an unplanned third-party dependency may appear. | Use a narrow internal allocator interface. M4 may use measured dedicated allocations for its small resource set; add internal block/suballocation before asset scale requires it. VMA requires a separate explicit approval. |
| A26 | §§8, 29.1; **medium** | Device loss and surface loss are not assigned recovery semantics. | Error handling may retry forever or continue with invalid state. | Treat device loss as a controlled fatal renderer shutdown with diagnostics initially; recreate the surface only where GLFW supports it safely, otherwise exit cleanly. |
| A27 | §§8, 9, 24; **high** | M4 requires shader loading/pipelines before the M5 shader asset pipeline exists. | M4 could create an ad-hoc asset system that M5 must replace. | M0 establishes deterministic GLSL→SPIR-V build helpers. M4 consumes build-generated SPIR-V through a minimal file/resource interface; M5 generalizes it into compiled asset management. |
| A28 | §§9, 24; **high** | Stable asset ID representation, path canonicalization, rename behaviour, and collision detection are unspecified. | References can break across platforms or collide silently. | Use persistent 128-bit asset IDs stored in authoring metadata, with canonical case-sensitive UTF-8 virtual paths as lookup aliases. Reject absolute paths, `..`, separator ambiguity, duplicate IDs, and case-fold collisions. |
| A29 | §§5, 9, 18, 19; **high** | Binary formats lack common endian, integer-width, size-validation, and unknown-field policy. | Files can overflow size calculations or be interpreted differently across hosts. | Use fixed-width little-endian fields, explicit header size/version, checked arithmetic, bounded lengths/counts, payload-size validation, and format-specific checksum policy. Never serialize native structs directly. |
| A30 | §9; **medium** | Atomic compilation and “incremental when practical” lack cache-key and output-commit semantics. | Interrupted builds can leave partially valid assets or stale dependencies. | Compile to a temporary file, validate, then atomically replace; key incremental work by compiler version, format version, source hash, options, and dependency hashes. |
| A31 | §§9, 24; **high** | Required initial asset classes include maps and UI layouts, but M5’s deliverable/gate mentions only shaders, textures, and meshes. | Map/UI format work may be silently omitted from M5 or duplicated later. | M5 defines common ID/format infrastructure and minimal compiled map/UI schema contracts; M6 and M13 implement their runtime semantics. |
| A32 | §§18, 24; **high** | `.amap` loading/export is assigned to M14, but M6 requires a map to load into a world and the vertical slice needs a loaded dungeon earlier. | M6 has no stable persisted map contract. | Move `.amap` v1 documentation and safe loader to M5/M6. M14 retains deterministic generation, connectivity validation, and export. |
| A33 | §§10, 29.3; **high** | Coordinate system, tile origin, world-to-grid rounding, cell size, and overflow limits are unspecified. | Negative coordinates and edge cells may map differently between navigation, collision, and world code. | Lock a right-handed world with Y-up; map X/Z to signed grid coordinates using floor after finite/range validation. Map metadata owns origin and cell size. |
| A34 | §§10, 29.3; **high** | Spatial structure selection is deferred to measurement, but M6 must deliver a foundation before representative workloads exist. | Implementation could be blocked or overgeneralized. | Use bounded arrays for static map/navigation data and a signed-coordinate spatial hash grid for dynamic entities. Expose a query interface so cell size can be profiled without changing consumers. |
| A35 | §29.3; **high** | The gameplay policy for movement, knockback, spawn, and teleport outside the playable world is not selected. | Low-level grids may silently clamp authoritative state. | Reject invalid spawns/teleports; resolve ordinary movement and knockback against world boundaries through gameplay collision; never clamp inside the indexing layer. |
| A36 | §12; **high** | Diagonal corner-cutting, integer costs, tie-break order, path endpoint semantics, and request-budget units are absent. | A* can be nondeterministic or traverse blocked corners. | Use integer costs, fixed neighbour order, deterministic `(f,h,insertion-sequence)` tie-breaks, forbid diagonal corner cutting by default, and budget requests by expanded-node count rather than wall time. |
| A37 | §13; **medium** | Shape inclusivity, epsilon policy, degenerate inputs, and 2D capsule/circle choice are unspecified. | Collision tests can disagree at boundaries. | Define closed-overlap semantics, explicit finite-input validation, centralized tolerances for presentation math, and circle/capsule ground-plane primitives selected by actor shape. |
| A38 | §14 / M10; **medium** | “Projectile/AOE as selected” leaves the first ability behaviour unresolved. | M10 scope and renderer/collision dependencies are uncertain. | Select a deterministic projectile with a small impact AOE; it exercises commands, movement, spatial queries, damage events, and visible response without broad ability machinery. |
| A39 | §17; **medium** | UI anchor semantics and coordinate/DPI policy are required but not provided. | Runtime and designer exports may interpret layouts differently. | Before M13, document normalized anchor point, pixel/logical-unit offsets, pivot, size, parent-relative layout, DPI scaling, visibility, and invalid-reference behaviour in one versioned schema. |
| A40 | §19; **medium** | “Migration where practical” does not distinguish required rejection from optional migration. | Old saves might be partially loaded. | Require exact-version validation and safe rejection initially; migrations are explicit version-to-version functions added only when supported and tested. |
| A41 | §§21, 24; **medium** | M1 precedes logging/assertion infrastructure in M2. | Platform and timing failures may acquire temporary diagnostics that are later replaced. | M0 supplies only a minimal fatal/configuration diagnostic and standard assertions; M2 introduces the structured runtime diagnostic API. |
| A42 | M8/M9; **medium** | Navigation movement precedes the collision milestone. | “Reliably navigates obstacle maps” may imply collision already exists. | M8 movement follows validated navigation cells and static walkability only; M9 adds continuous collision and dynamic candidate queries. State that distinction in both gates. |
| A43 | §§22, 23; **high** | Benchmark warm-up, build mode, sample count, reference hardware, percentile, scene seed, and allocation instrumentation are undefined. | Performance gates will be noisy or incomparable. | Store fixed benchmark seeds/scenes, run optimized builds after warm-up, report median/p95 plus environment metadata, and designate reference hardware before enforcing M11/M18 thresholds. |
| A44 | §§24, 25; **high** | Many gates use unmeasurable terms such as “stable,” “reliably,” “playable,” and “where practical.” | Milestones can be claimed complete without reproducible evidence. | Add a milestone acceptance matrix before each milestone with exact commands, test cases, failure conditions, artifact versions, and benchmark procedure. |
| A45 | §§22, 23; **high** | Performance tests under normal CTest execution can be hardware-sensitive and flaky. | CI may fail unpredictably or regressions may go unnoticed. | Separate correctness tests from labeled benchmark executables. CI records benchmark results; only reference-hardware gates enforce absolute budgets. |
| A46 | §§2, 25; **high** | Linux is primary and Windows secondary, but “supported configuration” does not state whether both must pass each milestone. | Linux-only assumptions can accumulate until late in development. | Require Linux GCC and Clang on every milestone; require Windows MSVC for M0 and every milestone that changes portable public/core code. Platform-specific smoke tests run on their relevant host. |
| A47 | §27; **medium** | The vertical slice contents are specified but not assigned one completion point. | Individual milestone gates can pass while the integrated slice remains broken. | Add a vertical-slice integration gate after M13; M14/M15 tooling must not delay it. Use a curated `.amap` until procedural generation arrives. |

# B. Locked Architecture Decisions

Approval of this audit locks the following choices for M0 and establishes defaults for later milestone plans.

1. **Authority and version:** Markdown is canonical v1.1; PDF is derivative and cannot override it.
2. **Project scope:** bespoke single-player ARPG engine; no general-purpose engine and no networking in the current roadmap.
3. **Authority boundary:** gameplay simulation owns state; rendering, UI, audio, VFX, and tools consume state/events without determining outcomes.
4. **Future networking posture:** tick-indexed commands, serializable authoritative state, and simulation-owned RNG remain suitable for a future authoritative server; no lockstep constraints.
5. **Determinism:** same engine version, build configuration, target platform, and CPU architecture must reproduce state from identical initial state, seeds, and command stream. Replays are versioned and incompatible versions fail safely.
6. **Simulation time:** integer tick index is authoritative; fixed rate is 60 Hz; wall-clock time is restricted to scheduling and render interpolation.
7. **Threading:** platform polling, input translation, simulation, and rendering begin on one thread. No locks/atomics are added until ownership actually crosses threads.
8. **Paused/minimized policy:** the single-player client pauses wall-clock simulation while minimized and resets its accumulator on restoration; headless simulation is unaffected.
9. **Input:** ordered platform events feed persistent device state plus one-shot transitions; named gameplay commands are assigned to ticks and consumed exactly once.
10. **PRNG/hash:** engine-specified PCG32 streams; canonical field serialization for state hashes; no hashing of object memory, pointers, padding, or unordered iteration.
11. **ECS:** 64-bit generational handles, sparse-set component pools, deferred ordered structural changes, and no retained component pointer across structural mutation.
12. **Memory:** normal RAII allocation for setup and long-lived ownership; bounded arenas/pools/PMR resources for measured hot paths; allocation counters make zero-allocation claims testable.
13. **C++ policy:** C++20, no compiler extensions, 64-bit only. Exceptions and RTTI remain enabled initially, but runtime APIs use explicit status/result contracts and exceptions do not cross fixed-tick subsystem boundaries.
14. **Build linkage:** static project libraries initially, target-local warnings/options, no global include directories, and no wildcard source discovery.
15. **Target topology:** `arpg_core_engine` → `arpg_gameplay` → applications; tools depend on core/shared formats, not gameplay unless a later tool has an explicit gameplay-authoring need. Tests link only the layers under test.
16. **Application layout:** add `apps/arpg-client` and later `apps/arpg-headless`, rather than placing entry points in engine/gameplay.
17. **Test framework:** Catch2 v3 with CTest labels for unit, integration, determinism, and performance.
18. **Dependency mechanism:** CMake `FetchContent` using immutable release archives plus SHA-256. No vcpkg, Conan, or CPM in M0. System packages provide compilers, Vulkan runtime/development tooling, and shader tools.
19. **Approved dependency baseline:** GLFW 3.4, GLM 1.0.3, Dear ImGui 1.92.8, Catch2 3.15.0, volk/Vulkan-Headers 1.4.350 while the engine requests and validates Vulkan 1.3 only. These are pinned release baselines, not permission to adopt Vulkan 1.4 features. Upstream release references: [GLFW](https://github.com/glfw/glfw/releases), [GLM](https://github.com/g-truc/glm), [Dear ImGui](https://github.com/ocornut/imgui/releases), [Catch2](https://github.com/catchorg/Catch2/releases), and [volk](https://github.com/zeux/volk/releases).
20. **Dependency allowlist:** no VMA, JSON library, logging library, physics engine, mesh importer, or image loader is approved by this plan. Later adoption requires an explicit architecture decision and licence review.
21. **Vulkan:** Vulkan 1.3 with dynamic rendering and synchronization2; two frames in flight initially; single render thread; fence-governed deferred destruction; `vkDeviceWaitIdle` is acceptable for initial swapchain recreation.
22. **GPU allocation:** internal allocator interface; dedicated allocations are acceptable for M4’s small resource set, followed by internal block/suballocation when asset scale requires it.
23. **Assets:** persistent 128-bit asset IDs plus validated canonical virtual paths; little-endian, versioned compiled formats with checked sizes and explicit serialization.
24. **World coordinates:** right-handed, Y-up world; gameplay ground plane is X/Z. Signed coordinates are validated before any unsigned index conversion.
25. **Spatial data:** bounded static map/navigation grids plus a signed-coordinate dynamic spatial hash.
26. **World boundaries:** invalid spawns/teleports fail; movement/knockback is resolved by gameplay collision; indexing code never silently clamps authoritative positions.
27. **Navigation:** deterministic integer-cost A*, fixed neighbour order, no diagonal corner cutting by default, and node-expansion request budgets.
28. **Map sequencing:** `.amap` v1 documentation/loader moves to M5/M6; procedural generation and export remain M14.
29. **First ability:** deterministic projectile with an impact AOE for M10.
30. **Performance:** optimized-build benchmarks with fixed seeds, warm-up, median/p95 reporting, allocation statistics, and recorded hardware/software metadata.
31. **Portability:** Linux GCC/Clang and Windows MSVC are first-class build configurations; filesystem and persisted formats never rely on native separator, struct layout, `long`, or host endianness.
32. **Project licence default:** until the owner selects a licence, README states that no licence grant is provided. All third-party licences are still recorded.

# C. Milestone M0 Execution Plan

## Task 1 — Correct and preserve the specification

- **Objective:** Establish v1.1 source-of-truth documentation before source history begins.
- **Files:** modify `docs/Master_Implementation_Specification.md`; correct/regenerate `docs/ARPG_Engine_Master_Implementation_Specification_v1.1.pdf`; create `docs/Architecture_Decisions.md`.
- **Dependencies:** confirmed authority/version decisions.
- **Validation:**
  ```bash
  rg -n 'Version 1\.0|Version 1\.1' docs
  pdftotext docs/ARPG_Engine_Master_Implementation_Specification_v1.1.pdf - | rg 'Version 1\.[01]'
  ```
- **Acceptance:** canonical Markdown and visible PDF body say v1.1; architecture decisions record the confirmed determinism/networking policies and decisions above.
- **Risks:** the original PDF-generation source is absent; regeneration must preserve the PDF as a secondary readable artifact without treating it as canonical.

## Task 2 — Initialise safe source control

- **Objective:** Create Git metadata while ensuring generated content is ignored before the first source commit.
- **Files:** create `.gitignore`, `.gitattributes`; preserve both specification files.
- **Dependencies:** Task 1.
- **Commands:**
  ```bash
  git init --initial-branch=main
  git status --short
  git check-ignore -v build/probe.o
  git diff --check
  ```
- **Acceptance:** repository is on `main`; specifications are tracked candidates; `/build`, IDE state, compiler output, caches, and generated assets are ignored; source/docs remain visible to Git.
- **Risks:** broad ignore patterns could hide source assets; use root-anchored patterns.

## Task 3 — Document and provision the host toolchain

- **Objective:** Detect the host, document exact prerequisites, install only with permitted elevation, and verify versions.
- **Files:** create `README.md`; create `cmake/ValidateToolchain.cmake`.
- **Dependencies:** APT package indexes available; elevated permission for installation.
- **Host command to present before elevation:**
  ```bash
  sudo apt-get update
  sudo apt-get install --no-install-recommends \
    build-essential cmake ninja-build git pkg-config \
    libvulkan-dev vulkan-tools vulkan-validationlayers \
    glslc glslang-tools spirv-tools \
    libglfw3-dev libglm-dev \
    clang clang-format clang-tidy
  ```
- **Verification:**
  ```bash
  cmake --version
  ninja --version
  g++ --version
  clang++ --version
  clang-format --version
  clang-tidy --version
  pkg-config --modversion vulkan glfw3 glm
  glslc --version
  glslangValidator --version
  spirv-val --version
  vulkaninfo --summary
  ```
- **Acceptance:** README records Zorin/Ubuntu detection and commands; required executable/header checks produce clear success or a named missing package. Current candidates include Vulkan 1.3.275, GLFW 3.3.10, GLM 0.9.9.8, GLSLC 2023.8, and LLVM 18.
- **Risks:** `sudo` requires human authorization; GPU/driver availability is separate from header/tool installation. M0 must not misreport a headless `vulkaninfo` display limitation as a compiler failure.

## Task 4 — Establish CMake policies and presets

- **Objective:** Produce reproducible, target-scoped C++20 builds.
- **Files:** create root `CMakeLists.txt`, `CMakePresets.json`, `cmake/ProjectOptions.cmake`, `cmake/CompilerWarnings.cmake`, `cmake/ValidateToolchain.cmake`.
- **Dependencies:** Task 3.
- **Commands:**
  ```bash
  cmake --preset linux-gcc-debug
  cmake --preset linux-clang-debug
  ```
- **Acceptance:** CMake fails early on CMake <3.28, non-64-bit targets, unsupported compilers, missing Vulkan/shader tools where enabled, or an in-source build; compiler extensions are off; build output is under `build/<preset>`.
- **Risks:** warning flags differ between GCC, Clang, and MSVC; apply only compiler-supported project flags.

## Task 5 — Pin approved dependencies

- **Objective:** Make dependency resolution reproducible and auditable.
- **Files:** create `cmake/Dependencies.cmake`, `THIRD_PARTY_NOTICES.md`; update README.
- **Dependencies:** network access for first fetch or a pre-populated CMake download cache.
- **Validation:**
  ```bash
  cmake --preset linux-gcc-debug --fresh
  cmake --build --preset linux-gcc-debug --target arpg_dependency_smoke --parallel
  cmake --preset linux-gcc-debug --fresh -DFETCHCONTENT_FULLY_DISCONNECTED=ON
  ```
- **Acceptance:** approved archives are pinned by version and SHA-256; no moving branch/tag is trusted without an archive hash; dependency warnings do not inherit project warnings-as-errors; disconnected configure works after the cache is populated.
- **Risks:** distro Vulkan headers may differ from pinned headers; project targets must consistently use pinned Vulkan headers while validating the runtime/API floor as Vulkan 1.3.

## Task 6 — Create the target and directory skeleton

- **Objective:** Encode dependency boundaries without claiming later functionality.
- **Files:** create `core-engine/CMakeLists.txt`, `arpg-gameplay/CMakeLists.txt`, `dev-tools/CMakeLists.txt`, `apps/CMakeLists.txt`, corresponding include/source directories, and minimal build-information/dependency-smoke sources.
- **Dependencies:** Tasks 4–5.
- **Validation:**
  ```bash
  cmake --build --preset linux-gcc-debug \
    --target arpg_core_engine arpg_gameplay arpg_tools arpg_dependency_smoke \
    --parallel
  ```
- **Acceptance:** all named targets exist; core has no gameplay/tool link; gameplay has no Vulkan types in its public contract; tools do not leak into runtime targets. Any interface-only bootstrap target is documented as build topology, not implemented functionality.
- **Risks:** empty placeholder libraries would violate the completion discipline; only real build metadata/dependency validation code is added, with no fake gameplay or renderer implementation.

## Task 7 — Add tests and CTest integration

- **Objective:** Prove the compiler mode, dependency graph, build metadata, and test runner work.
- **Files:** create `tests/CMakeLists.txt`, `tests/unit/bootstrap_tests.cpp`, `tests/integration/dependency_smoke.cpp`.
- **Dependencies:** Catch2 and Tasks 5–6.
- **Commands:**
  ```bash
  cmake --build --preset linux-gcc-debug --parallel
  ctest --preset linux-gcc-debug --output-on-failure
  ctest --preset linux-gcc-debug --show-only=json-v1
  ```
- **Acceptance:** CTest discovers labeled unit/integration tests; tests verify 64-bit C++20 configuration, v1.1 build metadata, dependency header/link integration, and target-boundary assumptions without opening a window or initializing Vulkan.
- **Risks:** a smoke test that merely returns zero is insufficient; assertions must validate actual bootstrap contracts.

## Task 8 — Establish formatting and static analysis

- **Objective:** Make style and diagnostics repeatable from M0 onward.
- **Files:** create `.clang-format`, `.clang-tidy`; extend root CMake and README with non-mutating check targets.
- **Dependencies:** clang-format/tidy 18.
- **Commands:**
  ```bash
  cmake --build --preset linux-clang-debug --target format-check
  cmake --build --preset linux-clang-debug --target tidy
  ```
- **Acceptance:** all project sources pass format check and selected correctness, bug-prone, performance, portability, and modernization checks; third-party code is excluded.
- **Risks:** enabling broad style-oriented clang-tidy checks can create noise; start with correctness-focused checks and document any narrowly justified exclusions.

## Task 9 — Validate the supported matrix

- **Objective:** Demonstrate reproducible clean configure/build/test on primary and secondary targets.
- **Files:** update README and presets; no subsystem implementation.
- **Linux commands:**
  ```bash
  cmake --preset linux-gcc-debug --fresh
  cmake --build --preset linux-gcc-debug --parallel
  ctest --preset linux-gcc-debug --output-on-failure

  cmake --preset linux-clang-debug --fresh
  cmake --build --preset linux-clang-debug --parallel
  ctest --preset linux-clang-debug --output-on-failure

  cmake --preset linux-gcc-release --fresh
  cmake --build --preset linux-gcc-release --parallel
  ctest --preset linux-gcc-release --output-on-failure
  ```
- **Windows commands from a VS 2022 developer shell/runner:**
  ```powershell
  cmake --preset windows-msvc-debug --fresh
  cmake --build --preset windows-msvc-debug --parallel
  ctest --preset windows-msvc-debug --output-on-failure
  ```
- **Acceptance:** all four configurations pass with no project warnings; commands require no undocumented working-directory or environment assumptions; CTest is suitable for CI invocation.
- **Risks:** Windows cannot be certified from the Linux host alone; evidence from a Windows runner is required before M0 is declared complete.

## Task 10 — Complete the M0 gate and initial commit

- **Objective:** Record a clean, truthful bootstrap checkpoint.
- **Files:** all M0 files; update README status and `docs/Architecture_Decisions.md`.
- **Commands:**
  ```bash
  git status --short
  git diff --check
  git diff --cached --stat
  git commit -m "Bootstrap repository and CMake build"
  ```
- **Acceptance:** the initial commit includes both specifications, ignores build output, records exact dependency pins, and contains successful Linux and Windows configure/build/test evidence. README claims M0 complete only after every gate passes.
- **Risks:** commit creation depends on configured Git identity and repository workflow permission; if unavailable, leave a clean staged diff and report the blocker without claiming completion.

# D. Proposed Repository Files

## `AGENTS.md`

- Establish Markdown specification precedence and v1.1.
- Encode one-milestone-at-a-time discipline and required inspect/plan/format/build/test/smoke/verify/report cycle.
- Record target dependency rules, authoritative simulation boundary, dependency allowlist, no-placeholder rule, persisted-format rules, and safe file-editing expectations.
- List canonical configure/build/test/format/tidy commands.
- Require explicit acceptance evidence before changing milestone status.

## `README.md`

- State project mission, current truthful milestone status, supported platforms, and no current project licence grant.
- List Zorin/Ubuntu and Windows prerequisites, exact install/verification commands, and elevated-permission behaviour.
- Document presets, clean build/test commands, dependency cache/offline mode, directory layout, and troubleshooting for missing Vulkan/shader tooling.
- Explain that the engine is not yet implemented at M0.

## Root `CMakeLists.txt`

- Require CMake 3.28 and C++20 with extensions disabled.
- Reject in-source and non-64-bit builds.
- Define project options and approved dependency resolution.
- Add engine, gameplay, applications, tools, and tests in dependency order.
- Enable CTest and expose aggregate check targets without global compiler/include pollution.

## `CMakePresets.json`

- Configure/build/test presets for Linux GCC Debug/Release, Linux Clang Debug, and Windows MSVC Debug.
- Use `build/<preset-name>` binary directories.
- Default tests/tools on, warning-as-error on for validation presets, and explicit compile-command export on Ninja presets.
- Avoid machine-local paths; developers put overrides in ignored `CMakeUserPresets.json`.

## `cmake/`

- `ProjectOptions.cmake`: language, architecture, build options, assertions, sanitizer switches.
- `CompilerWarnings.cmake`: compiler-specific project warnings.
- `Dependencies.cmake`: pinned approved dependencies and disconnected mode.
- `ValidateToolchain.cmake`: early, actionable dependency/compiler diagnostics.
- `ValidateTargetGraph.cmake`: enforce forbidden target dependencies.
- Later shader helpers belong here once M4 shader compilation is introduced.

## `tests/`

- `unit`, `integration`, `determinism`, and `performance` match the specification.
- M0 contains only meaningful bootstrap/dependency tests.
- Tests are labeled in CTest; performance tests are not part of ordinary correctness pass/fail timing.
- Shared test helpers remain test-only and never become runtime dependencies.

## `.clang-format`

- LLVM-derived C++20 style, four-space indentation, 120-column limit, attached braces, deterministic include sorting, no tabs.
- Apply only to project C/C++ sources, not fetched dependencies or generated SPIR-V.

## `.clang-tidy`

- Enable compiler diagnostics plus focused `bugprone`, `performance`, `portability`, selected `modernize`, and safe `readability` checks.
- Treat project diagnostics as errors in the tidy target.
- Exclude third-party/generated paths and document every project-source suppression.

## `.gitignore`

- Ignore `/build/`, CMake/Ninja output, compile databases copied to root, object/binary/debug files, IDE state, OS metadata, logs, profiler output, caches, `CMakeUserPresets.json`, and temporary asset-compiler output.
- Do not broadly ignore `assets-source`.
- Ignore generated `assets-compiled` payloads initially while retaining a tracked README/manifest placeholder.
- Add `.gitattributes` for LF-normalized text, binary PDF/assets, and explicit Windows script handling.

# E. Blockers and Questions

No unresolved human architectural decision blocks M0 planning.

Operational blockers expected during execution:

- Package installation requires elevated permission; present the exact APT commands and wait for that permission.
- Windows validation requires a Windows runner with MSVC/Vulkan prerequisites.
- Correct PDF regeneration may require access to its original generation process; the Markdown remains authoritative and M0 cannot claim documentation alignment until the visible PDF label is corrected.
- A public/open-source project licence is not selected. The safe default is no licence grant; this does not block private M0 work.
- VMA and other non-approved utility dependencies remain unavailable unless explicitly approved in a later architecture decision.

# F. Final Readiness Verdict

**READY FOR M0 WITH DOCUMENTED ASSUMPTIONS**

M0 may begin only after approval of this audit. Its first actions are documentation correction, `.gitignore` creation, Git initialization, and permission-aware prerequisite provisioning. No later milestone functionality is assumed or claimed.
