---
status: Draft
owner: tensor
last-reviewed: 2026-05-11
---

# `tensor` — Deployment View (arc42 §7)

| Metadata      | Value                                                          |
| ------------- | -------------------------------------------------------------- |
| Status        | Draft                                                          |
| Type          | arc42 §7 (Deployment View)                                     |
| Owner         | uyuutosa                                                       |
| Last Updated  | 2026-05-11                                                     |

> Per arc42 §7: this file describes **where the artifacts run**. For a header-only library the answer is mostly "wherever the user's compiler lives". This document focuses on the moving parts that *do* have a deployment shape: the Jupyter Book site, the CI matrix, and the published-citable-artifact path.

## 1. Library deployment (the simple half)

`tensor` is **header-only** ([TC-2](../02-architecture-constraints/overview.md), [ADR-0008](../09-decisions/0008-distribute-as-header-only-with-jupyter-tutorials.md)). There is no compiled artifact to publish. Consumers get the library by:

```bash
git clone https://github.com/uyuutosa/tensor.git
cd tensor
cmake --preset=default
cmake --build --preset=default
ctest --preset=default
```

The `cmake --preset=default` resolves dependencies via vcpkg manifest mode against the project's `builtin-baseline` (currently `99a97de2cb371449d4fb9dc970f2ac562d689ec2`; bump documented in [PR #47](https://github.com/uyuutosa/tensor/pull/47)). After install, the only file the user needs to include is:

```cpp
#include <tensor/tensor.hpp>
```

There is no `libtensor.so` / `libtensor.a` / `tensor.lib`. The `CMakeLists.txt` install rules at the repo root produce an `EXPORT tensorTargets.cmake` so downstream CMake projects can `find_package(tensor)` and `target_link_libraries(<target> PRIVATE tensor::tensor)`.

## 2. CI matrix deployment

CI is the project's most active "deployment" surface. The 10-job matrix lives in [`.github/workflows/ci.yml`](../../../.github/workflows/ci.yml):

| Runner | Compiler | Build | Backend |
| ------ | -------- | ----- | ------- |
| ubuntu-22.04 | GCC 11 | Debug + Release | reference |
| ubuntu-22.04 | Clang 13 | Debug + Release | reference |
| macos-14 | AppleClang | Debug + Release | reference |
| windows-2022 | MSVC 19.30+ | Debug + Release | reference |
| ubuntu-22.04 | GCC 11 | Release | eigen |
| ubuntu-22.04 | GCC 11 | Release | webgpu (stub) |

Gated by a fast `vendored-check` lint job that verifies every `third_party/` subdirectory carries a `VENDORED_FROM` file ([PR #41](https://github.com/uyuutosa/tensor/pull/41)).

A future self-hosted runner with a Dawn-compatible GPU is the precondition for P3.M3.2 / P3.M4.2 — adds an 11th job that exercises the real WGSL dispatch path against `reference` for numerical agreement within `1e-5` for `float`. See [ADR-0012 §Decision Outcome](../09-decisions/0012-webgpu-adapter-implementation-design.md).

## 3. Jupyter Book deployment

The book (under [`book/`](../../../book/)) is built by [`.github/workflows/deploy-book.yml`](../../../.github/workflows/deploy-book.yml) on every push to `develop`. The output is published to GitHub Pages (Source: GitHub Actions).

**Activation state**: the workflow exists and runs successfully on every push. The site goes live the first time the maintainer enables Pages in the repo's settings; this is a single-click action not yet performed. The Phase 4 release rehearsal report ([#48](https://github.com/uyuutosa/tensor/pull/48)) lists Pages-enable as a post-release hygiene step, not a release blocker.

When live, the URL will be: `https://uyuutosa.github.io/tensor/`.

## 4. Notebook CI deployment (auxiliary)

Notebooks under [`tutorials/`](../../../tutorials/) get two CI surfaces ([`.github/workflows/notebook-ci.yml`](../../../.github/workflows/notebook-ci.yml)):

- **Validate** (always-on): parses every `.ipynb` as JSON on every push / PR. Catches hand-edit corruption.
- **Execute (xeus-cpp)** (weekly Monday 03:00 UTC cron + workflow_dispatch): installs miniconda + xeus-cpp 0.10+ via conda-forge and runs `jupyter nbconvert --execute` against every notebook with the `xcpp20` kernel. Best-effort; `continue-on-error: true`.
- **Execute (xeus-cling legacy)** (same trigger): runs only `tutorials/00_intro.ipynb` against the xeus-cling kernel as a C++17-subset smoke for users on older conda-forge channels.

## 5. Citation discovery

The project's "deployment" as a *citable work* (per [ADR-0013](../09-decisions/0013-reframe-as-canonical-reference-for-named-tensor-computation.md)) goes through:

- [`CITATION.cff`](../../../CITATION.cff) at the repo root — drives GitHub's "Cite this repository" UI.
- The ADR sequence under [`docs/arc42/09-decisions/`](../09-decisions/) — the project's bibliography.
- The §12 glossary — the project's vocabulary cross-referenced to math literature.

When the maintainer cuts `0.1.0` (Phase 4 close per [#48](https://github.com/uyuutosa/tensor/pull/48)), GitHub Releases attaches the source archive to the tag and the CITATION.cff metadata stays current — downstream citation tools (Zotero, Mendeley, BibTeX exporters) can resolve from the tag.

## 6. What this section does *not* cover

- The compiled artifacts users of *adapters* depend on (Eigen, Dawn, gpu.cpp) — these come from their own deployment pipelines (vcpkg, vendored, etc.).
- The Hugging Face / PyPI / Conda distribution paths — `tensor` is not distributed via those channels and has no plan to be.

## 7. Cross-references

- §2 constraints shaping deployment: [TC-2 header-only, TC-3 CMake + vcpkg](../02-architecture-constraints/overview.md)
- §5 building blocks each artifact comes from: [`../05-building-blocks/overview.md`](../05-building-blocks/overview.md)
- §11 risks with a deployment dimension (substrate, baseline bump): [`../11-risks/overview.md`](../11-risks/overview.md)
- Release ceremony: [`docs/reports/2026-05-11_phase-4-release-rehearsal.md`](../../reports/2026-05-11_phase-4-release-rehearsal.md) §3
- Pages enable instructions: README §Documentation
