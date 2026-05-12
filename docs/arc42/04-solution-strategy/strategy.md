---
status: Draft
owner: tensor
last-reviewed: 2026-05-12
---

# `tensor` — Solution Strategy (arc42 §4)

| Metadata      | Value                                                          |
| ------------- | -------------------------------------------------------------- |
| Status        | Draft                                                          |
| Type          | arc42 §4 (Solution Strategy)                                   |
| Owner         | uyuutosa                                                       |
| Last Updated  | 2026-05-12                                                     |

> Per the arc42 convention this file is a **navigation index**: the seven highest-leverage decisions, each linking to its full ADR. Reasoning lives in the ADRs, not here.

## The seven decisions that shape everything else

In rough dependency order:

1. **[ADR-0001](../09-decisions/0001-pivot-to-educational-named-axis-dsl.md) (refined by [ADR-0010](../09-decisions/0010-refine-positioning-to-educational-first-production-capable.md); aspirational canonical-reference framing via [ADR-0015](../09-decisions/0015-aspire-to-canonical-reference-quality-not-self-anoint.md) superseding [ADR-0013](../09-decisions/0013-reframe-as-canonical-reference-for-named-tensor-computation.md)) — Educational-first, production-capable via backend adapters.**
   The Domain is small, readable, and unencumbered by ABI commitments; speed for production-shaped workloads comes from swappable `KernelBackend` adapters (reference, Eigen, WebGPU). Production adoption is *permitted* but adopted *as-is*. The project applies three forcing-function disciplines (bibliography / ubiquitous language / reproducibility per ADR-0015 §Decision Outcome) without claiming the canonical-reference label for itself. Every other decision below inherits this north star.

2. **[ADR-0002](../09-decisions/0002-rewrite-on-cpp20-baseline-with-mdspan-interop.md) — Rewrite on a C++20 baseline with `std::mdspan` interop via the Kokkos reference impl.**
   C++20 unlocks concepts, NTTPs, `consteval`, and `mdspan` interop without requiring bleeding-edge toolchains. The 2016 codebase is retired.

3. **[ADR-0009](../09-decisions/0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md) — Adopt DDD ubiquitous language + Hexagonal (Ports & Adapters) "lite".**
   Each container is classified `Domain` / `DrivingAdapter` / `DrivenAdapter`; ports are C++20 concepts in `concepts.hpp`; the dependency rule (Domain depends on no adapter) is enforced by CI. The architecture itself becomes a teaching artifact.

4. **[ADR-0004](../09-decisions/0004-adopt-hybrid-named-axis-api.md) — Adopt a hybrid named-axis API: runtime axis identity with an NTTP compile-time fast path via the `_ax` user-defined literal.**
   The headline feature. Compile-time mismatched-axis errors are pedagogically powerful; runtime axis identity covers dynamic shapes and notebooks. Both paths share one kernel; the user opts in.

5. **[ADR-0005](../09-decisions/0005-adopt-tex-lyx-as-authoring-surface.md) — Adopt TeX / LyX as a first-class authoring surface (`consteval` LaTeX-subset parser via the `_tex` UDL, with a LyX export module as a second phase).**
   Realises the project's slogan: *the formula is the program*. `tensor::tex` becomes the canonical DrivingAdapter implementing the `ExpressionSource` port.

6. **[ADR-0006](../09-decisions/0006-adopt-webgpu-as-gpu-backend.md) (refined by [ADR-0012](../09-decisions/0012-webgpu-adapter-implementation-design.md) for the adapter design and [ADR-0014 §1](../09-decisions/0014-external-substrate-strategy.md) + [ADR-0016](../09-decisions/0016-substrate-refinement-drop-gpu-cpp-talk-to-dawn-directly.md) for the substrate) — Adopt WebGPU (Dawn via vcpkg) as the GPU backend.**
   Zero proprietary install for the learner; runs on every desktop GPU and (eventually) in the browser. `tensor::core::backend::webgpu` is the DrivenAdapter implementing the `KernelBackend` port; it talks to Dawn directly via Dawn's own `webgpu_cpp.h` (Google-maintained, always synchronised). CUDA-direct is rejected. As of 2026-05-12 the adapter dispatches real GPU compute on 12 of the 15 `KernelBackend` methods for `float` on RTX 3090.

7. **[ADR-0007](../09-decisions/0007-adopt-autograd-as-first-class-subsystem.md) — Adopt tape-based reverse-mode autograd as a first-class subsystem.**
   Pivots the library from a pure tensor-algebra DSL to a tinygrad-class teaching ML framework grounded in named-axis semantics. `tensor::autograd` extends the Domain hexagon.

(Two implementation-flavored decisions — [ADR-0003](../09-decisions/0003-replace-eclipse-cdt-with-cmake-and-vcpkg.md) (CMake + vcpkg) and [ADR-0008](../09-decisions/0008-distribute-as-header-only-with-jupyter-tutorials.md) (header-only + Jupyter Book) — live one click away rather than in this top-7 index.)

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
