# Roadmap

A dated, append-only sequence of phases. Each phase has an [`impl-plan`](https://github.com/uyuutosa/tensor/tree/develop/docs/impl-plans) document; closed phases produce a retrospective [`report`](https://github.com/uyuutosa/tensor/tree/develop/docs/reports).

## Shipped

| Phase | Exit version | Description |
| ----- | ------------ | ----------- |
| Phase 1 | `0.0.1-alpha` | Build system reset (CMake + vcpkg + CI), core types (Axis, Shape, Tensor), `_tex` parser MVP, function/reference tensors, intro notebook. |
| Phase 2 | `0.0.2-alpha` | Autograd MVP, activations, broadcast-aware backward, contraction (matmul), autograd-from-scratch and MLP-on-toy notebooks. |
| Phase 2.5 | `0.0.3-alpha` | ADR-0010 / ADR-0011: `KernelBackend` port + reference and Eigen adapters; swappable-backends notebook. |

## In flight

| Phase | Status |
| ----- | ------ |
| Phase 1.5 mop-up | LabelTag + TypedTensor (✅), `_tex` evaluator (✅), `zero_grad` + `sgd_update` (✅); mdspan polyfill restore and xeus-cling notebook CI still backlog. |
| Phase 3 — WebGPU adapter | impl-plan dated 2026-05-11; six milestones (ADR-0012, stub adapter, element-wise WGSL kernels, GEMM kernel, broadcast/reduce kernels, tutorial 06). |
| Phase 4 — `0.1.0` public release | This book is the first deliverable; LyX export module and release tag still pending. |

## Planned

- **Phase 4 close** — final `0.1.0` tag with GitHub Release, full tutorial corpus, this book deployed on GitHub Pages.
- **LyX export module** — `lyx-export/` directory with a LyX Module file that exports tensor-math insets to the `_tex` UDL DSL.

## How to follow along

Each impl-plan and ADR lives in the repo under [`docs/`](https://github.com/uyuutosa/tensor/tree/develop/docs). The narrative around the roadmap (motivation, deferred items, what was harder than expected) is captured in the [Phase 1 retrospective](https://github.com/uyuutosa/tensor/blob/develop/docs/reports/2026-05-11_phase-1-retrospective.md).
