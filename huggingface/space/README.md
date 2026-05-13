---
title: tensor — named-axis Einstein evaluator
emoji: 🧮
colorFrom: indigo
colorTo: purple
sdk: gradio
sdk_version: 4.44.0
app_file: app.py
pinned: false
license: mit
short_description: Live LaTeX → named-axis tensor evaluation (Phase 6 SDK demo)
---

# `tensor` — named-axis differentiable C++ library, live in your browser

This Space hosts an interactive demo of the [`tensor`](https://github.com/uyuutosa/tensor) Python SDK (Phase 6, ADR-0018). The headline feature is `tensor.tex`: parse a LaTeX-subset expression, bind named tensors to its leaves, and evaluate against the C++ Einstein-broadcast machinery — *the formula is the program*.

## What you can try

1. **TeX evaluator** — type a LaTeX expression like `c_{ij} = a_i b_j` (outer product) or `\sum_i {a_i b_i}` (inner product), bind values for the named tensors, and see the result.
2. **Named-axis broadcast** — pick two axes and watch how same-label tensors contract while disjoint-label tensors broadcast outer.
3. **Autograd** — a small differentiable example that runs the C++ reverse-mode tape end-to-end through nanobind.

## Cold-start

The Space installs the SDK from source on first launch (nanobind + scikit-build-core + a C++20 toolchain via `packages.txt`). Build takes 2–4 minutes the first time the container spins up; subsequent loads are warm and fast.

Once `tensor-named-axis` lands on PyPI (Phase 6.M6 release), the install path will collapse to `pip install tensor-named-axis` and cold-start drops to seconds.

## See also

- Repo: <https://github.com/uyuutosa/tensor>
- Docs site: <https://uyuutosa.github.io/tensor/>
- Python notebooks (Colab badges): <https://github.com/uyuutosa/tensor#try-it-without-installing>
- Phase 6 ADR: [`0018-phase-6-python-sdk-entry-via-nanobind.md`](https://github.com/uyuutosa/tensor/blob/develop/docs/arc42/09-decisions/0018-phase-6-python-sdk-entry-via-nanobind.md)
