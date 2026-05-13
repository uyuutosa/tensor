# Roadmap

A dated, append-only sequence of phases. Each phase has an [`impl-plan`](https://github.com/uyuutosa/tensor/tree/develop/docs/impl-plans) document; closed phases produce a retrospective [`report`](https://github.com/uyuutosa/tensor/tree/develop/docs/reports).

## Shipped

| Phase | Exit version | Description |
| ----- | ------------ | ----------- |
| Phase 1 | `0.0.1-alpha` | Build system reset (CMake + vcpkg + CI), core types (Axis, Shape, Tensor), `_tex` parser MVP, function/reference tensors, intro notebook. |
| Phase 1.5 mop-up | — | `LabelTag` + `TypedTensor` compile-time-label path, `_tex` Evaluator end-to-end, `zero_grad` + `sgd_update`, mdspan polyfill restore, LyX export module + LyX plugin, bench framework + reference baseline, xeus-cling notebook CI workflow, LyX export golden-file CI smoke. |
| Phase 2 | `0.0.2-alpha` | Autograd MVP, activations, broadcast-aware backward, contraction (matmul), autograd-from-scratch and MLP-on-toy notebooks. |
| Phase 2.5 | `0.0.3-alpha` | ADR-0010 / ADR-0011: `KernelBackend` port + reference and Eigen adapters; swappable-backends notebook; perf report committed (reference baseline + Eigen + WebGPU measured 2026-05-12). |
| Phase 3 — WebGPU adapter | — | All 6 milestones shipped: ADR-0012 design (P3.M1), stub adapter (P3.M2), element-wise WGSL + dispatch (P3.M3.1 / .3 sources + P3.M3.2 wiring), tiled GEMM WGSL + dispatch (P3.M4.1 / .2), broadcast WGSL + dispatch (P3.M5), tutorial 06 design walkthrough (P3.M6). **12 of 15 `KernelBackend` methods dispatch real Dawn compute on `float`**; 3 (`reduce_sum`, `unbroadcast`, non-simple-GEMM `contract`) delegate to reference, matching the Eigen adapter's scope. Verified on RTX 3090 + Dawn `20260410.140140` + Vulkan. |

## In flight

| Phase | Status |
| ----- | ------ |
| **Phase 4 — `0.1.0` public release** | **Release-ready as of 2026-05-12**. Strict and soft blockers in the Phase 4 release rehearsal report are resolved. Remaining steps are the maintainer's ceremony: release branch from `develop`, version bump in `vcpkg.json` + `CMakeLists.txt` + `CHANGELOG.md`, PR to `main` with merge commit + `0.1.0` tag, back-merge to `develop`, GitHub Pages enable. |

## Planned (post-`0.1.0`)

- **Phase 5 — `tensor::linalg` shim over `kokkos/stdBLAS`** (ADR-0014 §Decision Outcome point 4). Pre-empted: the `__cpp_lib_linalg` feature-test path lets the project switch transparently when `std::linalg` ships in libc++ / libstdc++ / MSVC STL (currently 2028+).
- **Phase 6 — Python SDK** (ADR-0018, brought forward from "あとで" by the maintainer's 2026-05-12 directive). M1–M5 shipped 2026-05-12/13 (`Axis` / `DynamicShape` / `DynamicTensor` + arithmetic + `contract` + NumPy interop + autograd + `tex.Evaluator`). Bundle B + MVG demo polish 2026-05-13 (autograd extensions: `sin` / `cos` / `sqrt` / `__truediv__` / `reduce_along_label`; new tutorial notebooks for multi-focal tensors and perspective bundle adjustment with interactive 3D plotly visualisation; Binder + Colab badges across all five Python notebooks). M6 release-prep 2026-05-13 (cibuildwheel multi-platform matrix + tag-triggered PyPI publish via trusted publishing). **Remaining for `0.2.0`**: maintainer registers the PyPI project + configures the trusted-publisher policy, then cuts the `0.2.0` release branch and tag.
- **Bibliography audit** (ADR-0015 §Compliance): half-yearly, first audit due 2026-11-11. Checks ADR cross-refs resolve, §12 glossary covers every public name, clean clone → build + bench + notebook in under 30 minutes, no user-facing string asserts declarative "is the canonical reference" form.

## How to follow along

- **Per-phase impl-plans + retrospectives**: [`docs/impl-plans/`](https://github.com/uyuutosa/tensor/tree/develop/docs/impl-plans), [`docs/reports/`](https://github.com/uyuutosa/tensor/tree/develop/docs/reports).
- **Live decisions**: [`docs/arc42/09-decisions/`](https://github.com/uyuutosa/tensor/tree/develop/docs/arc42/09-decisions) (17 ADRs as of 2026-05-12; ADR-0013 superseded by ADR-0015; ADR-0014 §Decision Outcome point 2 refined by ADR-0016; ADR-0015 §Compliance bullet 3 refined by ADR-0017).
- **Detailed designs**: [`docs/detailed-design/`](https://github.com/uyuutosa/tensor/tree/develop/docs/detailed-design) — 7 instances covering `tensor::core`, `tensor::autograd`, `tensor::tex`, the WebGPU adapter trio (element-wise / GEMM / broadcast), and the `KernelBackend` port surface.
- **Discussion-points report**: [`docs/reports/2026-05-11_open-discussion-points.md`](https://github.com/uyuutosa/tensor/blob/develop/docs/reports/2026-05-11_open-discussion-points.md) — eight axes of "next decisions" with maintainer recommendations and status updates.
