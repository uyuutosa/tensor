# tutorials/

Hands-on Jupyter notebooks introducing `tensor`, executed against the [xeus-cling](https://github.com/jupyter-xeus/xeus-cling) C++20 kernel. Per [ADR-0008](../docs/arc42/09-decisions/0008-distribute-as-header-only-with-jupyter-tutorials.md), these notebooks are the **primary educational deliverable**: each release executes them end-to-end in CI, and a Jupyter Book site generated from this directory is published to GitHub Pages.

## Environment

A reproducible conda environment is provided:

```bash
conda env create -f environment.yml
conda activate tensor-tutorials
jupyter lab
```

This pulls xeus-cling, vcpkg, the C++20 toolchain, and Jupyter Book. Once the env is active, `jupyter lab` opens with the C++20 kernel selectable per notebook.

## Planned progression

| #   | Title                                              | Status      | Phase introduced |
| --- | -------------------------------------------------- | ----------- | ---------------- |
| 00  | Intro — modernized 2016 blog post                  | **shipped** (this directory) | 1 (M6) |
| 01  | Named-axis fundamentals                            | scaffolded  | 1 |
| 02  | Function and reference tensors as teaching exhibits | scaffolded  | 1 |
| 03  | Convolutions reformulated as tensor inner products | scaffolded  | 1 |
| 04  | TeX bridge — *the formula is the program*          | scaffolded  | 1 |
| 05  | Build your own autograd from scratch               | planned     | 2 |
| 06  | WebGPU acceleration                                | planned     | 3 |
| 07  | A small MLP on a toy dataset                       | planned     | 2 |

The first four notebooks together reproduce the 2016 README's narrative end-to-end on the new C++20 API. Notebooks 05–07 land in Phase 2 / Phase 3 respectively (see [`../docs/impl-plans/`](../docs/impl-plans/)).

## Editing rules

- **Notebooks track the live API.** When the API of a tutorial-cited symbol changes, the notebook is updated in the same PR.
- **CI executes every notebook end-to-end** at release-tag time (planned for Phase 1 close); a notebook that throws or produces wrong output blocks the release.
- **No long-form prose lifts** from the architecture documentation. Notebooks are *learning experiences*; reference material lives under `docs/`.

## Source for `00_intro`

The intro notebook is a modernized port of the author's [2016 Qiita post on convolutions-as-tensor-inner-products](http://qiita.com/uyuutosa/items/12e87f4695bd151b1d74). The blog post is the spiritual ancestor of the entire project; the notebook makes its narrative runnable.
