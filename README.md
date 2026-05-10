# tensor

> **Header-only C++20/23 library for named-axis tensor algebra — educational-first, production-capable via backend adapters.**
> Production users adopt **as-is**: no ABI stability guarantee, no commitment to operator-coverage parity with [Eigen](https://eigen.tuxfamily.org/) / [xtensor](https://github.com/xtensor-stack/xtensor) / [libtorch](https://pytorch.org/) / [Kokkos](https://kokkos.org/), no formal support. The Domain is intentionally small and readable; speed comes from swappable `KernelBackend` adapters ([ADR-0010](./docs/arc42/09-decisions/0010-refine-positioning-to-educational-first-production-capable.md)).

`tensor` revives a 2016 personal C++ template library around an unusual idea: tensors that carry **axis labels** (`a_i`, `b_j`) and operate on each other through Einstein-style broadcasting (`a_i + b_j → c_{ij}`). The 2026 rewrite ships:

- **`_tex` user-defined literal** — write tensor expressions as LaTeX (`R"(c_{ij} = a_i b_j)"_tex`); the slogan is *the formula is the program*. (Phase 1 MVP shipped.)
- **Hybrid named-axis API** — runtime axis labels by default, NTTP compile-time labels via the `_ax` UDL when the user wants the compiler to catch mismatches. (Runtime path shipped; compile-time path on the Phase 1.5 follow-up backlog.)
- **Tape-based reverse-mode autograd** typed against named-axis tensors: element-wise + - *, activations (exp / log / relu / neg), broadcast-aware backward, named-axis contraction (`dot`), finite-difference gradient checker. (Phase 2 shipped.)
- **Swappable kernel backends** via the `KernelBackend` port (ADR-0009 + ADR-0010): reference CPU shipped; Eigen adapter in Phase 2.5; WebGPU adapter in Phase 3 ([ADR-0006](./docs/arc42/09-decisions/0006-adopt-webgpu-as-gpu-backend.md)). Domain code is backend-agnostic.
- **Bundled xeus-cling Jupyter tutorials** — `00_intro`, `05_autograd-from-scratch`, `07_mlp-on-toy` shipped; backend-swap and WebGPU notebooks planned.

See [`docs/arc42/01-introduction-and-goals/overview.md`](./docs/arc42/01-introduction-and-goals/overview.md) for goals and [`docs/arc42/09-decisions/`](./docs/arc42/09-decisions/) for the ten foundational ADRs.

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

That builds the header-only library, fetches dependencies (`mdspan`, `fmt`, `doctest`) via vcpkg, and runs the test suite.

## Status

| Phase                                 | Milestones                                                            | State          |
| ------------------------------------- | --------------------------------------------------------------------- | -------------- |
| **Phase 1 — `0.0.1-alpha`**           | M1 build · M2 core · M3 runtime broadcast · M4 `_tex` MVP · M5 fn/ref · M6 intro notebook · CI green | ✅ shipped     |
| **Phase 1.5 — mop-up**                | M3 compile-time path (NTTP `LabelTag`) · M4 evaluator bridge · mdspan polyfill `Kokkos::` adapter · xeus-cling notebook CI | ⏳ backlog     |
| **Phase 2 — autograd (ADR-0007)**     | MVP + activations + broadcast-aware backward + contraction + autograd-from-scratch & MLP-on-toy notebooks | ✅ shipped     |
| **Phase 2.5 — `KernelBackend` port + Eigen adapter** ([ADR-0010](./docs/arc42/09-decisions/0010-refine-positioning-to-educational-first-production-capable.md)) | ADR-0011 port API · refactor reference adapter · Eigen adapter · perf comparison · tutorial 08 | 🚧 in flight  |
| **Phase 3 — WebGPU (ADR-0006)**       | `tensor::gpu` DrivenAdapter · WGSL codegen · browser demo             | planned        |
| **Phase 4 — `0.1.0` public release**  | Full tutorial corpus · Jupyter Book site · LyX export                 | planned        |

Live plan: see [`docs/impl-plans/`](./docs/impl-plans/) (one dated file per phase).

## Documentation

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

## License

MIT.
