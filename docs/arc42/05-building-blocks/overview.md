---
status: Draft
owner: tensor
last-reviewed: 2026-05-10
---

# `tensor` — Building Block View (arc42 §5)

| Metadata      | Value                                                          |
| ------------- | -------------------------------------------------------------- |
| Status        | Draft                                                          |
| Type          | arc42 §5 (Building Block View)                                 |
| Owner         | uyuutosa                                                       |
| Last Updated  | 2026-05-10                                                     |

> Per `.claude/rules/documentation.md` and arc42 convention, this file is a **navigation index** for the static decomposition. The C4 L2 container diagram in [`../../diagrams/c4/workspace.dsl`](../../diagrams/c4/workspace.dsl) is the structural single source of truth; this prose mirrors it. Implementation HOW lives under [`../../detailed-design/`](../../detailed-design/).

## L1 view (system context)

The system as a whole — `tensor` — is one black box from the learner's perspective. See [`../03-context-and-scope/system-context.md`](../03-context-and-scope/system-context.md).

## L2 view (containers inside `tensor`)

Names match the DSL exactly; do not introduce synonyms.

| Container                | Phase introduced | Purpose                                                                                              | Realises ADR |
| ------------------------ | ---------------- | ---------------------------------------------------------------------------------------------------- | ------------ |
| `tensor::core`           | 1                | Named-axis tensor types (`Tensor<T,N>`, `Shape<N>`, `Axis`, `LabelTag<...>`), expression templates, `mdspan` interop, the four arithmetic ops, indexing | [0002](../09-decisions/0002-rewrite-on-cpp20-baseline-with-mdspan-interop.md), [0004](../09-decisions/0004-adopt-hybrid-named-axis-api.md) |
| `tensor::tex`            | 1 (MVP) → 3 (LyX) | `consteval` LaTeX-subset parser exposed via the `_tex` UDL, runtime parser, `to_latex` renderer; later: LyX export module | [0005](../09-decisions/0005-adopt-tex-lyx-as-authoring-surface.md) |
| `tensor::autograd`       | 2                | Tape-based reverse-mode autograd over named-axis tensors; `Variable<Tensor>` wrapper, registered backwards, gradient-checking harness | [0007](../09-decisions/0007-adopt-autograd-as-first-class-subsystem.md) |
| `tensor::gpu`            | 3                | WebGPU code generator (named-axis expression → WGSL kernel) and runtime adapter (Dawn / wgpu-native) | [0006](../09-decisions/0006-adopt-webgpu-as-gpu-backend.md) |
| `tutorials/`             | 1                | Jupyter notebooks (xeus-cling C++20 kernel) demonstrating each container; CI-executed end-to-end per release | [0008](../09-decisions/0008-distribute-as-header-only-with-jupyter-tutorials.md) |
| Jupyter Book site        | 4                | Static site generated from `tutorials/`, deployed to GitHub Pages                                    | [0008](../09-decisions/0008-distribute-as-header-only-with-jupyter-tutorials.md) |
| `lyx-export/` (optional) | 3+               | LyX module that exports tensor-bearing `.lyx` documents to `_tex`-compatible source                  | [0005](../09-decisions/0005-adopt-tex-lyx-as-authoring-surface.md) |

### Container-to-container relationships

The DSL is authoritative; this is the prose mirror.

- `tensor::autograd` **wraps** `tensor::core`. Every primitive in `core` has a corresponding registered backward in `autograd`.
- `tensor::gpu` **lowers** expressions from `tensor::core`. The named-axis expression graph is the lingua franca; `gpu` only sees the lowered form and emits WGSL.
- `tensor::tex` **produces** expression graphs that `tensor::core` consumes. Round-trip property: `parse(render(e)) == e` for every expression in the corpus.
- `tutorials/` **imports and demos** `tensor::core` (Phase 1), `tensor::autograd` (Phase 2), `tensor::gpu` (Phase 3), `tensor::tex` (all phases).
- The Jupyter Book site **renders** `tutorials/` to static HTML.
- `lyx-export/` **exports** LyX content into the `_tex` DSL of `tensor::tex`.

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

Component-level views are added per container as detailed-design docs land. None exists in Phase 1 because no container is yet structurally complex enough to warrant L3.

## Cross-references

- §3 Context and Scope: [`../03-context-and-scope/system-context.md`](../03-context-and-scope/system-context.md)
- §4 Solution Strategy: [`../04-solution-strategy/strategy.md`](../04-solution-strategy/strategy.md)
- §6 Runtime: [`../06-runtime/`](../06-runtime/) (TBD)
- §9 Decisions: [`../09-decisions/`](../09-decisions/)
- C4 source: [`../../diagrams/c4/workspace.dsl`](../../diagrams/c4/workspace.dsl)
- Detailed designs (HOW): [`../../detailed-design/`](../../detailed-design/)
- Phase 1 plan: [`../../impl-plans/2026-05-10_revival-phase-1.md`](../../impl-plans/2026-05-10_revival-phase-1.md)
