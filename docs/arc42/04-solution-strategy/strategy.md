---
status: Draft
owner: tensor
last-reviewed: 2026-05-10
---

# `tensor` — Solution Strategy (arc42 §4)

| Metadata      | Value                                                          |
| ------------- | -------------------------------------------------------------- |
| Status        | Draft                                                          |
| Type          | arc42 §4 (Solution Strategy)                                   |
| Owner         | uyuutosa                                                       |
| Last Updated  | 2026-05-10                                                     |

> Per the arc42 convention this file is a **navigation index**: the seven highest-leverage decisions, each linking to its full ADR. Reasoning lives in the ADRs, not here.

## The seven decisions that shape everything else

In rough dependency order:

1. **[ADR-0001](../09-decisions/0001-pivot-to-educational-named-axis-dsl.md) — Pivot to educational / DSL-research positioning, anchored on named-axis tensor algebra.**
   The library exists where Eigen / xtensor / libtorch / Kokkos / `std::linalg` do not — in the underserved educational niche around named-axis algebra. Every other decision below inherits this north star.

2. **[ADR-0002](../09-decisions/0002-rewrite-on-cpp20-baseline-with-mdspan-interop.md) — Rewrite on a C++20 baseline with `std::mdspan` interop via the Kokkos reference impl.**
   C++20 unlocks concepts, NTTPs, `consteval`, and `mdspan` interop without requiring bleeding-edge toolchains. The 2016 codebase is retired.

3. **[ADR-0003](../09-decisions/0003-replace-eclipse-cdt-with-cmake-and-vcpkg.md) — Replace Eclipse CDT with CMake (≥ 3.25) and vcpkg.**
   Genre-default toolchain. Removes the contributor-onboarding wall that Eclipse CDT imposes. CI moves to GitHub Actions.

4. **[ADR-0004](../09-decisions/0004-adopt-hybrid-named-axis-api.md) — Adopt a hybrid named-axis API: runtime axis identity with an NTTP compile-time fast path via the `_ax` user-defined literal.**
   The headline feature. Compile-time mismatched-axis errors are pedagogically powerful; runtime axis identity covers dynamic shapes and notebooks. Both paths share one kernel; the user opts in.

5. **[ADR-0005](../09-decisions/0005-adopt-tex-lyx-as-authoring-surface.md) — Adopt TeX / LyX as a first-class authoring surface (`consteval` LaTeX-subset parser via the `_tex` UDL, with a LyX export module as a second phase).**
   Realises the project's slogan: *the formula is the program*. The same expression graph is reachable from C++ tensor expressions, from a `_tex` literal, and (Phase 3+) from a LyX document.

6. **[ADR-0006](../09-decisions/0006-adopt-webgpu-as-gpu-backend.md) — Adopt WebGPU (gpu.cpp + Dawn / wgpu-native) as the GPU backend.**
   Zero proprietary install for the learner; runs on every desktop GPU and (eventually) in the browser. CUDA-direct is rejected. Kokkos remains reachable as a future complementary backend.

7. **[ADR-0007](../09-decisions/0007-adopt-autograd-as-first-class-subsystem.md) — Adopt tape-based reverse-mode autograd as a first-class subsystem.**
   Pivots the library from a pure tensor-algebra DSL to a tinygrad-class teaching ML framework grounded in named-axis semantics. Forward-mode and source-to-source remain reachable as future complementary backends.

(A common eighth decision — [ADR-0008](../09-decisions/0008-distribute-as-header-only-with-jupyter-tutorials.md), header-only library + xeus-cling Jupyter tutorials + Jupyter Book site — is implementation-flavored and lives one click away rather than in this top-7 index.)

## How these decisions hang together

- ADR-0001 sets *who* this is for and *what* differentiates it.
- ADR-0002, ADR-0003 set the *substrate*: modern C++ + a contributor-friendly build.
- ADR-0004, ADR-0005 set the *headline experience*: named axes you can see, write in TeX, and have the compiler check.
- ADR-0006, ADR-0007 set the *educational arc*: from algebra to GPU acceleration to small-NN training.

The §1 goals (G-1..G-7) are realised by these seven decisions; the §5 building-block decomposition is a direct projection of them onto code. See [`../01-introduction-and-goals/overview.md`](../01-introduction-and-goals/overview.md) and [`../05-building-blocks/overview.md`](../05-building-blocks/overview.md).

## Cross-references

- §1 Introduction and Goals: [`../01-introduction-and-goals/overview.md`](../01-introduction-and-goals/overview.md)
- §3 Context and Scope: [`../03-context-and-scope/system-context.md`](../03-context-and-scope/system-context.md)
- §5 Building Blocks: [`../05-building-blocks/overview.md`](../05-building-blocks/overview.md)
- §9 Decisions (full ADRs): [`../09-decisions/`](../09-decisions/)
