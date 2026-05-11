---
status: Draft
owner: tensor
last-reviewed: 2026-05-11
---

# `tensor` — Building Block View (arc42 §5)

| Metadata      | Value                                                          |
| ------------- | -------------------------------------------------------------- |
| Status        | Draft                                                          |
| Type          | arc42 §5 (Building Block View)                                 |
| Owner         | uyuutosa                                                       |
| Last Updated  | 2026-05-11                                                     |

> Per `.claude/rules/documentation.md` and arc42 convention, this file is a **navigation index** for the static decomposition. The C4 L2 container diagram in [`../../diagrams/c4/workspace.dsl`](../../diagrams/c4/workspace.dsl) is the structural single source of truth; this prose mirrors it. Implementation HOW lives under [`../../detailed-design/`](../../detailed-design/).

## L1 view (system context)

The system as a whole — `tensor` — is one black box from the learner's perspective. See [`../03-context-and-scope/system-context.md`](../03-context-and-scope/system-context.md).

## L2 view (containers inside `tensor`)

Names match the DSL exactly; do not introduce synonyms.

The library is laid out as a Hexagonal (Ports & Adapters) "lite" architecture per [ADR-0009](../09-decisions/0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md). Each container below carries a **hexagonal classification** — `Domain`, `DrivingAdapter`, or `DrivenAdapter`. The dependency rule is one-way: `Domain` depends on no adapter; adapters depend on `Domain`; **no adapter depends on another adapter directly** ([`../../design-guide/architectural-discipline.md`](../../design-guide/architectural-discipline.md) explains and how it is enforced).

**Shipped / in-flight as of 2026-05-11** (after PR #1–#38): `tensor::core` ✅ (incl. `LabelTag` + `TypedTensor` compile-time path, mdspan interop); `tensor::tex` ✅ (parser + `to_latex` + `_tex` UDL + Evaluator end-to-end); `tensor::autograd` ✅ (Phase 2 full: MVP, activations, broadcast backward, contraction, `zero_grad`, `sgd_update`); **three backend adapters** ✅ (`reference`, `eigen`, **`webgpu` stub** delegating to reference per ADR-0012 / P3.M2) + WebGPU adapter design fixed in [ADR-0012](../09-decisions/0012-webgpu-adapter-implementation-design.md); tutorials 00 / 05 / 07 / 08 ✅; Jupyter Book scaffold ✅ ([`book/`](../../../book/)); perf bench framework ✅ ([`../../../bench/`](../../../bench/)); LyX export module + LyX plugin ✅ ([`../../../lyx-export/`](../../../lyx-export/)); `CONTRIBUTING.md` + `CODE_OF_CONDUCT.md` + `CHANGELOG.md` ✅; xeus-cling notebook CI workflow ✅. **In flight / planned**: WGSL kernels (Phase 3 P3.M3+ — element-wise, GEMM, broadcast/reduce); `0.1.0` release tag (Phase 4 close).

```
                ┌─────────────────────────────────────┐
   Driving      │                                     │
   adapters ──► │   tensor::core  +  tensor::autograd │ ◄── Driven adapters
   (input)      │       (the Domain hexagon)          │     (output)
                │                                     │
                └─────────────────────────────────────┘
                                  ▲
                                  │ ports = C++20 concepts
                                  │ in concepts.hpp
                                  ▼
   tensor::tex          (DrivingAdapter, ExpressionSource port)
   _ax UDL              (DrivingAdapter, embedded in Domain header)
   Python bindings      (DrivingAdapter, future)
   LyX module           (DrivingAdapter, Phase 3+)
                                                      ▲
   tensor::gpu          (DrivenAdapter, KernelBackend port)
   CPU reference        (DrivenAdapter, KernelBackend port — lives inside core)
   mdspan exporter      (DrivenAdapter, BufferExporter port)
   LaTeX renderer       (DrivenAdapter, ExpressionSink port)
   Autograd tape writer (DrivenAdapter, BackwardSink port)
```

| Container                | Hex. class.       | Phase introduced | Purpose                                                                                              | Realises ADR |
| ------------------------ | ----------------- | ---------------- | ---------------------------------------------------------------------------------------------------- | ------------ |
| `tensor::core`           | **Domain**        | 1 ✅              | Named-axis tensor types (`Tensor<T,N>`, `Shape<N>`, `Axis`, `LabelTag<...>`), expression templates, `mdspan` interop *(deferred to Phase 1.5)*, the four arithmetic ops, indexing. Owns `concepts.hpp` declaring `TensorLike`, `Shape`, `Axis`, `KernelBackend`, `BufferExporter` ports. | [0002](../09-decisions/0002-rewrite-on-cpp20-baseline-with-mdspan-interop.md), [0004](../09-decisions/0004-adopt-hybrid-named-axis-api.md), [0009](../09-decisions/0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md) |
| `tensor::autograd`       | **Domain** (ext.) | 2 🚧 (MVP shipped) | Tape-based reverse-mode autograd over named-axis tensors; `Variable<Tensor>` wrapper, registered backwards, gradient-checking harness. Declares `Differentiable`, `BackwardOp`, `BackwardSink` concepts in its own `concepts.hpp`. | [0007](../09-decisions/0007-adopt-autograd-as-first-class-subsystem.md), [0009](../09-decisions/0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md) |
| `tensor::tex`            | **DrivingAdapter** | 1 ✅ (MVP) → 3 (LyX) | `consteval` LaTeX-subset parser *(parser is runtime in MVP; consteval upgrade is Phase 1.5)* exposed via the `_tex` UDL, runtime parser, `to_latex` renderer; later: LyX export module. Implements the `ExpressionSource` and `ExpressionSink` ports declared in `tensor::core::concepts`. | [0005](../09-decisions/0005-adopt-tex-lyx-as-authoring-surface.md) |
| `tensor::core::backend::webgpu` | **DrivenAdapter** | 3 🚧 (P3.M2 stub shipped) | WebGPU adapter satisfying `KernelBackend`. PR #38 shipped a stub that delegates to `reference::Backend`; P3.M3+ replaces method bodies with WGSL kernels dispatched via gpu.cpp + Dawn. (Container renamed from `tensor::gpu` in PR #38 to match the implemented namespace.) | [0006](../09-decisions/0006-adopt-webgpu-as-gpu-backend.md), [0012](../09-decisions/0012-webgpu-adapter-implementation-design.md) |
| `tutorials/`             | (out of hexagon)  | 1 ✅ (00 shipped) | Jupyter notebooks (xeus-cling C++20 kernel) demonstrating each container; CI-executed end-to-end per release | [0008](../09-decisions/0008-distribute-as-header-only-with-jupyter-tutorials.md) |
| Jupyter Book site        | (out of hexagon)  | 4                | Static site generated from `tutorials/`, deployed to GitHub Pages                                    | [0008](../09-decisions/0008-distribute-as-header-only-with-jupyter-tutorials.md) |
| `lyx-export/` (optional) | **DrivingAdapter** | 3+               | LyX module that exports tensor-bearing `.lyx` documents to `_tex`-compatible source                  | [0005](../09-decisions/0005-adopt-tex-lyx-as-authoring-surface.md) |

**Cross-cutting GoF patterns** named where they apply (per ADR-0009): Composite for expression trees, Visitor for AST traversals (parse / WGSL codegen / LaTeX render / backward), Strategy for `KernelBackend` selection, Command for autograd tape entries, Bridge to keep the named-axis runtime and NTTP fast paths over a single underlying kernel implementation.

### Container-to-container relationships

The DSL is authoritative; this is the prose mirror, expressed in hexagonal direction.

- `tensor::autograd` **extends the Domain hexagon**: it consumes `tensor::core` types and concepts, never the other way around.
- `tensor::tex` (DrivingAdapter) **produces** expression graphs by consuming the `tensor::core` `ExpressionSource` port. Round-trip property: `parse(render(e)) == e` for every expression in the corpus.
- `tensor::gpu` (DrivenAdapter) **implements** the `KernelBackend` port declared by `tensor::core::concepts`; it consumes lowered expression graphs from the Domain. The Domain is unaware of `tensor::gpu` even existing.
- `tutorials/` **imports and demos** `tensor::core` (Phase 1), `tensor::tex` (all phases), `tensor::autograd` (Phase 2+), `tensor::gpu` (Phase 3+). Tutorials are *outside* the hexagon — they are demos, not adapters.
- The Jupyter Book site **renders** `tutorials/` to static HTML — fully out of hexagon.
- `lyx-export/` (DrivingAdapter) **exports** LyX content into the `_tex` DSL of `tensor::tex` — adapter-to-adapter via Domain only, never directly.

**The single hard rule (ADR-0009)**: a header under `include/tensor/core/` that is not `concepts.hpp` may not `#include` anything from `include/tensor/{gpu,tex,autograd}/`. CI enforces this; see [`../../design-guide/architectural-discipline.md`](../../design-guide/architectural-discipline.md).

## Why this decomposition

Each container exists because at least one of the §1 goals or one runtime scenario in [`../06-runtime/`](../06-runtime/) requires it:

| Container          | Justifying goals                                      | Cut if…                                                |
| ------------------ | ----------------------------------------------------- | ------------------------------------------------------ |
| `tensor::core`     | G-1 (named axes), G-2 (modern C++)                    | Never — this *is* the project.                         |
| `tensor::tex`      | G-3 ("formula is the program")                        | Drop only if ADR-0005 is superseded.                   |
| `tensor::autograd` | G-4 (end-to-end teaching arc)                         | Drop only if ADR-0007 is superseded back to no-autograd. |
| `tensor::gpu`      | G-4 (teaching arc), G-5 (zero-friction install)       | Drop only if ADR-0006 is superseded back to none.       |
| `tutorials/`       | G-4 (teaching arc), G-5 (install)                     | Drop only if ADR-0008 is superseded; never realistic.   |
| Jupyter Book site  | G-6 (living book)                                     | Drop only if ADR-0008 is partially superseded.          |
| `lyx-export/`      | G-3 (TeX/LyX surface)                                 | Soft-deletable: a Phase-3 nice-to-have.                 |

Phase 1 ships only `tensor::core`, `tensor::tex` (MVP), and `tutorials/` (intro only). Subsequent containers are introduced phase-by-phase per [`../../impl-plans/2026-05-10_revival-phase-1.md`](../../impl-plans/2026-05-10_revival-phase-1.md) and follow-on plans.

## Naming conventions

- **C++ namespace mirrors container name**: `tensor::core::Tensor`, `tensor::autograd::Variable`, etc.
- **Headers live under `include/tensor/<container>/`** (e.g. `include/tensor/core/tensor.hpp`, `include/tensor/tex/parser.hpp`).
- **Container-internal types are not part of the public ABI / API.** Only types in `include/tensor/<container>/<container>.hpp` are user-facing.
- **No abbreviations in container names.** `gpu`, not `g`; `autograd`, not `ag`. Educational pitch demands names that read.

## What is *not* here

- *How* `Tensor<T,N>` stores data, *how* the `consteval` parser is structured, *how* the autograd tape is allocated → [`../../detailed-design/`](../../detailed-design/).
- *When* a backward pass runs, *when* a WGSL dispatch happens → [`../06-runtime/`](../06-runtime/).
- *Why* WebGPU and not CUDA → [ADR-0006](../09-decisions/0006-adopt-webgpu-as-gpu-backend.md).

## L3 (component) zoom-ins

Component-level views land as detailed-design docs under [`../../detailed-design/`](../../detailed-design/). Current instances:

- [`tensor-core.md`](../../detailed-design/tensor-core.md) — Domain centerpiece.
- [`webgpu-element-wise-kernels.md`](../../detailed-design/webgpu-element-wise-kernels.md) — WebGPU element-wise binary + unary kernels (P3.M3).
- [`webgpu-gemm-kernel.md`](../../detailed-design/webgpu-gemm-kernel.md) — WebGPU tiled GEMM (P3.M4).

Planned siblings (not yet written): `tensor-autograd.md`, `tensor-tex.md`, `kernel-backend-port.md`.

## Cross-references

- §3 Context and Scope: [`../03-context-and-scope/system-context.md`](../03-context-and-scope/system-context.md)
- §4 Solution Strategy: [`../04-solution-strategy/strategy.md`](../04-solution-strategy/strategy.md)
- §6 Runtime: [`../06-runtime/overview.md`](../06-runtime/overview.md) — four scenarios (broadcast / `_tex` / autograd / backend swap)
- §9 Decisions: [`../09-decisions/`](../09-decisions/)
- C4 source: [`../../diagrams/c4/workspace.dsl`](../../diagrams/c4/workspace.dsl)
- Detailed designs (HOW): [`../../detailed-design/`](../../detailed-design/)
- Phase 1 plan: [`../../impl-plans/2026-05-10_revival-phase-1.md`](../../impl-plans/2026-05-10_revival-phase-1.md)
