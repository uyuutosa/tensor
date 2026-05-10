# tensor

> **Header-only C++20/23 educational library for named-axis tensor algebra.**
> *A teaching artifact, not a production library.* For production tensor / linear algebra needs use [Eigen](https://eigen.tuxfamily.org/), [xtensor](https://github.com/xtensor-stack/xtensor), [libtorch](https://pytorch.org/), [Kokkos](https://kokkos.org/), or the upcoming `std::mdspan` + `std::linalg`.

`tensor` revives a 2016 personal C++ template library around an unusual idea: tensors that carry **axis labels** (`a_i`, `b_j`) and operate on each other through Einstein-style broadcasting (`a_i + b_j → c_{ij}`). The 2026 rewrite, currently in progress, also ships:

- **`_tex` user-defined literal** — write tensor expressions as LaTeX (`R"(c_{ij} = a_i b_j)"_tex`); the slogan is *the formula is the program*.
- **Hybrid named-axis API** — runtime axis labels by default, NTTP compile-time labels via the `_ax` UDL when the user wants the compiler to catch mismatches.
- **Tape-based reverse-mode autograd** (planned) typed against named-axis tensors.
- **WebGPU backend** (planned) via [gpu.cpp](https://github.com/AnswerDotAI/gpu.cpp) / Dawn / wgpu-native — zero proprietary GPU toolchain install.
- **Bundled xeus-cling Jupyter tutorials** — the educational deliverable, served as a Jupyter Book site.

See [`docs/arc42/01-introduction-and-goals/overview.md`](./docs/arc42/01-introduction-and-goals/overview.md) for goals and [`docs/arc42/09-decisions/`](./docs/arc42/09-decisions/) for the eight foundational ADRs.

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

`0.0.1-alpha` (in progress). See the active [Phase 1 implementation plan](./docs/impl-plans/2026-05-10_revival-phase-1.md) for the milestone breakdown; M1 (build system reset) is closing now.

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
