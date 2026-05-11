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

## Notebook corpus

| #   | Title                                              | Status      | Phase introduced |
| --- | -------------------------------------------------- | ----------- | ---------------- |
| 00  | Intro — modernized 2016 blog post                  | **shipped** | 1 (M6) |
| 05  | Build your own autograd from scratch               | **shipped** | 2 (P2.M5) |
| 06  | WebGPU acceleration (design walkthrough)           | **shipped** | 3 (P3.M6) |
| 07  | A small MLP on a toy dataset                       | **shipped** | 2 (P2.M6) |
| 08  | Swappable backends — Hexagonal payoff (reference vs Eigen) | **shipped** | 2.5 (P2.5.M5) |

The five shipped notebooks cover the pedagogical arc end-to-end:

- **00 + 07** demonstrate the headline named-axis story and a real training loop converging to W ≈ 2, b ≈ 1.
- **05** walks the tape-based autograd primitive-by-primitive.
- **08** demonstrates the Hexagonal-lite architectural payoff (same Domain code on reference / Eigen).
- **06** narrates the WGSL kernels + Dawn dispatch design (and now references real-GPU-verified shipped kernels via PRs #60 / #61 / #62).

Earlier planning material referenced slots `01_named-axis-fundamentals`, `02_function-and-reference-tensors`, `03_convolutions`, and `04_tex-bridge` as "scaffolded". Those numbers are **explicitly out of scope** per the Phase 4 release rehearsal (PR #48 §2 + #63): notebook 00 already absorbs the 2016 Qiita post's named-axis fundamentals + function/reference tensors + convolutions material, and notebook 06's WGSL walkthrough plus the `tensor::tex` Evaluator (with the [`lyx-export/`](../lyx-export/) plugin) cover the `_tex` bridge end-to-end. Adding scaffolded notebooks under those numbers would duplicate material that is already legible elsewhere.

## Editing rules

- **Notebooks track the live API.** When the API of a tutorial-cited symbol changes, the notebook is updated in the same PR.
- **CI validates every notebook's JSON on every push** (see [`.github/workflows/notebook-ci.yml`](../.github/workflows/notebook-ci.yml)); a corrupted file is rejected.
- **CI executes every notebook weekly** via xeus-cpp on a Monday-03:00-UTC cron. The job is best-effort and `continue-on-error: true`. A failure files an implicit signal that the notebooks need updating against the current API surface — file an issue if you notice the badge red. A parallel `legacy-xeus-cling` job runs only `00_intro.ipynb` against xeus-cling to keep the C++17-subset smoke green for users on older conda-forge channels.
- **No long-form prose lifts** from the architecture documentation. Notebooks are *learning experiences*; reference material lives under `docs/`.

## Source for `00_intro`

The intro notebook is a modernized port of the author's [2016 Qiita post on convolutions-as-tensor-inner-products](http://qiita.com/uyuutosa/items/12e87f4695bd151b1d74). The blog post is the spiritual ancestor of the entire project; the notebook makes its narrative runnable.
