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
| Phase 4 — `0.1.0` public release | `0.1.0` | Maintainer release ceremony: release branch from `develop`, version bumps, PR #91 to `main` with `0.1.0` tag, back-merge PR #92 to `develop`, GitHub Pages enable. Strict + soft blockers in the rehearsal report all resolved before cut. |
| Phase 6 — Python SDK | — *(pending `0.2.0` tag)* | All six milestones M1–M6 + three unplanned bundles shipped 2026-05-12/13 across 17 PRs (`#95`–`#113`). `tensor.DynamicTensor` + `DynamicTensorF32` + arithmetic with Einstein-style broadcast; `contract` + NumPy interop; `tensor.autograd` (`DynamicVariable`, ops, `dot`, `sum_all`, `backward`, `sgd_update` + Bundle B: `sin` / `cos` / `sqrt` / `__truediv__` / `reduce_along_label`); `tensor.tex` (`parse`, `to_latex`, `Evaluator`). Five Python notebooks (`00`–`04`) including paper-style multi-focal tensors + perspective bundle adjustment with interactive 3D plotly. HuggingFace Space scaffold + `deploy.sh`. M6 release-prep: cibuildwheel multi-platform matrix + tag-triggered PyPI publish via PyPA trusted publishing (OIDC). Retrospective: [`docs/reports/2026-05-13_phase-6-python-sdk-retrospective.md`](https://github.com/uyuutosa/tensor/blob/develop/docs/reports/2026-05-13_phase-6-python-sdk-retrospective.md). **`0.2.0` tag pending** maintainer's PyPI project register + trusted-publisher policy. |

## Planned (post-`0.1.0`)

- **Phase 5 — `tensor::linalg` shim over `kokkos/stdBLAS`** (ADR-0014 §Decision Outcome point 4). Pre-empted: the `__cpp_lib_linalg` feature-test path lets the project switch transparently when `std::linalg` ships in libc++ / libstdc++ / MSVC STL (currently 2028+).
- **Phase 6.5 — runtime `set_backend()` via PEP-508 extras** ([ADR-0019](https://github.com/uyuutosa/tensor/blob/develop/docs/arc42/09-decisions/0019-phase-6-5-runtime-backend-selection-via-extras.md), [impl-plan](https://github.com/uyuutosa/tensor/blob/develop/docs/impl-plans/2026-05-13_phase-6-5-set-backend.md)). `pip install tensor-named-axis[eigen]` / `[webgpu]` / `[all]` brings the matching `KernelBackend` adapter; `tensor.set_backend("reference" | "eigen" | "webgpu")` switches at runtime between *installed* backends. Default reference-only install stays at ~5 MB (educational path); WebGPU's ~50 MB Dawn runtime is opt-in. Four milestones P6.5.M1–P6.5.M4, target window 2026-05-14 → 2026-06-15, exit on `0.3.0` tag with three PyPI distributions.
- **Bibliography audit** (ADR-0015 §Compliance): half-yearly, first audit due 2026-11-11. Checks ADR cross-refs resolve, §12 glossary covers every public name, clean clone → build + bench + notebook in under 30 minutes, no user-facing string asserts declarative "is the canonical reference" form.

## How to follow along

- **Per-phase impl-plans + retrospectives**: [`docs/impl-plans/`](https://github.com/uyuutosa/tensor/tree/develop/docs/impl-plans), [`docs/reports/`](https://github.com/uyuutosa/tensor/tree/develop/docs/reports).
- **Live decisions**: [`docs/arc42/09-decisions/`](https://github.com/uyuutosa/tensor/tree/develop/docs/arc42/09-decisions) (19 ADRs as of 2026-05-13; ADR-0013 superseded by ADR-0015; ADR-0014 §Decision Outcome point 2 refined by ADR-0016; ADR-0015 §Compliance bullet 3 refined by ADR-0017; ADR-0018 anchors Phase 6; ADR-0019 anchors Phase 6.5).
- **Detailed designs**: [`docs/detailed-design/`](https://github.com/uyuutosa/tensor/tree/develop/docs/detailed-design) — 7 instances covering `tensor::core`, `tensor::autograd`, `tensor::tex`, the WebGPU adapter trio (element-wise / GEMM / broadcast), and the `KernelBackend` port surface.
- **Discussion-points report**: [`docs/reports/2026-05-11_open-discussion-points.md`](https://github.com/uyuutosa/tensor/blob/develop/docs/reports/2026-05-11_open-discussion-points.md) — eight axes of "next decisions" with maintainer recommendations and status updates.
