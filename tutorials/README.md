# tutorials/

Hands-on Jupyter notebooks introducing `tensor`, executed against the [xeus-cpp](https://github.com/compiler-research/xeus-cpp) `xcpp20` kernel per [ADR-0014](../docs/arc42/09-decisions/0014-external-substrate-strategy.md) (xeus-cling was the original choice but is effectively frozen at C++17 / Clang-13). Per [ADR-0008](../docs/arc42/09-decisions/0008-distribute-as-header-only-with-jupyter-tutorials.md), these notebooks are the **primary educational deliverable**: each release executes them end-to-end in CI, and a Jupyter Book site generated from this directory is published to GitHub Pages.

## Environment

A reproducible conda environment is provided:

```bash
conda env create -f environment.yml
conda activate tensor-tutorials
jupyter lab
```

This pulls xeus-cpp 0.10.0+, vcpkg, the C++20 toolchain, and Jupyter Book. Once the env is active, `jupyter lab` opens with the `xcpp20` kernel selectable per notebook.

## Planned progression

| #   | Title                                              | Status      | Phase introduced |
| --- | -------------------------------------------------- | ----------- | ---------------- |
| 00  | Intro — modernized 2016 blog post                  | **shipped** (this directory) | 1 (M6) |
| 01  | Named-axis fundamentals                            | scaffolded  | 1 |
| 02  | Function and reference tensors as teaching exhibits | scaffolded  | 1 |
| 03  | Convolutions reformulated as tensor inner products | scaffolded  | 1 |
| 04  | TeX bridge — *the formula is the program*          | scaffolded  | 1 |
| 05  | Build your own autograd from scratch               | **shipped** (this directory) | 2 (P2.M5) |
| 06  | WebGPU acceleration (design walkthrough)           | **shipped** (this directory; live-execution form pending GPU runner) | 3 (P3.M6) |
| 07  | A small MLP on a toy dataset                       | **shipped** (this directory) | 2 (P2.M6) |
| 08  | Swappable backends — Hexagonal payoff (reference vs Eigen) | **shipped** (this directory) | 2.5 (P2.5.M5) |

The first four notebooks together reproduce the 2016 README's narrative end-to-end on the new C++20 API. Notebooks 05–07 land in Phase 2 / Phase 3 respectively (see [`../docs/impl-plans/`](../docs/impl-plans/)).

## Editing rules

- **Notebooks track the live API.** When the API of a tutorial-cited symbol changes, the notebook is updated in the same PR.
- **CI validates every notebook's JSON on every push** (see [`.github/workflows/notebook-ci.yml`](../.github/workflows/notebook-ci.yml)); a corrupted file is rejected.
- **CI executes every notebook weekly** via xeus-cpp on a Monday-03:00-UTC cron. The job is best-effort and `continue-on-error: true`. A failure files an implicit signal that the notebooks need updating against the current API surface — file an issue if you notice the badge red. A parallel `legacy-xeus-cling` job runs only `00_intro.ipynb` against xeus-cling to keep the C++17-subset smoke green for users on older conda-forge channels.
- **No long-form prose lifts** from the architecture documentation. Notebooks are *learning experiences*; reference material lives under `docs/`.

## Source for `00_intro`

The intro notebook is a modernized port of the author's [2016 Qiita post on convolutions-as-tensor-inner-products](http://qiita.com/uyuutosa/items/12e87f4695bd151b1d74). The blog post is the spiritual ancestor of the entire project; the notebook makes its narrative runnable.
