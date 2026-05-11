---
status: Draft
owner: tensor
last-reviewed: 2026-05-11
---

# `tensor` — Glossary (arc42 §12)

| Metadata      | Value                                                          |
| ------------- | -------------------------------------------------------------- |
| Status        | Draft                                                          |
| Type          | arc42 §12 (Glossary)                                           |
| Owner         | uyuutosa                                                       |
| Last Updated  | 2026-05-11                                                     |

> Per arc42 §12 and the [G-8 ubiquitous-language discipline](../01-introduction-and-goals/overview.md): every public name in this project must trace to a recognised source. This glossary is the **bibliography for that vocabulary**. Each entry names the project term, gives a one-paragraph definition, and points at the mathematical-literature / ADR / paper source the term descends from.
>
> Synonyms across the codebase, ADRs, and tutorials are bugs. If a term appears twice with different definitions, file an issue.

## Conventions

- **Bold** term — the canonical name used in code, ADRs, and tutorials.
- *Italic* term — an alternative spelling in the literature; deprecated for project use unless explicitly cited as the alternative.
- `Code identifier` — the public C++ name in `include/tensor/`.

## Domain (`tensor::core`)

### Axis

A pair of (label, extent). The **label** is a `std::string_view` (runtime) or a `FixedString` non-type template parameter (compile-time, see ADR-0004); the **extent** is a `std::size_t` index range. In the math literature this is sometimes called a *named dimension* or *named index*. The project uses **axis** throughout, matching the 2016 Qiita post's vocabulary.

- Code: `tensor::core::Axis`
- Compile-time form: `tensor::core::LabelTag<S>` via the `_ax` UDL ([ADR-0004](../09-decisions/0004-adopt-hybrid-named-axis-api.md))
- Source: Original Einstein-notation usage; modern named-tensor literature ([Maclaurin et al., "Dex" and "named tensor" papers]).

### Shape

An ordered sequence of `Axis` values. Static-rank form `Shape<N>` (where `N` is `std::size_t`) and runtime-rank form `DynamicShape`.

- Code: `tensor::core::Shape<N>`, `tensor::core::DynamicShape`
- Source: standard tensor terminology; project's hybrid static/dynamic split is from [ADR-0004](../09-decisions/0004-adopt-hybrid-named-axis-api.md).

### Tensor

The Domain centerpiece: a multi-dimensional array with a `Shape` and an element type `T`. Static-rank `Tensor<T, N>` (rank known at compile time, labels runtime) and runtime-rank `DynamicTensor<T>` (rank and labels runtime). The compile-time-labelled variant is `TypedTensor<T, "i", "j", ...>` (via `FixedString` NTTPs).

- Code: `tensor::core::Tensor<T, N>`, `tensor::core::DynamicTensor<T>`, `tensor::core::TypedTensor<T, Labels...>`
- See [`docs/user-manual/how-to/named-tensor-types.md`](../../user-manual/how-to/named-tensor-types.md) for the decision guide on which to pick.

### Broadcast (Einstein-style)

The named-axis project's interpretation of broadcasting: two tensors `a_i` and `b_j` with disjoint axis labels combine via outer product into `c_{ij}`; with overlapping labels they align on those labels and broadcast along the rest. Distinct from NumPy's positional broadcasting.

- Code: `tensor::core::broadcast_shapes`, `BroadcastPlan`, `project_index`, `unbroadcast`
- Source: Einstein-notation convention; the project's API descends from the 2016 named-axis design.

### Contraction (Einstein-style)

The Einstein-convention inner product: summation over shared axis labels. With a single shared label, contraction is matrix-vector or matrix-matrix product depending on the rank pair. With multiple shared labels, full multi-index summation.

- Code: `tensor::core::contract`, `contract_with_plan`, `ContractPlan`, `tensor::autograd::dot`
- Source: standard Einstein-notation summation; ([ADR-0011](../09-decisions/0011-kernel-backend-port-api.md) names this op as one of the 15 in the `KernelBackend` port).

### `mdspan` interop

A non-owning, multi-dimensional view of a tensor's data buffer, compatible with C++23's `std::mdspan` and the `kokkos/mdspan` reference implementation (for builds where `<mdspan>` is not yet available — see [ADR-0002](../09-decisions/0002-rewrite-on-cpp20-baseline-with-mdspan-interop.md)).

- Code: `tensor::core::mdview`, `tensor::core::from_mdspan`
- Source: ISO C++ paper [P0009](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p0009r17.html); polyfill from [kokkos/mdspan](https://github.com/kokkos/mdspan).

## Autograd (`tensor::autograd`)

### Variable

A `Tensor` wrapper that participates in automatic differentiation. Holds a forward value, an accumulated gradient (`GradAccum`), and a registered backward function. `Variable<T, N>` for static-rank, `DynamicVariable<T>` for runtime-rank.

- Code: `tensor::autograd::Variable<T, N>`, `tensor::autograd::DynamicVariable<T>`
- Source: tape-based reverse-mode autodiff, modeled on micrograd / tinygrad ([ADR-0007](../09-decisions/0007-adopt-autograd-as-first-class-subsystem.md)).

### Tape

A thread-local append-only list of registered backward closures. `backward()` walks the tape in reverse, invoking each closure to propagate gradients from outputs to inputs.

- Code: `tensor::autograd::Tape` (thread-local singleton)
- Source: Wengert tape, classical reverse-mode autodiff ([Wengert 1964], rediscovered as "tape-based autodiff" in modern ML frameworks).

### Gradient check

A finite-difference test that the analytical gradient computed by `backward()` matches the central-difference estimate of the derivative. The canonical correctness witness for an autograd implementation.

- Code: `tensor::autograd::gradient_check`
- Source: standard ML practice; used in micrograd, tinygrad, JAX test suites.

## TeX bridge (`tensor::tex`)

### `_tex` UDL

A user-defined literal that parses a LaTeX-subset expression at compile time (via `consteval`) into an `Expression` AST. The project's slogan *the formula is the program* originates here ([ADR-0005](../09-decisions/0005-adopt-tex-lyx-as-authoring-surface.md)).

- Code: `tensor::tex::operator""_tex`
- Source: the project's own ADR; influenced by [LyX](https://www.lyx.org/) and TeX-based authoring traditions.

### Expression (`tensor::tex::Expression`)

The AST node type produced by `parse()` and consumed by `Evaluator`. Variants: `IndexedVar`, `BinOp`, `Sum`, `Equation`, `Group`.

- Code: `tensor::tex::Expression`
- Source: standard AST design; specific node set matches the LaTeX subset declared in [ADR-0005](../09-decisions/0005-adopt-tex-lyx-as-authoring-surface.md).

### Evaluator (`tensor::tex::Evaluator`)

Binds named tensors to AST `IndexedVar` leaves, then evaluates the expression into a `DynamicTensor<T>`. The bridge from *the formula* to *the program*'s result.

- Code: `tensor::tex::Evaluator<T>`
- Source: standard tree-walking interpreter; the project's own ADR.

## Architecture (cross-cutting)

### Domain (Hexagonal lite)

The set of headers under `include/tensor/{core,autograd,tex}/` that may not depend on any adapter (other than via the C++20 concepts declared in `concepts.hpp`). See [ADR-0009](../09-decisions/0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md).

- Source: DDD ([Eric Evans, *Domain-Driven Design*, 2003]) + Hexagonal Architecture ([Alistair Cockburn, "Ports and Adapters", 2005]).

### Port

A C++20 `concept` declared in one of the Domain containers' `concepts.hpp` headers. Defines an interface that DrivenAdapters (e.g. `KernelBackend`) must satisfy. The Domain depends only on its ports, never on a specific adapter.

- Code: `tensor::core::KernelBackend`, `TensorLike`, etc.
- Source: Hexagonal Architecture terminology; the project's [ADR-0009](../09-decisions/0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md) + [ADR-0011](../09-decisions/0011-kernel-backend-port-api.md).

### Adapter (Driving / Driven)

A header outside the Domain that either *drives* the Domain (e.g. `tensor::tex` produces expression graphs) or is *driven by* the Domain (e.g. `tensor::core::backend::eigen` implements the `KernelBackend` port).

- Source: Hexagonal Architecture terminology.

### `KernelBackend` (the port)

The 15-method C++20 concept that every kernel-execution adapter must satisfy. Currently three concrete adapters: `reference`, `eigen`, `webgpu` (stub, dispatch wiring deferred).

- Code: `tensor::core::KernelBackend` (in `concepts.hpp`)
- Source: [ADR-0011](../09-decisions/0011-kernel-backend-port-api.md).

### Canonical reference (positioning)

The project's stated identity: *"the canonical reference for differentiable named-axis tensor computation in modern C++"*. Operationalised by three disciplines — bibliography (ADR sequence), ubiquitous language (this glossary), reproducibility (clean-clone build/bench/notebook under 30 minutes).

- Source: [ADR-0013](../09-decisions/0013-reframe-as-canonical-reference-for-named-tensor-computation.md).

## Substrate (vendored / external)

### gpu.cpp

[AnswerDotAI/gpu.cpp](https://github.com/AnswerDotAI/gpu.cpp): a thin C++ wrapper around the WebGPU C API. Vendored under [`third_party/gpu_cpp/gpu.hpp`](../../../third_party/gpu_cpp/gpu.hpp) at tag `0.2.0` per [ADR-0014](../09-decisions/0014-external-substrate-strategy.md).

### Dawn

[Google Dawn](https://dawn.googlesource.com/dawn): the WebGPU implementation behind Chrome's WebGPU. The project's planned WebGPU build path is `find_package(dawn CONFIG REQUIRED)` after a vcpkg baseline bump (deferred to P3.M3.2 / P3.M4.2).

### WGSL

WebGPU Shading Language. The kernel sources committed under [`include/tensor/core/backend/webgpu_wgsl.hpp`](../../../include/tensor/core/backend/webgpu_wgsl.hpp) are WGSL with gpu.cpp's `{{precision}}` / `{{workgroupSize}}` placeholders.

### xeus-cpp

[compiler-research/xeus-cpp](https://github.com/compiler-research/xeus-cpp): the Jupyter C++ kernel built on Clang-Repl + CppInterOp. Successor to xeus-cling for this project's notebook CI (PR #42).

### `std::linalg` (P1673)

C++26 standard linear algebra surface. Specified in [P1673R13](https://isocpp.org/files/papers/P1673R13.html); not yet shipped in libc++ / libstdc++ / MSVC STL as of 2026-05. The project plans a `tensor::linalg` namespace shim over [kokkos/stdBLAS](https://github.com/kokkos/stdBLAS) per [ADR-0014](../09-decisions/0014-external-substrate-strategy.md).

## Document framework

### MADR

[Markdown Architectural Decision Records](https://adr.github.io/madr/), v3.0. The format every ADR under [`docs/arc42/09-decisions/`](../09-decisions/) follows. Includes the Y-statement format pioneered by Olaf Zimmermann.

### Y-statement

A single-sentence summary of an architecture decision: *"In the context of X, facing Y, we decided for Z to achieve Q, accepting downside R."* Required in every ADR's Decision Outcome section.

### Hexagonal lite

The project's variant of Hexagonal Architecture: full Domain / Driving / Driven classification but *without* the formal command-bus / event-bus machinery that "full" Hexagonal sometimes implies. See [ADR-0009](../09-decisions/0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md) for the choice rationale.

### Diátaxis

The [Diátaxis framework](https://diataxis.fr/) for documentation: tutorials, how-tos, reference, explanation. The project organises `tutorials/` (tutorial type), `docs/user-manual/how-to/` (how-to type), `docs/arc42/` (reference type), `docs/reports/` (explanation type).

### CFF

[Citation File Format](https://citation-file-format.github.io/) v1.2.0. The format of [`CITATION.cff`](../../../CITATION.cff). Powers GitHub's "Cite this repository" UI.

## Cross-references

- §1 §G-8 (citability discipline) where this glossary plays a role: [`../01-introduction-and-goals/overview.md`](../01-introduction-and-goals/overview.md)
- ADRs cited above: [`../09-decisions/`](../09-decisions/)
- `CITATION.cff` (the work's own citation metadata): [`../../../CITATION.cff`](../../../CITATION.cff)
- Diátaxis-typed documentation roots: [`tutorials/`](../../../tutorials/), [`docs/user-manual/how-to/`](../../user-manual/how-to/), [`docs/arc42/`](../), [`docs/reports/`](../../reports/)
