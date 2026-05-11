# Changelog

Format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/). Versions are *logical* alpha snapshots corresponding to phase closures from the [impl-plans](./docs/impl-plans/); concrete git tags will be cut at maintainer discretion. The first published tag will be `0.1.0` at Phase 4 close.

The sequence below reads bottom-up if you want the project's narrative arc; top-down if you want what shipped most recently.

---

## [Unreleased]

### Added

- **Canonical-reference reframing** in [ADR-0013](./docs/arc42/09-decisions/0013-reframe-as-canonical-reference-for-named-tensor-computation.md): the project's positioning is sharpened from "educational-first, production-capable" to "the canonical reference for differentiable named-tensor computation in modern C++ — educational-first, production-capable via backend adapters". Refines (does not supersede) ADR-0010.
- **External-substrate strategy** in [ADR-0014](./docs/arc42/09-decisions/0014-external-substrate-strategy.md): four coupled tactical decisions — Dawn via vcpkg (the P3.M3 unblock), gpu.cpp vendored under `third_party/gpu_cpp/`, notebook execution migrated to xeus-cpp 0.10.0, and a `tensor::linalg` shim namespace over kokkos/stdBLAS for the eventual `std::linalg` (P1673) standard library. Refines operational details of ADR-0006 / ADR-0008 / ADR-0012.
- **External-substrate research report** ([`docs/reports/2026-05-11_external-substrate-research.md`](./docs/reports/2026-05-11_external-substrate-research.md)): Layer B audit of gpu.cpp / vcpkg port status / xeus-cling / `std::linalg` (P1673) as of 2026-05-11. Triggered ADR-0013 + ADR-0014.
- Phase 3 **P3.M4.1 — Tiled GEMM WGSL kernel source** (`include/tensor/core/backend/webgpu_wgsl.hpp::kGemmF32`): one readable tiled-GEMM kernel covering matvec and matmul with one shared axis. 16 × 16 × 16 tile structure; workgroup-shared `shA`/`shB`; uniform `Params` buffer carries M/N/K. `docs/detailed-design/webgpu-gemm-kernel.md` is the design. Text-validated; inert until P3.M4.2 wires the dispatch.
- arc42 §1 (Introduction and Goals) reframed for the canonical-reference identity per ADR-0013, with G-8 (citability discipline) added and §6 success criteria refreshed to reflect the PR #1–#45 shipped state.
- Phase 3 **P3.M3.3 — Unary WGSL kernel sources**: four more `constexpr std::string_view` WGSL kernels (`kExpF32` / `kLogF32` / `kReluF32` / `kNegF32`) appended to `include/tensor/core/backend/webgpu_wgsl.hpp`. Two-binding template (input + output) instead of the binary three. ReLU expressed via `max(a, 0.0)` (single GPU instruction on Vulkan / Metal / D3D12). The element-wise *source* surface is now feature-complete: 8 ops covered (4 binary + 4 unary).
- Phase 3 **P3.M3.1 — WGSL element-wise kernel sources** (`include/tensor/core/backend/webgpu_wgsl.hpp`): four `constexpr std::string_view` WGSL kernels (`kAddF32` / `kSubF32` / `kMulF32` / `kDivF32`) in gpu.cpp's `{{workgroupSize}}` / `{{precision}}` templated form. `tests/test_webgpu_wgsl.cpp` text-validates them. Inert source code until P3.M3.2 wires the dispatch — see [`docs/detailed-design/webgpu-element-wise-kernels.md`](./docs/detailed-design/webgpu-element-wise-kernels.md) for the design.
- **Detailed-design doc for the WebGPU element-wise kernels** ([`docs/detailed-design/webgpu-element-wise-kernels.md`](./docs/detailed-design/webgpu-element-wise-kernels.md)) — first detailed-design instance under arc42 §5; specifies the gpu.cpp dispatch sequence to land in P3.M3.2.
- `tools/check-vendored.sh` + CI `vendored-check` job enforcing ADR-0014 (PR #41).
- `third_party/gpu_cpp/` — gpu.cpp@0.2.0 vendored under Apache-2.0 (PR #41).
- Notebook CI primary kernel migrated to **xeus-cpp 0.10.0** (PR #42), with a legacy xeus-cling smoke path retained for `00_intro.ipynb`.
- `CITATION.cff` at the repo root + "How to cite this work" sections (PR #40), honoring ADR-0013's bibliography discipline.
- Phase 3 **P3.M2 — WebGPU stub adapter** (PR #38): `tensor::core::backend::webgpu::Backend` satisfies the `KernelBackend` concept and delegates every method to a private `reference::Backend` instance. Selecting `-DTENSOR_KERNEL_BACKEND=webgpu` defines `TENSOR_HAS_WEBGPU` and routes through the third slot while running on CPU underneath. P3.M3+ progressively replaces method bodies with WGSL kernels dispatched via gpu.cpp + Dawn per [ADR-0012](./docs/arc42/09-decisions/0012-webgpu-adapter-implementation-design.md).
- `CHANGELOG.md` itself in Keep a Changelog format with three logical alphas (PR #37).
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
- CI status: 8 reference jobs green; Eigen job + WebGPU-stub compile-only job exist and run on `develop` / PR. Real GPU numerical agreement is gated by a self-hosted GPU runner that does not exist yet (deferred to P3.M3 entry per ADR-0012).

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
