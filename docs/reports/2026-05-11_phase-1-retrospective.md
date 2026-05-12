---
status: Stable
owner: tensor
last-reviewed: 2026-05-11
---

# Phase 1 retrospective — `tensor` revival, 2026-05-10 → 2026-05-11

| Metadata      | Value                                                          |
| ------------- | -------------------------------------------------------------- |
| Status        | Stable                                                         |
| Type          | Layer B — retrospective report (dated, append-only)            |
| Owner         | uyuutosa                                                       |
| Related plan  | [`../impl-plans/2026-05-10_revival-phase-1.md`](../impl-plans/2026-05-10_revival-phase-1.md) |
| Successor     | [`../impl-plans/2026-05-11_phase-2-autograd.md`](../impl-plans/2026-05-11_phase-2-autograd.md) |

## Executive summary

Phase 1 closed in **two compressed days** (the original plan targeted seven weeks). The library now has a modern build, six Domain headers, a complete LaTeX-subset parsing adapter (`tensor::tex`), a working test suite, and a first runnable Jupyter notebook. CI is green on all eight cross-platform jobs. The autograd MVP that opens Phase 2 also landed in the same window. The deferred items — M3 compile-time path, M4 evaluator bridge, and mdspan polyfill — collectively form the Phase 1.5 mop-up backlog.

## What shipped

| Domain area                          | Files                                                           | PRs                  |
| ------------------------------------ | --------------------------------------------------------------- | -------------------- |
| Documentation scaffold (pentaglyph)  | `docs/`, `.claude/`, 9 ADRs                                     | #1, #3               |
| arc42 §1 / §3 / §4 / §5              | overview / system-context / strategy / building-blocks          | #1, #3               |
| C4 model (Structurizr DSL)           | `docs/diagrams/c4/workspace.dsl`                                | #1, #3               |
| Build system                         | CMake + vcpkg + CI matrix (Ubuntu/macOS/Windows × GCC/Clang/AppleClang/MSVC × Debug/Release) | #2, #9 |
| Core types                           | `Axis`, `Shape<N>`, `Tensor<T, N>`, `concepts.hpp`              | #4                   |
| Format                               | `operator<<` with 2016 ASCII-style banner                       | #4                   |
| Runtime broadcast                    | `DynamicShape`, `DynamicTensor<T>`, `broadcast_shapes`, `+ - * /` | #5                |
| TeX authoring surface                | `tensor::tex::{Expression, parse, to_latex}`, `_tex` UDL        | #6                   |
| Function / Reference tensors         | `FunctionTensor<F>`, `ReferenceTensor<T>` (modernized)          | #7                   |
| Tutorials                            | `tutorials/00_intro.ipynb` + environment + README               | #8                   |
| Autograd MVP                         | `Variable<T, N>`, `Tape`, `sum_all`, `backward`, `gradient_check` | #10                |
| CI green-up                          | vcpkg manifest, polyfill defer, test-target warning suppression | #9                   |

11 merged PRs across the window. CI green achieved on PR #9 after five iterations diagnosing vcpkg, polyfill, and test-discipline issues.

## What hit the 2016 README byte-for-byte

| 2016 example                                  | Reproduced on new API |
| --------------------------------------------- | --------------------- |
| `a + b` 5×5 sum table (`i`, `j` distinct)     | ✅ exact              |
| `a - b`, `a * b`, `a / b` 5×5 tables          | ✅ exact              |
| `a * f` with `f(i,v) = v + 2i` → `(1,4,7,10,13)` | ✅ exact         |
| `r * 3` reference tensor → `(9,27,81,243,729)`  | ✅ exact         |

## What was harder than planned

1. **vcpkg manifest baseline**. Five CI iterations to chase down (a) missing `builtin-baseline`, (b) shallow clone losing port-version trees, (c) `version>=` constraints clashing with baseline-fixed ports, (d) the Kokkos `mdspan` polyfill placing symbols in `Kokkos::` rather than `std::` / `std::experimental::`, and (e) the `[[nodiscard]]` × `CHECK_THROWS_AS` warning interaction. Items (a)–(c) and (e) resolved cleanly; (d) deferred to Phase 1.5.
2. **Test-design gap**. `parse("a_i garbage trailing")` was supposed to throw on trailing characters, but the parser's juxtaposition rule consumes "garbage" and "trailing" as legal implicit multiplications. The test was wrong, not the parser. Lesson: when adopting a fluent / consume-everything grammar, *trailing-character* tests need sentinel characters that explicitly cannot start an atom.
3. **`Shape<N>::rank` API drift**. Mid-stream, harmonising the `rank` API between static- and runtime-rank shapes required moving from a static data member to a method. The `ShapeLike` / `TensorLike` concepts were simplified at the same time. No external API breakage, but the refactor surfaced because writing the new `DynamicShape` made the inconsistency visible.

## What was easier than planned

1. **MADR ADRs as design closure**. Each design tension (production-vs-educational, named-axis hybrid model, GPU strategy, autograd model, distribution medium, TeX/LyX surface, hexagonal structure) became an ADR before any code was written. Subsequent code reviewers (here: future-self / contributors) inherited the *why* alongside the *what*.
2. **Hexagonal "lite" (ADR-0009)** mapped onto containers with zero friction. `tensor::core` and `tensor::autograd` are Domain; `tensor::tex` is DrivingAdapter; the (deferred) `tensor::gpu` will be DrivenAdapter. Ports declared as C++20 concepts in `concepts.hpp` per container; the discipline already paid off when the autograd MVP only needed to consume `tensor::core::Tensor` + `Shape`.
3. **Workflow throughput**. The pentaglyph scaffold's `documentation.md` rule made it natural to update §5 building-blocks and the impl-plan alongside each PR. The discipline did not slow anything down.

## Deferred items (Phase 1.5 backlog)

- **M3 compile-time path**: `FixedString<N>` NTTP helper, `LabelTag<S>`, `_ax` user-defined literal, `static_assert`-quality compile-time axis-mismatch errors. The runtime path is fully shipped; the compile-time fast path is an additive enrichment.
- **M4 evaluator bridge**: `tensor::tex::Expression` → `tensor::core::DynamicTensor` evaluator that accepts named bindings and reduces an AST to a concrete tensor.
- **mdspan polyfill `Kokkos::` namespace adapter**: re-enable `mdspan_interop.hpp` against the vcpkg port's actual namespace. Re-add `mdspan` to `vcpkg.json`. Re-enable `test_mdspan_interop.cpp` in `tests/CMakeLists.txt`. Document the choice in `docs/design-guide/`.
- **xeus-cling notebook execution in CI** — runs every release-tagged commit; currently the notebook is JSON-validated only.

These deferrals are tracked in a separate Phase 1.5 mop-up impl-plan (TBD) and on this report's predecessor record.

## Numbers

- 11 merged PRs in window.
- 9 ADRs in `Accepted` status.
- Six arc42 sections (§1, §3, §4, §5, §9 ADRs, §12 glossary stub) populated.
- 18 source headers and 9 doctest TUs under `include/tensor/` and `tests/`.
- One Jupyter notebook (`tutorials/00_intro.ipynb`).
- CI matrix: 8 jobs ({ubuntu-22.04, macos-14, windows-2022} × {GCC 11 / Clang 13 / AppleClang / MSVC 19.30} × {Debug, Release}). Green on all eight at Phase 1 close.

## Phase 2 entry conditions

- Phase 2 plan landed: [`../impl-plans/2026-05-11_phase-2-autograd.md`](../impl-plans/2026-05-11_phase-2-autograd.md).
- Phase 2 P2.M1 already shipped via PR #10 alongside CI green-up. The team enters Phase 2 with the autograd MVP, a clear roadmap of activations / broadcast-backward / contraction / two tutorials, and a green CI baseline.

## References

- Phase 1 implementation plan (predecessor, closed): [`../impl-plans/2026-05-10_revival-phase-1.md`](../impl-plans/2026-05-10_revival-phase-1.md)
- Phase 2 implementation plan (successor): [`../impl-plans/2026-05-11_phase-2-autograd.md`](../impl-plans/2026-05-11_phase-2-autograd.md)
- ADR set: [`../arc42/09-decisions/`](../arc42/09-decisions/)
- C++ tensor library landscape (Phase 0 research): [`./2026-05-10_tensor-revival-landscape.md`](./2026-05-10_tensor-revival-landscape.md)
