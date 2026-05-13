---
status: Draft
owner: tensor
last-reviewed: 2026-05-12
---

# `tensor` — Building Block View (arc42 §5)

| Metadata      | Value                                                          |
| ------------- | -------------------------------------------------------------- |
| Status        | Draft                                                          |
| Type          | arc42 §5 (Building Block View)                                 |
| Owner         | uyuutosa                                                       |
| Last Updated  | 2026-05-12                                                     |

> Per `.claude/rules/documentation.md` and arc42 convention, this file is a **navigation index** for the static decomposition. The C4 L2 container diagram in [`../../diagrams/c4/workspace.dsl`](../../diagrams/c4/workspace.dsl) is the structural single source of truth; this prose mirrors it. Implementation HOW lives under [`../../detailed-design/`](../../detailed-design/).

## L1 view (system context)

The system as a whole — `tensor` — is one black box from the learner's perspective. See [`../03-context-and-scope/system-context.md`](../03-context-and-scope/system-context.md).

## L2 view (containers inside `tensor`)

Names match the DSL exactly; do not introduce synonyms.

The library is laid out as a Hexagonal (Ports & Adapters) "lite" architecture per [ADR-0009](../09-decisions/0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md). Each container below carries a **hexagonal classification** — `Domain`, `DrivingAdapter`, or `DrivenAdapter`. The dependency rule is one-way: `Domain` depends on no adapter; adapters depend on `Domain`; **no adapter depends on another adapter directly** ([`../../design-guide/architectural-discipline.md`](../../design-guide/architectural-discipline.md) explains and how it is enforced).

**Shipped as of 2026-05-12** (after PR #1–#75): `tensor::core` ✅ (incl. `LabelTag` + `TypedTensor` compile-time path, mdspan interop); `tensor::tex` ✅ (parser + `to_latex` + `_tex` UDL + Evaluator end-to-end); `tensor::autograd` ✅ (Phase 2 full: MVP, activations, broadcast backward, contraction, `zero_grad`, `sgd_update`); **three backend adapters** ✅ (`reference` canonical, `eigen` Eigen 3.4 SIMD + GEMM, `webgpu` Dawn — the WebGPU adapter dispatches real GPU compute on 12 of the 15 `KernelBackend` methods for `float` on RTX 3090 per [ADR-0012](../09-decisions/0012-webgpu-adapter-implementation-design.md) + [ADR-0016](../09-decisions/0016-substrate-refinement-drop-gpu-cpp-talk-to-dawn-directly.md); the remaining three — `reduce_sum`, `unbroadcast`, non-simple-GEMM `contract` — delegate to reference, matching the Eigen adapter's scope); tutorials 00 / 05 / 06 / 07 / 08 ✅; Jupyter Book TOC populated ✅ ([`book/`](../../../book/)); perf bench framework + RTX 3090 three-backend measurements ✅ ([`../../../bench/`](../../../bench/)); LyX export module + plugin + golden-file CI smoke ✅ ([`../../../lyx-export/`](../../../lyx-export/)); `CONTRIBUTING.md` + `CODE_OF_CONDUCT.md` + `CHANGELOG.md` + `CITATION.cff` + `docs/INDEX.md` ✅; xeus-cling + xeus-cpp notebook CI ✅. **Remaining for `0.1.0` close**: maintainer-only release ceremony (release branch / version bump / tag on `main` / back-merge / GitHub Pages enable) per the [Phase 4 release rehearsal report](../../reports/2026-05-11_phase-4-release-rehearsal.md) §3 checklist.

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
   Python bindings      (DrivingAdapter, future — Axis H, Phase 6)
   lyx-export           (DrivingAdapter, shipped)
                                                      ▲
   tensor::core::backend::reference (DrivenAdapter, canonical KernelBackend)
   tensor::core::backend::eigen     (DrivenAdapter, KernelBackend via Eigen 3.4)
   tensor::core::backend::webgpu    (DrivenAdapter, KernelBackend via Dawn)
   mdspan exporter      (DrivenAdapter, BufferExporter port)
   LaTeX renderer       (DrivenAdapter, ExpressionSink port)
   Autograd tape writer (DrivenAdapter, BackwardSink port)
```

| Container                | Hex. class.       | Phase introduced | Purpose                                                                                              | Realises ADR |
| ------------------------ | ----------------- | ---------------- | ---------------------------------------------------------------------------------------------------- | ------------ |
| `tensor::core`           | **Domain**        | 1 ✅              | Named-axis tensor types (`Tensor<T,N>`, `Shape<N>`, `Axis`, `LabelTag<...>`), expression templates, `mdspan` interop, the four arithmetic ops, indexing. Owns `concepts.hpp` declaring `TensorLike`, `Shape`, `Axis`, `KernelBackend`, `BufferExporter` ports. | [0002](../09-decisions/0002-rewrite-on-cpp20-baseline-with-mdspan-interop.md), [0004](../09-decisions/0004-adopt-hybrid-named-axis-api.md), [0009](../09-decisions/0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md), [0011](../09-decisions/0011-kernel-backend-port-api.md) |
| `tensor::autograd`       | **Domain** (ext.) | 2 ✅              | Tape-based reverse-mode autograd over named-axis tensors; `Variable<Tensor>` wrapper, registered backwards, gradient-checking harness, `zero_grad` + `sgd_update`. Declares `Differentiable`, `BackwardOp`, `BackwardSink` concepts in its own `concepts.hpp`. | [0007](../09-decisions/0007-adopt-autograd-as-first-class-subsystem.md), [0009](../09-decisions/0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md) |
| `tensor::tex`            | **DrivingAdapter** | 1 ✅ (MVP) → 1.5 ✅ (Evaluator) | LaTeX-subset parser exposed via the `_tex` UDL + runtime parser + `to_latex` renderer + `Evaluator<T>` that runs parsed expressions to `DynamicTensor`. Implements the `ExpressionSource` and `ExpressionSink` ports declared in `tensor::core::concepts`. | [0005](../09-decisions/0005-adopt-tex-lyx-as-authoring-surface.md) |
| `tensor::core::backend::reference` | **DrivenAdapter** | 2.5 ✅ | Canonical CPU implementation of the `KernelBackend` port — every other adapter cross-validates against this one. | [0010](../09-decisions/0010-refine-positioning-to-educational-first-production-capable.md), [0011](../09-decisions/0011-kernel-backend-port-api.md) |
| `tensor::core::backend::eigen` | **DrivenAdapter** | 2.5 ✅ | Eigen 3.4 SIMD + GEMM adapter for the `KernelBackend` port; delegates out-of-scope methods to the reference adapter. | [0010](../09-decisions/0010-refine-positioning-to-educational-first-production-capable.md), [0011](../09-decisions/0011-kernel-backend-port-api.md) |
| `tensor::core::backend::webgpu` | **DrivenAdapter** | 3 ✅ | Dawn-backed WebGPU adapter for the `KernelBackend` port. As of 2026-05-12, 12 of 15 methods dispatch real GPU compute on `float` on RTX 3090 (4 binary + 4 unary + 1 contract + 3 broadcast); the rest delegate to reference. Talks to Dawn directly via Dawn's own `webgpu_cpp.h` per [ADR-0016](../09-decisions/0016-substrate-refinement-drop-gpu-cpp-talk-to-dawn-directly.md). (Namespace renamed from the pre-ADR-0011 `tensor::gpu`.) | [0006](../09-decisions/0006-adopt-webgpu-as-gpu-backend.md), [0011](../09-decisions/0011-kernel-backend-port-api.md), [0012](../09-decisions/0012-webgpu-adapter-implementation-design.md), [0016](../09-decisions/0016-substrate-refinement-drop-gpu-cpp-talk-to-dawn-directly.md) |
| `tutorials/`             | (out of hexagon)  | 1 ✅              | Six Jupyter notebooks (xeus-cpp 0.10+ `xcpp20` kernel, with xeus-cling kept as legacy smoke for 00): 00 intro, 01 formula-is-the-program, 05 autograd-from-scratch, 06 webgpu-acceleration, 07 mlp-on-toy, 08 swappable-backends. | [0008](../09-decisions/0008-distribute-as-header-only-with-jupyter-tutorials.md), [0014](../09-decisions/0014-external-substrate-strategy.md) |
| Jupyter Book site        | (out of hexagon)  | 4 (TOC ✅)        | Static site generated from `book/_toc.yml` referencing `tutorials/` + arc42 + detailed-design + reports; deployed to GitHub Pages on Pages-enable (maintainer-only). | [0008](../09-decisions/0008-distribute-as-header-only-with-jupyter-tutorials.md) |
| `lyx-export/`            | **DrivingAdapter** | 1.5 ✅           | LyX module + Python translator that exports tensor-bearing `.lyx` documents to `_tex`-compatible source; golden-file CI smoke verifies the translator. | [0005](../09-decisions/0005-adopt-tex-lyx-as-authoring-surface.md) |
| `python/`                | **DrivingAdapter** | 6 ✅ (M1–M6 shipped) → 6.5 🚧 (deferred `set_backend()`) | Python SDK via nanobind. The `tensor` package wraps the C++ Domain end-to-end: `DynamicTensor` + `DynamicTensorF32` + arithmetic with Einstein-style broadcast (M2); `contract` + NumPy interop (M3); `tensor.autograd` with `DynamicVariable` + `dot` + `sum_all` + `backward` + `sgd_update` + activations `exp` / `log` / `relu` / `neg` / `sin` / `cos` / `sqrt` + `__truediv__` + `reduce_along_label` (M4 + Bundle B); `tensor.tex` with `parse` + `to_latex` + `Evaluator` / `EvaluatorF32` (M5); cibuildwheel matrix + PyPA trusted-publishing release infrastructure (M6). Runtime `tensor.set_backend()` between `reference` / `eigen` / `webgpu` is the Phase 6.5 deliverable, packaged via PEP-508 extras per [ADR-0019](../09-decisions/0019-phase-6-5-runtime-backend-selection-via-extras.md). Boundary patterns: [`../../detailed-design/python-sdk-binding-surface.md`](../../detailed-design/python-sdk-binding-surface.md). | [0018](../09-decisions/0018-phase-6-python-sdk-entry-via-nanobind.md), [0019](../09-decisions/0019-phase-6-5-runtime-backend-selection-via-extras.md), [0009](../09-decisions/0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md) |
| `python/extras/` (planned for Phase 6.5) | **DrivingAdapter** (companion projects) | 6.5 🚧 | Two companion PyPI distributions under the same `tensor/` PEP-420 namespace: `tensor-named-axis-eigen` and `tensor-named-axis-webgpu`. Each ships only the matching `_tensor_native_<name>.so` and declares `tensor-named-axis==<exact-version>` as a runtime dep. The base distribution's `[project.optional-dependencies]` (`eigen` / `webgpu` / `all`) pulls them in. | [0019](../09-decisions/0019-phase-6-5-runtime-backend-selection-via-extras.md) |
| `huggingface/space/`     | **DrivingAdapter** (out-of-hexagon adapter to a hosted demo) | 6 ✅ (scaffold + `deploy.sh`) | Gradio app (three tabs: `tex.Evaluator`, named-axis broadcast, autograd training loop) that exhibits the Python SDK on HuggingFace Spaces without local install. Source-of-truth in-tree; canonical deploy target is a separate `huggingface.co/spaces/<user>/tensor-named-axis-demo` Git remote. | [0018](../09-decisions/0018-phase-6-python-sdk-entry-via-nanobind.md) |

**Cross-cutting GoF patterns** named where they apply (per ADR-0009): Composite for expression trees, Visitor for AST traversals (parse / WGSL codegen / LaTeX render / backward), Strategy for `KernelBackend` selection, Command for autograd tape entries, Bridge to keep the named-axis runtime and NTTP fast paths over a single underlying kernel implementation.

### Container-to-container relationships

The DSL is authoritative; this is the prose mirror, expressed in hexagonal direction.

- `tensor::autograd` **extends the Domain hexagon**: it consumes `tensor::core` types and concepts, never the other way around.
- `tensor::tex` (DrivingAdapter) **produces** expression graphs by consuming the `tensor::core` `ExpressionSource` port. Round-trip property: `parse(render(e)) == e` for every expression in the corpus.
- Each of the three `tensor::core::backend::{reference,eigen,webgpu}` adapters (DrivenAdapter) **implements** the `KernelBackend` port declared by `tensor::core::concepts`; the Domain is unaware of which adapter is linked. `eigen` and `webgpu` keep a private `reference::Backend ref_` member to delegate out-of-scope methods to the canonical implementation — the pattern documented in [`../../detailed-design/kernel-backend-port.md`](../../detailed-design/kernel-backend-port.md).
- `tutorials/` **imports and demos** `tensor::core` + `tensor::tex` + `tensor::autograd` (notebooks 00 / 05 / 07), the Hexagonal-lite payoff via Eigen (notebook 08), and the WebGPU adapter design (notebook 06). Tutorials are *outside* the hexagon — they are demos, not adapters.
- The Jupyter Book site **renders** `tutorials/` + arc42 + detailed-design + reports to static HTML — fully out of hexagon.
- `lyx-export/` (DrivingAdapter) **exports** LyX content into the `_tex` DSL of `tensor::tex` — adapter-to-adapter via Domain only, never directly.

**The single hard rule (ADR-0009 + ADR-0011)**: a header under `include/tensor/core/` that is not `concepts.hpp` and not under `include/tensor/core/backend/<adapter>/` may not `#include` anything from `include/tensor/{tex,autograd}/`; and no header under `include/tensor/core/backend/<adapter>/` may include from another adapter. CI enforces this; see [`../../design-guide/architectural-discipline.md`](../../design-guide/architectural-discipline.md).

**Enforcement examples** — the exact `grep` invocations that catch a violation (also recorded in [`../../design-guide/architectural-discipline.md`](../../design-guide/architectural-discipline.md) §5):

```bash
# Core depending on autograd / tex — must return nothing.
grep -nE '#include[ ]+<tensor/(autograd|tex)' \
    include/tensor/core/*.hpp include/tensor/core/*/*.hpp \
    | grep -v include/tensor/core/concepts.hpp

# Adapter depending on a sibling adapter — must return nothing.
for adapter in reference eigen webgpu; do
  others=$(echo "reference eigen webgpu" | tr ' ' '\n' | grep -v $adapter | paste -sd'|' -)
  grep -nE "#include[ ]+<tensor/core/backend/($others)/" \
      include/tensor/core/backend/$adapter/*.hpp
done

# autograd depending on tex (or vice-versa) — must return nothing.
grep -nE '#include[ ]+<tensor/tex' include/tensor/autograd/*.hpp
grep -nE '#include[ ]+<tensor/autograd' include/tensor/tex/*.hpp
```

When any of these returns a line, the violation must be fixed before merge (per OC-3's PR-time discipline). A future cycle may package these into a `tools/check-hexagon.sh` script and add it to the CI lint workflow alongside `tools/check-vendored.sh`.

### Container ownership table

Every container in §5's main table maps to exactly one source directory under the repo root. Lookup table for contributors landing in unfamiliar code:

| Container                        | Source directory                                                            | Test directory                                 |
| -------------------------------- | --------------------------------------------------------------------------- | ---------------------------------------------- |
| `tensor::core`                   | `include/tensor/core/`                                                       | `tests/test_core_*.cpp`, `tests/test_dynamic_*.cpp`, `tests/test_typed_tensor.cpp`, `tests/test_label_tag.cpp`, `tests/test_axis_shape.cpp`, `tests/test_contract.cpp`, etc. |
| `tensor::autograd`               | `include/tensor/autograd/`                                                   | `tests/test_autograd_*.cpp`                    |
| `tensor::tex`                    | `include/tensor/tex/`                                                        | `tests/test_tex_*.cpp`, `tests/test_evaluator*.cpp` |
| `tensor::core::backend::reference` | `include/tensor/core/backend/reference/`                                    | implicit (other tests run against it as canonical) |
| `tensor::core::backend::eigen`   | `include/tensor/core/backend/eigen/`                                         | `tests/test_eigen_backend.cpp`                 |
| `tensor::core::backend::webgpu`  | `include/tensor/core/backend/webgpu/`                                        | `tests/test_webgpu_backend.cpp`                |
| `python/`                        | `python/src/`, `python/tensor/`, `python/CMakeLists.txt`                     | `python/tests/test_*.py`                       |
| `python/extras/` (Phase 6.5 planned) | `python/extras/{eigen,webgpu}/`                                          | `python/tests/test_backend_parity.py`          |
| `huggingface/space/`             | `huggingface/space/`                                                         | manual smoke via `python app.py` locally       |
| `lyx-export/`                    | `lyx-export/`                                                                | `lyx-export/tests/`, golden-file CI            |
| Jupyter Book site                | `book/` + `book/_toc.yml` + `book/stage.sh`                                  | manual `jupyter-book build book` smoke         |
| CI workflows                     | `.github/workflows/{ci,deploy-book,notebook-ci,lyx-export-ci,python-wheel-smoke,cibuildwheel}.yml` | self-testing                          |

## Why this decomposition

Each container exists because at least one of the §1 goals or one runtime scenario in [`../06-runtime/`](../06-runtime/) requires it:

| Container          | Justifying goals                                      | Cut if…                                                |
| ------------------ | ----------------------------------------------------- | ------------------------------------------------------ |
| `tensor::core`     | G-1 (named axes), G-2 (modern C++)                    | Never — this *is* the project.                         |
| `tensor::tex`      | G-3 ("formula is the program")                        | Drop only if ADR-0005 is superseded.                   |
| `tensor::autograd` | G-4 (end-to-end teaching arc)                         | Drop only if ADR-0007 is superseded back to no-autograd. |
| `tensor::core::backend::{reference,eigen,webgpu}` | G-4 (teaching arc), G-5 (zero-friction install) | The reference adapter is never cut (it is the canonical implementation). `eigen` drops only if ADR-0010 is rolled back. `webgpu` drops only if ADR-0006 is superseded. |
| `tutorials/`       | G-4 (teaching arc), G-5 (install)                     | Drop only if ADR-0008 is superseded; never realistic.   |
| Jupyter Book site  | G-6 (living book)                                     | Drop only if ADR-0008 is partially superseded.          |
| `lyx-export/`      | G-3 (TeX/LyX surface)                                 | Drop only if ADR-0005 is superseded.                    |

Containers were introduced phase-by-phase per [`../../impl-plans/`](../../impl-plans/); the [Phase 1 retrospective](../../reports/2026-05-11_phase-1-retrospective.md) and [Phase 2 + 2.5 retrospective](../../reports/2026-05-11_phase-2-and-2-5-retrospective.md) record the introduction order.

## Naming conventions

- **C++ namespace mirrors container name**: `tensor::core::Tensor`, `tensor::autograd::Variable`, `tensor::core::backend::webgpu::Backend`, etc.
- **Headers live under `include/tensor/<container>/`** (e.g. `include/tensor/core/tensor.hpp`, `include/tensor/tex/parser.hpp`, `include/tensor/core/backend/webgpu.hpp`).
- **Container-internal types are not part of the public ABI / API.** Only types in `include/tensor/<container>/<container>.hpp` are user-facing.
- **No abbreviations in container names.** `webgpu`, not `wgpu`; `autograd`, not `ag`. Educational pitch demands names that read. The `tensor::gpu` namespace from before [ADR-0011](../09-decisions/0011-kernel-backend-port-api.md) was renamed to `tensor::core::backend::webgpu` to match the port-and-adapter discipline.

## What is *not* here

- *How* `Tensor<T,N>` stores data, *how* the `consteval` parser is structured, *how* the autograd tape is allocated → [`../../detailed-design/`](../../detailed-design/).
- *When* a backward pass runs, *when* a WGSL dispatch happens → [`../06-runtime/`](../06-runtime/).
- *Why* WebGPU and not CUDA → [ADR-0006](../09-decisions/0006-adopt-webgpu-as-gpu-backend.md).

## L3 (component) zoom-ins

Component-level views land as detailed-design docs under [`../../detailed-design/`](../../detailed-design/). Current instances:

- [`tensor-core.md`](../../detailed-design/tensor-core.md) — Domain centerpiece.
- [`tensor-autograd.md`](../../detailed-design/tensor-autograd.md) — tape-based reverse-mode autograd over named-axis tensors.
- [`tensor-tex.md`](../../detailed-design/tensor-tex.md) — `_tex` UDL parser, `Expression` AST, `Evaluator`, LyX export module.
- [`webgpu-element-wise-kernels.md`](../../detailed-design/webgpu-element-wise-kernels.md) — WebGPU element-wise binary + unary kernels (P3.M3).
- [`webgpu-gemm-kernel.md`](../../detailed-design/webgpu-gemm-kernel.md) — WebGPU tiled GEMM (P3.M4).
- [`webgpu-broadcast-kernels.md`](../../detailed-design/webgpu-broadcast-kernels.md) — WebGPU label-aware broadcast (P3.M5).
- [`kernel-backend-port.md`](../../detailed-design/kernel-backend-port.md) — the `KernelBackend` C++20 concept that decouples the Domain from execution (14 methods + `backend_tag`).

All originally-planned detailed-design instances now ship.

## Cross-references

- §3 Context and Scope: [`../03-context-and-scope/system-context.md`](../03-context-and-scope/system-context.md)
- §4 Solution Strategy: [`../04-solution-strategy/strategy.md`](../04-solution-strategy/strategy.md)
- §6 Runtime: [`../06-runtime/overview.md`](../06-runtime/overview.md) — four scenarios (broadcast / `_tex` / autograd / backend swap)
- §9 Decisions: [`../09-decisions/`](../09-decisions/)
- C4 source: [`../../diagrams/c4/workspace.dsl`](../../diagrams/c4/workspace.dsl)
- Detailed designs (HOW): [`../../detailed-design/`](../../detailed-design/)
- Phase 1 plan: [`../../impl-plans/2026-05-10_revival-phase-1.md`](../../impl-plans/2026-05-10_revival-phase-1.md)
