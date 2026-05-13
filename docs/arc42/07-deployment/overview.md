---
status: Draft
owner: tensor
last-reviewed: 2026-05-12
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

## 4b. Python SDK wheel deployment (added in Phase 6)

The Python SDK (`tensor-named-axis` on PyPI) ships via [`.github/workflows/cibuildwheel.yml`](../../../.github/workflows/cibuildwheel.yml). The workflow is triggered by:

- `workflow_dispatch` — manual smoke before tag-cut. Builds wheels + sdist; does NOT publish.
- `push` of a tag matching `v*` or `[0-9]*.[0-9]*.[0-9]*` — builds + publishes.

**Matrix at `0.2.0`** (one backend per wheel, reference):
- Linux x86_64 / manylinux2014 / CPython 3.9–3.13 = 5 wheels
- macOS x86_64 / macos-13 / CPython 3.9–3.13 = 5
- macOS arm64 / macos-14 / CPython 3.9–3.13 = 5
- Windows x86_64 / windows-latest / CPython 3.9–3.13 = 5
= **20 wheels + 1 sdist** per release.

**Matrix at `0.3.0` (Phase 6.5)** per [ADR-0019](../09-decisions/0019-phase-6-5-runtime-backend-selection-via-extras.md): triples to ~52 wheels by adding `tensor-named-axis-eigen` and `tensor-named-axis-webgpu` companion distributions (Windows WebGPU is M3-scoped out pending Dawn vcpkg port verification).

**Authentication**: PyPA trusted publishing via OIDC (no API tokens in repo secrets). The maintainer's one-time setup is PyPI project Settings → Publishing → Add → GitHub publisher with workflow `cibuildwheel.yml` + environment `pypi`.

**Smoke**: each per-platform build runs `python -c "import tensor; assert tensor.hello() == 'hello from tensor::core'"` post-build before uploading.

## 4c. HuggingFace Space deployment (added in Phase 6)

The Gradio-SDK Space sourced from [`huggingface/space/`](../../../huggingface/space/) is the project's free hosted demo. Topology: the in-tree directory is the source of truth; the canonical deploy target is a separate `huggingface.co/spaces/<user>/tensor-named-axis-demo` Git remote. The maintainer-side deploy is one command:

```bash
hf auth login                                       # one-time
./huggingface/space/deploy.sh <hf-username>         # creates Space, syncs, pushes
```

Cold-start on HuggingFace's free CPU runner is ~3–5 min while nanobind builds the C++ extension from `git+https://github.com/uyuutosa/tensor.git@develop`. Will collapse to seconds once the `requirements.txt` switches to `tensor-named-axis>=0.2.0` from PyPI after the first wheel publish.

## 4d. conda-forge deployment (planned — Phase 6.5 follow-up)

Conda-forge distribution is a Phase 6.5 follow-up (per the [Phase 6.5 impl-plan](../../impl-plans/2026-05-13_phase-6-5-set-backend.md) §"Out of scope"). The intended shape once it ships:

- A separate `tensor-feedstock` repo under the conda-forge GitHub org (standard convention; the maintainer submits a PR to `conda-forge/staged-recipes` to bootstrap).
- A `recipe/meta.yaml` with three outputs mirroring the PyPI extras structure: `tensor-named-axis` (reference baseline), `tensor-named-axis-eigen`, `tensor-named-axis-webgpu`.
- conda-forge's CI matrix builds across Linux x86_64 / macOS x86_64 / macOS arm64 / Windows x86_64 × CPython 3.9–3.13, same shape as cibuildwheel.
- Eigen + Dawn conda-forge ports satisfy the runtime deps; no vendoring change required.

Until this lands, conda users install via `pip install` inside a conda env — same wheels, just bridged through conda's Python.

## 4e. Deployment-incident runbook

When a deployment-channel surface breaks (Pages 404s, PyPI rejects upload, HF Space stuck building, cibuildwheel matrix red on one row), the entry-point doc is [`../../postmortems/`](../../postmortems/) once an incident lands. As of 2026-05-14 the directory contains only the README — no incidents have been severe enough to file a postmortem ([severity threshold: Medium and above](../../postmortems/README.md)).

Anticipated incident patterns (preregistered in the §11 risks table):

- **R-P6 / R-P7 / R-P8 lessons-learned** — these have been classified and mitigated rather than written up as postmortems because they were caught during active development (PRs #116 / #117 / #118 / #120) before any user impact. If a future regression reintroduces one of these patterns *with* user impact, a postmortem entry under `postmortems/YYYY-MM-DD_<title>.md` is the right artifact.
- **PyPI / OIDC trusted-publishing failure** — most likely scenario is the maintainer's trusted-publisher policy expiring or the workflow file path drifting away from what the policy expects. Postmortem template should call out the exact PyPI Settings → Publishing page state at the time of the failure.
- **HF Space build timeout** — pip-install-from-Git path is sensitive to network state; first occurrence would get a postmortem only if HF Spaces support can't explain it.

## 5. Citation discovery

The project's "deployment" as a *citable work* (per [ADR-0015](../09-decisions/0015-aspire-to-canonical-reference-quality-not-self-anoint.md), superseding [ADR-0013](../09-decisions/0013-reframe-as-canonical-reference-for-named-tensor-computation.md)) goes through:

- [`CITATION.cff`](../../../CITATION.cff) at the repo root — drives GitHub's "Cite this repository" UI.
- The ADR sequence under [`docs/arc42/09-decisions/`](../09-decisions/) — the project's bibliography.
- The §12 glossary — the project's vocabulary cross-referenced to math literature.

When the maintainer cuts `0.1.0` (Phase 4 close per [#48](https://github.com/uyuutosa/tensor/pull/48)), GitHub Releases attaches the source archive to the tag and the CITATION.cff metadata stays current — downstream citation tools (Zotero, Mendeley, BibTeX exporters) can resolve from the tag.

## 6. What this section does *not* cover

- The compiled artifacts users of *adapters* depend on (Eigen via vcpkg `eigen3`; Dawn via vcpkg `dawn[core,vulkan]` per [ADR-0014 §1](../09-decisions/0014-external-substrate-strategy.md)) — these come from their own deployment pipelines.
- Conda-forge distribution — planned as a Phase 6.5 follow-up; the `tensor-feedstock` recipe scaffold is the next deliverable after the maintainer's PyPI trusted-publisher setup.

## 7. Cross-references

- §2 constraints shaping deployment: [TC-2 header-only, TC-3 CMake + vcpkg](../02-architecture-constraints/overview.md)
- §5 building blocks each artifact comes from: [`../05-building-blocks/overview.md`](../05-building-blocks/overview.md)
- §11 risks with a deployment dimension (substrate, baseline bump): [`../11-risks/overview.md`](../11-risks/overview.md)
- Release ceremony: [`docs/reports/2026-05-11_phase-4-release-rehearsal.md`](../../reports/2026-05-11_phase-4-release-rehearsal.md) §3
- Pages enable instructions: README §Documentation
