# tensor

> **Header-only C++20/23 library — the canonical reference for differentiable named-axis tensor computation in modern C++.**
> Educational-first, production-capable via backend adapters ([ADR-0013](./docs/arc42/09-decisions/0013-reframe-as-canonical-reference-for-named-tensor-computation.md), refining [ADR-0010](./docs/arc42/09-decisions/0010-refine-positioning-to-educational-first-production-capable.md)). Production users adopt **as-is**: no ABI stability guarantee, no commitment to operator-coverage parity with [Eigen](https://eigen.tuxfamily.org/) / [xtensor](https://github.com/xtensor-stack/xtensor) / [libtorch](https://pytorch.org/) / [Kokkos](https://kokkos.org/), no formal support. The Domain is intentionally small and readable; speed comes from swappable `KernelBackend` adapters.

`tensor` revives a 2016 personal C++ template library around an unusual idea: tensors that carry **axis labels** (`a_i`, `b_j`) and operate on each other through Einstein-style broadcasting (`a_i + b_j → c_{ij}`). The 2026 rewrite ships:

- **`_tex` user-defined literal** — write tensor expressions as LaTeX (`R"(c_{ij} = a_i b_j)"_tex`); the slogan is *the formula is the program*. (Phase 1 MVP shipped.)
- **Hybrid named-axis API** — runtime axis labels by default, NTTP compile-time labels via the `_ax` UDL when the user wants the compiler to catch mismatches. (Runtime path shipped; compile-time path on the Phase 1.5 follow-up backlog.)
- **Tape-based reverse-mode autograd** typed against named-axis tensors: element-wise + - *, activations (exp / log / relu / neg), broadcast-aware backward, named-axis contraction (`dot`), finite-difference gradient checker. (Phase 2 shipped.)
- **Swappable kernel backends** via the `KernelBackend` port (ADR-0009 + ADR-0010): three concrete adapter slots — reference CPU ✅, Eigen ✅ (Phase 2.5), WebGPU **stub** ✅ (Phase 3 P3.M2 — concept-satisfying skeleton delegating to reference; WGSL kernels land in P3.M3+ per [ADR-0006](./docs/arc42/09-decisions/0006-adopt-webgpu-as-gpu-backend.md) / [ADR-0012](./docs/arc42/09-decisions/0012-webgpu-adapter-implementation-design.md)). Domain code is backend-agnostic; swapping is one CMake variable.
- **Bundled xeus-cling Jupyter tutorials** — `00_intro`, `05_autograd-from-scratch`, `07_mlp-on-toy` shipped; backend-swap and WebGPU notebooks planned.

See [`docs/arc42/01-introduction-and-goals/overview.md`](./docs/arc42/01-introduction-and-goals/overview.md) for goals and [`docs/arc42/09-decisions/`](./docs/arc42/09-decisions/) for the fourteen foundational ADRs (ADR-0013 sharpens positioning to canonical-reference; ADR-0014 captures the external-substrate strategy derived from the [2026-05-11 research](./docs/reports/2026-05-11_external-substrate-research.md)).

The previous 2016 implementation has been retired to [`archive/legacy-2016/`](./archive/legacy-2016/) for reference; the rewrite under [`include/tensor/`](./include/tensor/) is *not* a port — see [ADR-0001](./docs/arc42/09-decisions/0001-pivot-to-educational-named-axis-dsl.md) and [ADR-0002](./docs/arc42/09-decisions/0002-rewrite-on-cpp20-baseline-with-mdspan-interop.md).

## Quickstart

Requires CMake ≥ 3.25, a C++20 compiler (GCC ≥ 11 / Clang ≥ 13 / MSVC 19.30+), and [vcpkg](https://vcpkg.io) (set `VCPKG_ROOT` to your clone).

```bash
git clone https://github.com/uyuutosa/tensor.git
cd tensor
cmake --preset=default
cmake --build --preset=default
ctest --preset=default
```

That builds the header-only library, fetches dependencies (`mdspan`, `fmt`, `doctest`) via vcpkg, and runs the test suite. Optional: `-DTENSOR_KERNEL_BACKEND=eigen` for Eigen-backed kernels; `-DTENSOR_BUILD_BENCH=ON` to build the cross-backend performance bench (see [`docs/reports/2026-05-11_backend-performance-comparison.md`](./docs/reports/2026-05-11_backend-performance-comparison.md)).

## Status

| Phase                                 | Milestones                                                            | State          |
| ------------------------------------- | --------------------------------------------------------------------- | -------------- |
| **Phase 1 — `0.0.1-alpha`**           | M1 build · M2 core · M3 runtime broadcast · M4 `_tex` MVP · M5 fn/ref · M6 intro notebook · CI green | ✅ shipped     |
| **Phase 1.5 — mop-up**                | M3 compile-time path (`LabelTag` + `TypedTensor`) ✅ · M4 `_tex` evaluator ✅ · `zero_grad` + `sgd_update` ✅ · mdspan polyfill restore ✅ · LyX export module + LyX plugin ✅ · perf bench framework + reference baseline ✅ · xeus-cling notebook CI backlog | mostly ✅ |
| **Phase 2 — autograd (ADR-0007)**     | MVP + activations + broadcast-aware backward + contraction + autograd-from-scratch & MLP-on-toy notebooks | ✅ shipped     |
| **Phase 2.5 — `KernelBackend` port + Eigen adapter** ([ADR-0010](./docs/arc42/09-decisions/0010-refine-positioning-to-educational-first-production-capable.md)) | ADR-0011 port API ✅ · reference adapter ✅ · Eigen adapter ✅ · tutorial 08 ✅ · perf comparison report | ✅ shipped (perf report still backlog) |
| **Phase 3 — WebGPU (ADR-0006)**       | ADR-0012 design ✅ · `tensor::core::backend::webgpu` stub adapter ✅ (P3.M2; delegates to reference) · WGSL element-wise kernels (P3.M3) · GEMM kernels (P3.M4) · reduce + broadcast (P3.M5) · browser demo | P3.M1/M2 ✅, P3.M3 onward planned |
| **Phase 4 — `0.1.0` public release**  | Full tutorial corpus · Jupyter Book site (scaffold ✅) · LyX export · `CONTRIBUTING.md` + `CODE_OF_CONDUCT.md` ✅ · `CHANGELOG.md` ✅ · release tag | in progress    |

Live plan: see [`docs/impl-plans/`](./docs/impl-plans/) (one dated file per phase).

## Documentation

The bundled Jupyter Book is published from [`book/`](./book/) to GitHub Pages on every push to `develop` (see [`.github/workflows/deploy-book.yml`](./.github/workflows/deploy-book.yml)). Enable Pages in the repository settings (Source: *GitHub Actions*) the first time you want it live.

## Contributing

See [`CONTRIBUTING.md`](./CONTRIBUTING.md) for the contributor workflow (Git Flow, Conventional Commits, Hexagonal discipline checklist) and [`CODE_OF_CONDUCT.md`](./CODE_OF_CONDUCT.md) for project norms.

## What changed when

See [`CHANGELOG.md`](./CHANGELOG.md) for a phase-by-phase rundown of what landed in each logical alpha.

## Architecture docs

This repository ships with the [pentaglyph-docs](https://github.com/uyuutosa/pentaglyph-docs)
scaffold (arc42 + C4 + MADR + Diátaxis + TiSDD bound by one workflow).

- Documentation root: [`docs/`](./docs/)
- Entry point for humans: [`docs/INDEX.md`](./docs/INDEX.md)
- Entry point for AI agents: [`docs/AI_INSTRUCTIONS.md`](./docs/AI_INSTRUCTIONS.md)
- "What to write where": [`docs/WORKFLOW.md`](./docs/WORKFLOW.md)
- Upstream kit (vendored as a git subtree): [`libs/pentaglyph-docs/`](./libs/pentaglyph-docs/)

To pull future upstream updates:

```bash
git subtree pull --prefix=libs/pentaglyph-docs \
  https://github.com/uyuutosa/pentaglyph-docs.git main --squash
```

## Citing

This project is positioned as the canonical reference for differentiable named-axis tensor computation in modern C++ ([ADR-0013](./docs/arc42/09-decisions/0013-reframe-as-canonical-reference-for-named-tensor-computation.md)). If you build on it, please cite via [`CITATION.cff`](./CITATION.cff). The ADR sequence under [`docs/arc42/09-decisions/`](./docs/arc42/09-decisions/) is the project's bibliography — individual decisions can be cited as well-defined design choices with rationale.

## License

MIT.
