# tensor

> **Header-only C++20/23 library for differentiable named-axis tensor algebra — educational-first, production-capable via backend adapters. Documented and designed to canonical-reference quality.**
> Three disciplines per [ADR-0015](./docs/arc42/09-decisions/0015-aspire-to-canonical-reference-quality-not-self-anoint.md) (superseding [ADR-0013](./docs/arc42/09-decisions/0013-reframe-as-canonical-reference-for-named-tensor-computation.md)): **bibliography** (every public name traces to a paper or ADR), **ubiquitous language** (one name per concept across code / ADR / glossary), **reproducibility** (clean clone → build + bench + notebook in under 30 minutes). Production users adopt **as-is**: no ABI stability guarantee, no commitment to operator-coverage parity with [Eigen](https://eigen.tuxfamily.org/) / [xtensor](https://github.com/xtensor-stack/xtensor) / [libtorch](https://pytorch.org/) / [Kokkos](https://kokkos.org/), no formal support. The Domain is intentionally small and readable; speed comes from swappable `KernelBackend` adapters.

`tensor` revives a 2016 personal C++ template library around an unusual idea: tensors that carry **axis labels** (`a_i`, `b_j`) and operate on each other through Einstein-style broadcasting (`a_i + b_j → c_{ij}`). The 2026 rewrite ships:

- **`_tex` user-defined literal** — write tensor expressions as LaTeX (`R"(c_{ij} = a_i b_j)"_tex`); the slogan is *the formula is the program*. (Phase 1 MVP shipped.)
- **Hybrid named-axis API** — runtime axis labels by default, NTTP compile-time labels via the `_ax` UDL when the user wants the compiler to catch mismatches. (Runtime path shipped; compile-time path on the Phase 1.5 follow-up backlog.)
- **Tape-based reverse-mode autograd** typed against named-axis tensors: element-wise + - *, activations (exp / log / relu / neg), broadcast-aware backward, named-axis contraction (`dot`), finite-difference gradient checker. (Phase 2 shipped.)
- **Swappable kernel backends** via the `KernelBackend` port (ADR-0009 + ADR-0010 + ADR-0011): three concrete adapters — reference CPU ✅, Eigen ✅ (Phase 2.5), **WebGPU via Dawn ✅** (Phase 3 — 12 of the 15 `KernelBackend` methods dispatch real GPU compute on `float` on RTX 3090 per [ADR-0006](./docs/arc42/09-decisions/0006-adopt-webgpu-as-gpu-backend.md) / [ADR-0012](./docs/arc42/09-decisions/0012-webgpu-adapter-implementation-design.md) / [ADR-0016](./docs/arc42/09-decisions/0016-substrate-refinement-drop-gpu-cpp-talk-to-dawn-directly.md); the remaining three — `reduce_sum`, `unbroadcast`, non-simple-GEMM `contract` — delegate to reference, matching the Eigen adapter's scope). Domain code is backend-agnostic; swapping is one CMake variable.
- **Bundled xeus-cling / xeus-cpp Jupyter tutorials** — `00_intro` (named-axis fundamentals from the 2016 post), `05_autograd-from-scratch` (autograd primitive-by-primitive), `06_webgpu-acceleration` (design walkthrough), `07_mlp-on-toy` (small model trained on toy data), `08_swappable-backends` (Hexagonal payoff: reference vs Eigen) — all shipped.

See [`docs/arc42/01-introduction-and-goals/overview.md`](./docs/arc42/01-introduction-and-goals/overview.md) for goals and [`docs/arc42/09-decisions/`](./docs/arc42/09-decisions/) for the **seventeen** foundational ADRs (ADR-0014 captures the external-substrate strategy from the [2026-05-11 research](./docs/reports/2026-05-11_external-substrate-research.md); ADR-0015 reframes ADR-0013's positioning from "is canonical reference" to "aspires to canonical-reference quality" — the discipline stays, the claim becomes honest; ADR-0016 refines ADR-0014 §Decision Outcome point 2 — drop the gpu.cpp wrapper layer and talk to Dawn directly via `webgpu_cpp.h`; ADR-0017 splits the ADR-0015 §Compliance reproducibility envelope into build+test+bench and notebook audit halves after the [2026-05-12 adversarial self-review](./docs/reports/2026-05-12_landscape-recheck-and-adversarial-review.md) caught that the original "30-minute" wording overpromised on the notebook dimension).

The previous 2016 implementation has been retired to [`archive/legacy-2016/`](./archive/legacy-2016/) for reference; the rewrite under [`include/tensor/`](./include/tensor/) is *not* a port — see [ADR-0001](./docs/arc42/09-decisions/0001-pivot-to-educational-named-axis-dsl.md) and [ADR-0002](./docs/arc42/09-decisions/0002-rewrite-on-cpp20-baseline-with-mdspan-interop.md).

## Try it without installing

All five Python notebooks open in Colab. Each one's first cell installs the SDK from the live repository (`!pip install git+https://github.com/uyuutosa/tensor.git`) on first run — the initial cell takes ~3-5 min while the nanobind extension compiles on the Colab runner; subsequent runs are instant.

| Notebook | What it shows |
| -------- | ------------- |
| [![Colab](https://colab.research.google.com/assets/colab-badge.svg)](https://colab.research.google.com/github/uyuutosa/tensor/blob/main/python/notebooks/00_python-sdk-tour.ipynb) [`00`](./python/notebooks/00_python-sdk-tour.ipynb) | `Axis` / `DynamicShape` / `DynamicTensor` + NumPy interop tour |
| [![Colab](https://colab.research.google.com/assets/colab-badge.svg)](https://colab.research.google.com/github/uyuutosa/tensor/blob/main/python/notebooks/01_python-autograd.ipynb) [`01`](./python/notebooks/01_python-autograd.ipynb) | Tape-based autograd + linear-regression training loop |
| [![Colab](https://colab.research.google.com/assets/colab-badge.svg)](https://colab.research.google.com/github/uyuutosa/tensor/blob/main/python/notebooks/02_python-tex.ipynb) [`02`](./python/notebooks/02_python-tex.ipynb) | `_tex` UDL Python equivalent — `tex.parse` + `Evaluator` |
| [![Colab](https://colab.research.google.com/assets/colab-badge.svg)](https://colab.research.google.com/github/uyuutosa/tensor/blob/main/python/notebooks/03_multifocal-tensors.ipynb) [`03`](./python/notebooks/03_multifocal-tensors.ipynb) | **Bifocal / trifocal / quadrifocal tensors** learned via autograd from a torus-knot point cloud; paper-style with Hartley–Zisserman references |
| [![Colab](https://colab.research.google.com/assets/colab-badge.svg)](https://colab.research.google.com/github/uyuutosa/tensor/blob/main/python/notebooks/04_python-bundle-adjustment-perspective.ipynb) [`04`](./python/notebooks/04_python-bundle-adjustment-perspective.ipynb) | **Perspective bundle adjustment** with `sin` / `cos` rotation + `__truediv__` perspective divide; interactive 3D plotly camera + point-cloud overlay |

Or open the entire corpus in one click with Binder: [![Binder](https://mybinder.org/badge_logo.svg)](https://mybinder.org/v2/gh/uyuutosa/tensor/main?labpath=python%2Fnotebooks) (builds the nanobind extension once at launch, ~5 min).

For a zero-click Gradio demo of the `tensor.tex` evaluator + named-axis broadcast + autograd, see [`huggingface/space/`](./huggingface/space) — the source for the HuggingFace Space deployed at `huggingface.co/spaces/<maintainer>/tensor-named-axis-demo` (link will be added here once the Space is published; the directory is build-ready today).

## Quickstart

Requires CMake ≥ 3.25, a C++20 compiler (GCC ≥ 11 / Clang ≥ 13 / MSVC 19.30+), and [vcpkg](https://vcpkg.io) (set `VCPKG_ROOT` to your clone).

```bash
git clone https://github.com/uyuutosa/tensor.git
cd tensor
cmake --preset=default
cmake --build --preset=default
ctest --preset=default
```

That builds the header-only library, fetches dependencies (`mdspan`, `fmt`, `doctest`) via vcpkg, and runs the test suite. Optional: `-DTENSOR_KERNEL_BACKEND=eigen` for Eigen-backed kernels, `-DTENSOR_KERNEL_BACKEND=webgpu` for the WebGPU-via-Dawn adapter (requires `vcpkg install 'dawn[core,vulkan]'` and a system Vulkan / Metal / D3D12 driver); `-DTENSOR_BUILD_BENCH=ON` to build the cross-backend performance bench (see [`docs/reports/2026-05-11_backend-performance-comparison.md`](./docs/reports/2026-05-11_backend-performance-comparison.md) — the 2026-05-12 subsection has real numbers across all three backends on RTX 3090).

## Status

| Phase                                 | Milestones                                                            | State          |
| ------------------------------------- | --------------------------------------------------------------------- | -------------- |
| **Phase 1 — `0.0.1-alpha`**           | M1 build · M2 core · M3 runtime broadcast · M4 `_tex` MVP · M5 fn/ref · M6 intro notebook · CI green | ✅ shipped     |
| **Phase 1.5 — mop-up**                | M3 compile-time path (`LabelTag` + `TypedTensor`) ✅ · M4 `_tex` evaluator ✅ · `zero_grad` + `sgd_update` ✅ · mdspan polyfill restore ✅ · LyX export module + LyX plugin ✅ · perf bench framework + reference baseline ✅ · xeus-cling notebook CI ✅ · LyX export golden-file CI smoke ✅ | ✅ shipped |
| **Phase 2 — autograd (ADR-0007)**     | MVP + activations + broadcast-aware backward + contraction + autograd-from-scratch & MLP-on-toy notebooks | ✅ shipped     |
| **Phase 2.5 — `KernelBackend` port + Eigen adapter** ([ADR-0010](./docs/arc42/09-decisions/0010-refine-positioning-to-educational-first-production-capable.md)) | ADR-0011 port API ✅ · reference adapter ✅ · Eigen adapter ✅ · tutorial 08 ✅ · perf comparison report (with 2026-05-12 three-backend RTX 3090 measurements) ✅ | ✅ shipped     |
| **Phase 3 — WebGPU (ADR-0006)**       | ADR-0012 design ✅ · ADR-0016 substrate refinement ✅ · `tensor::core::backend::webgpu` adapter ✅ · WGSL element-wise kernels (P3.M3) ✅ · GEMM kernel (P3.M4) ✅ · broadcast kernels (P3.M5) ✅ · tutorial 06 design walkthrough ✅ · **12 of 15 `KernelBackend` methods dispatching real GPU compute on RTX 3090** | ✅ functionally complete (3 methods delegate to reference, matching the Eigen adapter's scope) |
| **Phase 4 — `0.1.0` public release**  | Full tutorial corpus (6 notebooks) ✅ · Jupyter Book TOC populated ✅ (Pages enable pending) · LyX export module + plugin ✅ · `CONTRIBUTING.md` + `CODE_OF_CONDUCT.md` ✅ · `CHANGELOG.md` ✅ · `docs/INDEX.md` project-grounded ✅ · release tag | **release-ready as of 2026-05-12**; maintainer-only steps remain |

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

This project aspires to canonical-reference-quality documentation and design for differentiable named-axis tensor computation in modern C++ ([ADR-0015](./docs/arc42/09-decisions/0015-aspire-to-canonical-reference-quality-not-self-anoint.md)). If you build on it, please cite via [`CITATION.cff`](./CITATION.cff). The ADR sequence under [`docs/arc42/09-decisions/`](./docs/arc42/09-decisions/) is the project's bibliography — individual decisions can be cited as well-defined design choices with rationale. *Whether the work earns canonical-reference status is determined by external adoption, not by self-declaration; the three disciplines aim to make the work ready when (and if) such adoption happens.*

## License

MIT.
