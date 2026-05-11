# Changelog

Format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/). Versions are *logical* alpha snapshots corresponding to phase closures from the [impl-plans](./docs/impl-plans/); concrete git tags will be cut at maintainer discretion. The first published tag will be `0.1.0` at Phase 4 close.

The sequence below reads bottom-up if you want the project's narrative arc; top-down if you want what shipped most recently.

---

## [Unreleased]

### Added

- Phase 1.5 mop-up completed: `Variable::zero_grad()` + `sgd_update()` helpers (PR #26), mdspan polyfill restore via macro-based namespace detection (PR #30), LyX export module with CLI translator + LyX export-converter plugin (PR #31), `TypedTensor<T, "i", "j", …>` compile-time-labelled wrapper (PR #24), `tensor::tex::Evaluator<T>` end-to-end (PR #25), bench framework with reference baseline (PR #33), xeus-cling notebook CI workflow (PR #36).
- Phase 2.5 backend port + Eigen adapter shipped: ADR-0011 `KernelBackend` port API (PR #19), `tensor::core::backend::reference::Backend` (PR #20), `tensor::core::backend::eigen::Backend` (PR #21), `tutorials/08_swappable-backends.ipynb` (PR #22).
- Phase 3 design fixed in ADR-0012 (PR #32); Phase 3 impl-plan dated (PR #27).
- Phase 4 prep: Jupyter Book scaffold (`book/` + GitHub Pages deploy workflow) (PR #28).
- Project-sustainability docs: `CONTRIBUTING.md`, `CODE_OF_CONDUCT.md`, and a `which-named-tensor-type` how-to (PR #35).
- ADR-0010 refining ADR-0001 to "educational-first, production-capable" (PR #18).
- Discussion-points report covering seven open axes (PR #34).
- Cumulative documentation audits keeping the durable docs in sync (PR #29).
- Phase 2 + 2.5 retrospective (PR #29) alongside the existing Phase 1 retrospective.

### Notes

- No concrete git tags cut yet; the sequence below is logical (one alpha per phase exit).
- CI status: 8 reference jobs green; Eigen job exists but its CI iteration is pending verification; WebGPU job is planned for Phase 3.

---

## [0.0.3-alpha] — Phase 2.5 logical close (post-PR #22, 2026-05-11)

### Added

- `KernelBackend` port API (ADR-0011) with the 15-method surface in `include/tensor/core/concepts.hpp`.
- `tensor::core::backend::reference::Backend` — always-available default adapter.
- `tensor::core::backend::eigen::Backend` — SIMD element-wise + BLAS-flavoured GEMM for `double`; delegates other types and uncovered cases to the reference adapter.
- CMake cache variable `TENSOR_KERNEL_BACKEND={reference, eigen}` selects the active adapter at configure time.
- `tutorials/08_swappable-backends.ipynb` — Hexagonal payoff demonstration; same Domain code runs on either Backend.

### Changed

- ADR-0001 refined (not superseded) by ADR-0010: positioning is now "educational-first, production-capable via backend adapters".
- `tensor::core::contract()` split into `contract_with_plan(a, b, plan)` + a convenience overload that computes the plan internally.
- `tensor::core::unbroadcast()` moved from `tensor::autograd::detail` to `tensor::core` so the `KernelBackend` port can name it.

---

## [0.0.2-alpha] — Phase 2 close (post-PR #17, 2026-05-11)

### Added

- `tensor::autograd::Variable<T, N>` and `DynamicVariable<T>` with reverse-mode tape.
- Element-wise differentiable operators (`+ - *`) plus reductions (`sum_all`).
- Activation primitives: `exp`, `log`, `relu`, `neg` (with `operator-` unary).
- Broadcast-aware backward via the `unbroadcast` helper.
- Contraction (`dot`) for matvec and matmul; both autograd directions reuse the forward `contract` kernel.
- `gradient_check(f, x)` — finite-difference numerical gradient verification.
- `tutorials/05_autograd-from-scratch.ipynb` walking the autograd implementation primitive-by-primitive.
- `tutorials/07_mlp-on-toy.ipynb` — capstone: 200-epoch SGD training loop on `y = 2x + 1`, converging to W ≈ 2, b ≈ 1.

### Changed

- `Shape<N>::rank` and `Tensor<T, N>::rank` moved from static data members to static methods so a single `ShapeLike` / `TensorLike` concept works for both static- and runtime-rank shapes.

---

## [0.0.1-alpha] — Phase 1 close (post-PR #8, 2026-05-10)

### Added

- pentaglyph-docs scaffold (arc42 + C4 + MADR + Diátaxis + TiSDD); nine foundational ADRs at this milestone (ADR-0001..0009).
- Build system: CMake ≥ 3.25 + vcpkg manifest + CMakePresets; 8-job CI matrix (Ubuntu / macOS / Windows × GCC / Clang / AppleClang / MSVC × Debug / Release).
- Core types: `Axis`, `Shape<N>`, `Tensor<T, N>`, `DynamicShape`, `DynamicTensor<T>`, `mdview()`, `from_mdspan()`, `operator<<` printing in the 2016 README's ASCII style.
- Runtime broadcast: `broadcast_shapes()`, `BroadcastPlan`, element-wise `+ - * /` reproducing the 2016 README's 5×5 tables byte-for-byte.
- `tensor::tex` parser: `Expression`, `parse(string_view)`, `to_latex(Expression)`, `_tex` UDL — round-trip property tested on a 10-expression corpus.
- Modernised function tensor and reference tensor: `a * f = (1, 4, 7, 10, 13)`, `r * 3 = (9, 27, 81, 243, 729)`.
- `tutorials/00_intro.ipynb` — modernised 2016 Qiita walkthrough.

### Removed

- Eclipse CDT files archived to `archive/eclipse-cdt/`; 2016 implementation archived to `archive/legacy-2016/`.

### Fixed

- CI green-up: vcpkg manifest baseline, full-clone vcpkg bootstrap, `-Wno-unused-result` on tests, `mdspan` polyfill deferred (later restored in PR #30).

---

## Tagging policy

- `0.1.0` will be the first published GitHub release. Phase 4 close.
- Alphas (`0.0.1-alpha` ... `0.0.4-alpha`) are *narrative* phase exits; no tag has been cut for them.
- Post-`0.1.0` versioning follows semver as described in `.claude/rules/version-control.md`.
