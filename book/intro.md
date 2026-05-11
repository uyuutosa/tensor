# `tensor` — named-axis tensor algebra in modern C++

> Educational-first, production-capable via backend adapters.

`tensor` is a header-only C++20/23 library that explores **named-axis tensor algebra** — tensors carry axis labels (`a_i`, `b_j`), and operations broadcast over them in Einstein style (`a_i + b_j → c_{ij}`). The project's slogan is *the formula is the program*: the same expression you'd write in a paper can be parsed, evaluated, and gradient-checked in a notebook.

## What this book covers

This Jupyter Book renders the notebooks shipped under [`tutorials/`](https://github.com/uyuutosa/tensor/tree/develop/tutorials) so a reader can follow the project end-to-end without leaving the browser.

- **Getting started** — `00_intro` walks through the 2016 README's named-axis examples on the new API.
- **Autograd** — `05_autograd-from-scratch` builds the tape-based reverse-mode autograd primitive by primitive; `07_mlp-on-toy` trains a small model end-to-end.
- **Architecture** — `08_swappable-backends` shows the Hexagonal "lite" payoff: same Domain code on reference, Eigen, or (Phase 3) WebGPU.

## When you want production speed

The Domain is intentionally small and readable. Speed for production-shaped workloads comes from swappable `KernelBackend` adapters (reference, Eigen, WebGPU). Configure with:

```bash
cmake --preset=default -DTENSOR_KERNEL_BACKEND=eigen
```

Production users adopt the library **as-is**: no ABI stability guarantee, no commitment to operator-coverage parity with mature libraries (Eigen / xtensor / libtorch / Kokkos / `std::linalg`), no formal support. See [ADR-0001](https://github.com/uyuutosa/tensor/blob/develop/docs/arc42/09-decisions/0001-pivot-to-educational-named-axis-dsl.md) and [ADR-0010](https://github.com/uyuutosa/tensor/blob/develop/docs/arc42/09-decisions/0010-refine-positioning-to-educational-first-production-capable.md) for the positioning, and [`docs/`](https://github.com/uyuutosa/tensor/tree/develop/docs) for the full architecture surface.

## Reading this book

Each chapter is a Jupyter notebook. The C++ code blocks are written to be executable in the [xeus-cling](https://github.com/jupyter-xeus/xeus-cling) kernel. The static site you're reading shows pre-rendered outputs; clone the repo and run `jupyter lab` to interact with them yourself.

## Project status

| Phase | Status |
| ----- | ------ |
| Phase 1 — `0.0.1-alpha` foundation | ✅ shipped |
| Phase 2 — autograd | ✅ shipped (MVP → activations → broadcast backward → contraction → MLP-on-toy) |
| Phase 2.5 — backend adapters | ✅ shipped (`KernelBackend` port; reference + Eigen adapters; tutorial 08) |
| Phase 1.5 mop-up | partial — `LabelTag` + `TypedTensor`, `_tex` evaluator, `zero_grad` + `sgd_update` ✅; mdspan polyfill restore, xeus-cling notebook CI, LyX export backlog |
| Phase 3 — WebGPU adapter | planned (impl-plan dated 2026-05-11) |
| Phase 4 — `0.1.0` public release | in progress (this book scaffold + GitHub Pages deploy workflow ✅; release tag and full tutorial corpus pending) |

For the detailed roadmap, see the [Phase 3 plan](https://github.com/uyuutosa/tensor/blob/develop/docs/impl-plans/2026-05-11_phase-3-webgpu.md) and the [Phase 1 retrospective](https://github.com/uyuutosa/tensor/blob/develop/docs/reports/2026-05-11_phase-1-retrospective.md).
