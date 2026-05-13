---
status: Draft
owner: tensor
last-reviewed: 2026-05-12
---

# `tensor` — System Context (arc42 §3)

| Metadata      | Value                                                          |
| ------------- | -------------------------------------------------------------- |
| Status        | Draft                                                          |
| Type          | arc42 §3 (Context and Scope)                                   |
| Owner         | uyuutosa                                                       |
| Related       | `arc42/01-introduction-and-goals/overview.md`; ADR-0001..ADR-0017 (the supersession + refinement chain in force as of 2026-05-12: ADR-0013 superseded by ADR-0015; ADR-0014 §Decision Outcome point 2 refined by ADR-0016; ADR-0015 §Compliance bullet 3 refined by ADR-0017) |
| Last Updated  | 2026-05-12                                                     |

> The C4 Level 1 system-context diagram lives in [`../../diagrams/c4/workspace.dsl`](../../diagrams/c4/workspace.dsl). Render via the [`/diagram-render`](../../../.claude/skills/diagram-render/SKILL.md) skill; this file mirrors the diagram in prose and tables.

## Reading order

1. **Actors** — who interacts with `tensor`.
2. **External systems** — what `tensor` depends on at runtime / build time.
3. **Boundaries** — what is explicitly inside vs. outside the system.
4. **Out-of-band channels** — non-runtime relationships (e.g. upstream subtree).

---

## 1. Actors

| Identifier   | Role                                | Goal                                                                                       | Primary interaction pattern |
| ------------ | ----------------------------------- | ------------------------------------------------------------------------------------------ | --------------------------- |
| `learner`    | Self-taught C++ programmer or researcher | Understand named-axis tensor algebra, autograd, and modern C++ by reading and running the tutorials. Primary audience. | Browses the [Pages site](https://uyuutosa.github.io/tensor/); clones repo; runs `cmake --preset=default && jupyter lab tutorials/`; uses [`docs/user-manual/how-to/`](../../user-manual/how-to/) when stuck. |
| `instructor` | University / bootcamp instructor    | Assign tutorials and exercises as coursework. Secondary audience.                          | Forks the repo to create an exercise variant; cites the ADR sequence in a syllabus; uses tutorial 00–08 as lecture material. |
| `cv-learner` (Phase 6) | Computer-vision student working through Hartley & Zisserman | Reproduce bifocal / trifocal / quadrifocal tensor results via autograd; read the MVG demos. | Opens the Pages site, follows the Colab badge on `python/notebooks/03_*` or `04_*`, reads the paper-style math + 3D plotly figures. |
| `py-ml-practitioner` (Phase 6) | Data scientist evaluating named-axis libs for an internal tool | `pip install tensor-named-axis`; run a small Einstein-broadcast computation; measure parity vs NumPy / PyTorch. | `pip install tensor-named-axis[eigen]`; opens `python/notebooks/00_python-sdk-tour.ipynb` on Binder for the 5-min onboarding. |
| `gradio-visitor` (Phase 6) | Anyone arriving from a social-media link to the HuggingFace Space | Try `tex.Evaluator` / broadcast / autograd interactively without installing anything. | Visits `huggingface.co/spaces/<user>/tensor-named-axis-demo`; types LaTeX into the Gradio textbox; reads the result. |

The library is **not** intended for production engineers picking a tensor lib for a real workload — that audience is redirected to Eigen / xtensor / libtorch / Kokkos / `std::linalg` (see [ADR-0001](../09-decisions/0001-pivot-to-educational-named-axis-dsl.md) §Stakeholders). The Python wedge (Phase 6+) opens a *teaching* surface for production Python users — those who want to *read* idiomatic named-axis code without committing their stack to it.

## 2. External systems

| Identifier            | Description                                                                          | Interface                              |
| --------------------- | ------------------------------------------------------------------------------------ | -------------------------------------- |
| `toolchain`           | Host C++ compiler + build tools: GCC 11+ / Clang 13+ / MSVC 19.30+, CMake 3.25+, vcpkg | Compile-time. `tensor` is consumed as a header-only CMake interface library. |
| `jupyterStack`        | Jupyter Lab / Notebook with the [xeus-cpp](https://github.com/compiler-research/xeus-cpp) 0.10+ `xcpp20` C++20 kernel (ADR-0014 §3); a parallel `legacy-xeus-cling` job keeps `00_intro.ipynb` runnable on older conda-forge channels via [xeus-cling](https://github.com/jupyter-xeus/xeus-cling) | Runtime. Tutorials execute inside this kernel. |
| `webgpuRuntime`       | [Dawn](https://dawn.googlesource.com/dawn) — vcpkg port `20260410.140140` (ADR-0014 §1 + ADR-0016) — uses the OS GPU driver (Vulkan / Metal / D3D12) already on the learner's machine | Runtime. `tensor::core::backend::webgpu` dispatches WGSL kernels through the [WebGPU API](https://www.w3.org/TR/webgpu/) via Dawn's own `webgpu_cpp.h`. As of 2026-05-12, 12 of 15 `KernelBackend` methods dispatch real GPU compute on `float`. |
| `browser`             | Chrome / Firefox / Safari with WebGPU support                                        | Reads the Jupyter Book site. (Future) runs browser-side demos. |
| `ghPages`             | GitHub Pages                                                                         | Build-time deploy target for the Jupyter Book site.            |
| `upstreamPentaglyph`  | The [pentaglyph-docs](https://github.com/uyuutosa/pentaglyph-docs) repository — vendored as a git subtree at `libs/pentaglyph-docs/` | Build-time / authoring-time. `git subtree pull` keeps the kit current. |
| `pypi`                | [PyPI](https://pypi.org/project/tensor-named-axis/) — distributes the Python SDK wheel matrix (`tensor-named-axis`, `tensor-named-axis-eigen`, `tensor-named-axis-webgpu` per [ADR-0019](../09-decisions/0019-phase-6-5-runtime-backend-selection-via-extras.md)) | Install-time. `pip install tensor-named-axis[<extras>]` brings the chosen backend(s). Publish happens via OIDC trusted publishing from `.github/workflows/cibuildwheel.yml` on tag push (per [ADR-0018](../09-decisions/0018-phase-6-python-sdk-entry-via-nanobind.md) §M6). |
| `pythonInterp`        | CPython 3.9 – 3.13 — the runtime that loads the nanobind-compiled `_tensor_native_<backend>.so` extension(s). | Runtime. Python entry points re-enter the C++ Domain through nanobind; no algebra logic lives in Python. |
| `numpyArray`          | NumPy ≥ 1.20 `ndarray` — the standard interop type for getting data into / out of `DynamicTensor`. | Runtime. `tensor.from_numpy(arr, labels)` and `t.numpy()` round-trip with copy semantics. |
| `binder` / `colab`    | [mybinder.org](https://mybinder.org/v2/gh/uyuutosa/tensor/main?labpath=python%2Fnotebooks) and [Google Colab](https://colab.research.google.com/) — host the Python notebook corpus without local install. | Runtime. The first cell `pip install`s the SDK from upstream Git; cold-start ~3–5 min while nanobind builds the extension. Will collapse to `pip install tensor-named-axis` after the Phase 6.M6 PyPI publish. |
| `hfSpace`             | [HuggingFace Spaces](https://huggingface.co/spaces) — Gradio-SDK hosted demo (`huggingface.co/spaces/<user>/tensor-named-axis-demo`) sourced from `huggingface/space/` in-tree | Runtime. The Space pulls the SDK from `git+https://github.com/uyuutosa/tensor.git@develop` until the PyPI publish lands; deploy is one `./huggingface/space/deploy.sh <user>` command (per `huggingface/space/DEPLOY.md`). |

## 3. Boundaries — what is inside the system

The system under design is **the `tensor` repository as a whole**, including:

- The header-only C++20 library (`tensor/` namespace, distributed via `include/`).
- The `tutorials/` notebook corpus.
- The Jupyter Book site generated from those notebooks.
- The LyX export module + plugin (`lyx-export/`).

The system does **not** include the C++ toolchain, the Jupyter stack, or the WebGPU runtime — those are external dependencies the learner installs. The rule of thumb: anything the maintainer can fix by editing this repo is inside; anything else is outside.

## 4. Out-of-band channels (non-runtime)

| Channel                                       | Direction          | Purpose                                                                  |
| --------------------------------------------- | ------------------ | ------------------------------------------------------------------------ |
| `git subtree pull --prefix=libs/pentaglyph-docs ... main --squash` | inbound | Pulls upstream documentation-kit changes from `uyuutosa/pentaglyph-docs`. |
| `git subtree push` (rare)                     | outbound           | Pushes documentation-kit improvements made here back upstream.            |
| GitHub Issues / PRs                           | bidirectional      | Community contributions and discussion.                                   |
| Jupyter Book site (`ghPages`)                 | outbound           | Public-facing reading surface; unlike issues this is one-way.             |
| The 2016 Qiita blog post                      | inbound (legacy)   | Source content for `tutorials/00_intro.ipynb`. Treated as external prior art. |

## 5. Relationships (mirrors the DSL)

| From          | To                | Verb                                                       |
| ------------- | ----------------- | ---------------------------------------------------------- |
| `learner`     | `tensor`          | Builds against, reads, and runs tutorials of               |
| `instructor`  | `tensor`          | Assigns tutorials and exercises from                       |
| `tensor`      | `toolchain`       | Is consumed by (CMake + vcpkg)                             |
| `tensor`      | `jupyterStack`    | Tutorials run inside (xeus-cpp / xeus-cling kernel)        |
| `tensor`      | `webgpuRuntime`   | Dispatches GPU kernels through (Dawn via `webgpu_cpp.h`)   |
| `tensor`      | `ghPages`         | Jupyter Book site is published to                          |
| `tensor`      | `upstreamPentaglyph` | Vendored documentation kit is pulled from               |

L2 (container-level) relationships are documented in [`../05-building-blocks/overview.md`](../05-building-blocks/overview.md) to avoid duplicating the DSL.

## 7. System-context diagram

The C4 Level-1 (context) view is rendered from [`../../diagrams/c4/workspace.dsl`](../../diagrams/c4/workspace.dsl) by the [`/diagram-render`](../../skills/diagram-render/SKILL.md) skill. As of 2026-05-14 the `docs/diagrams/c4/exports/` directory does not yet contain a committed SVG — generation has been deferred to a future cycle (regenerating requires `structurizr-cli` + the docs-style hooks). The DSL itself is the source of truth for the boxes-and-arrows; this `§3 system-context.md` table is the canonical text.

When the SVG lands, it will be linked here as `exports/c4-l1-context.svg` and the deploy-book pipeline will surface it in the rendered Pages site.

The Phase 6 expansion (Python SDK + HuggingFace Space + PyPI / Binder / Colab externals) is reflected in the latest `workspace.dsl` commit (see §5 below for the container-level relationships); render lag is tracked as a docs-system soft-blocker, not a code blocker.

## 8. What is *not* in scope here

- Internal decomposition of the library — see [`../05-building-blocks/`](../05-building-blocks/).
- Detailed runtime sequences (e.g. autograd backward pass) — see [`../06-runtime/`](../06-runtime/).
- Module HOW (data layout, kernel implementation) — see [`../../detailed-design/`](../../detailed-design/).

## Cross-references

- §1 Introduction and Goals: [`../01-introduction-and-goals/overview.md`](../01-introduction-and-goals/overview.md)
- §4 Solution Strategy: [`../04-solution-strategy/strategy.md`](../04-solution-strategy/strategy.md)
- §5 Building Blocks: [`../05-building-blocks/overview.md`](../05-building-blocks/overview.md)
- §9 Decisions: [`../09-decisions/`](../09-decisions/)
- C4 source: [`../../diagrams/c4/workspace.dsl`](../../diagrams/c4/workspace.dsl)
