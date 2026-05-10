---
status: Accepted
owner: tensor
last-reviewed: 2026-05-10
---

# ADR-0008: Distribute as a header-only library with bundled xeus-cling Jupyter tutorials

| Metadata  | Value                                                          |
| --------- | -------------------------------------------------------------- |
| Status    | **Accepted**                                                   |
| Type      | ADR (Type 5 / MADR v3.0 / arc42 §9)                            |
| Date      | 2026-05-10                                                     |
| Deciders  | uyuutosa                                                       |
| Consulted | Claude (research brief §5)                                     |
| Informed  | future contributors                                            |
| Ticket    | —                                                              |

---

## Context and Problem Statement

An educational C++ library succeeds or fails on how easy it is for a learner to *try* it. The research brief §5 notes that successful didactic genre members ([micrograd](https://github.com/karpathy/micrograd), [tinygrad](https://github.com/tinygrad/tinygrad), [teenygrad](https://github.com/j4orz/teenygrad), [Tenseur](https://istmarc.github.io/post/2024/10/27/on-designing-tenseur-a-c-tensor-library-with-lazy-evaluation/), [Fastor](https://github.com/romeric/Fastor)) all bundle pedagogy with code; the projects that don't, fade.

The maintainer has chosen Jupyter notebooks (via [xeus-cling](https://github.com/jupyter-xeus/xeus-cling) — a Cling-based C++ Jupyter kernel) as part of the deliverable. This ADR records the full distribution model.

---

## Decision Drivers

- **DD-1**: Cost of entry. A learner should be able to `git clone` and run a tutorial within minutes.
- **DD-2**: Reproducibility. Tutorials must run on the published library version, not on `main` of the day.
- **DD-3**: Pedagogical alignment. Notebooks render LaTeX (ADR-0005), animate intermediate values, and let the learner edit & re-run.
- **DD-4**: Build cost. Header-only avoids ABI commitments and prebuilt binaries.

---

## Considered Options

1. **Header-only library, no notebooks.**
2. **Header-only library + xeus-cling Jupyter notebooks bundled in `tutorials/`.**
3. **Pre-built shared library + notebooks.**
4. **Header-only library + companion online book / blog (mdBook / Quarto / Jupyter Book).**

---

## Decision Outcome

**Chosen option: 2 + 4 — Header-only library with bundled xeus-cling Jupyter tutorials in `tutorials/`, plus a [Jupyter Book](https://jupyterbook.org/) site published from those notebooks via GitHub Pages.**

Header-only is the natural distribution for a teaching C++ template library: no ABI commitments, no prebuilt binaries to maintain across platforms, learners build into their own translation unit. Bundled notebooks ([xeus-cling](https://github.com/jupyter-xeus/xeus-cling) supports C++20 in 2025+) give the "open and run" experience that micrograd / tinygrad have established as the genre default. A Jupyter Book deployed to GitHub Pages converts those same notebooks into a free online book without duplicate authoring — closing DD-3 pedagogical alignment without doubling content.

The 2016 author's [Qiita blog post](http://qiita.com/uyuutosa/items/12e87f4695bd151b1d74) becomes the introduction chapter, modernized.

### Y-statement summary

> In the context of **shipping the rewritten `tensor` library to learners**, facing **the choice of distribution medium**, we decided for **a header-only library with bundled xeus-cling Jupyter tutorials and a Jupyter Book site published from those notebooks**, to achieve **a "clone and learn" experience matching the educational genre default**, accepting **the maintenance cost of keeping notebooks runnable against released versions**.

---

## Pros and Cons of the Options

### Option 1: Header-only, no notebooks

- Pros: simplest scope.
- Cons: learners drop off; the educational pitch is unsupported by deliverables.

### Option 2: Header-only + bundled notebooks (chosen)

- Pros: matches genre defaults; xeus-cling C++20 support exists; LaTeX/HTML rendering native; round-trips with TeX/LyX integration (ADR-0005).
- Cons: notebooks rot; CI must execute them per release; xeus-cling has occasional fragility on bleeding-edge C++.

### Option 3: Prebuilt shared library + notebooks

- Pros: faster notebook startup.
- Cons: ABI commitments; per-platform binaries; out of scope for a solo project.

### Option 4: Header-only + companion online book (chosen as second layer)

- Pros: free hosting via GitHub Pages; SEO; one canonical narrative.
- Cons: extra build step (Jupyter Book CI).

---

## Consequences

### Positive

- `git clone && jupyter lab tutorials/00_intro.ipynb` is the "hello world" path.
- The Jupyter Book site (e.g. `tensor.uyuutosa.dev` or `https://uyuutosa.github.io/tensor/`) acts as both reference and tutorial, free of charge.
- TeX/LyX integration (ADR-0005) shows up in notebook output cells as proper LaTeX, closing the "code reads like the formula" loop.
- The autograd tutorial (ADR-0007) and the WebGPU tutorial (ADR-0006) get a rendered home.

### Negative

- Notebooks must be regenerated and re-tested every release; CI must execute them.
- xeus-cling tracks the Cling C++ frontend, which historically lags slightly on language standard support. A notebook running C++20 with the polyfilled `mdspan` is realistic; a notebook running C++23 modules is not.
- A Jupyter Book deployment workflow (GitHub Pages + Actions) must be added.

### Neutral

- A future move toward Quarto or mdBook is possible without breaking the bundled notebooks.

### Follow-ups

- [ ] Add `tutorials/` directory with `00_intro.ipynb` (modernized blog post), `01_named-axes.ipynb`, `02_function-and-reference-tensors.ipynb`, `03_convolutions-as-inner-products.ipynb`, `04_tex-bridge.ipynb`, `05_autograd-from-scratch.ipynb`, `06_webgpu-acceleration.ipynb`, `07_mlp-on-mnist.ipynb`.
- [ ] CI workflow that executes every notebook end-to-end on every release.
- [ ] Jupyter Book scaffold (`book/_config.yml`, `book/_toc.yml`) generating from `tutorials/`.
- [ ] GitHub Pages deployment via Actions.
- [ ] License clarification: notebooks under same MIT terms as code.

---

## Compliance / Validation

- Verification: every notebook runs end-to-end in CI without errors at release-tagging time.
- Frequency: every release tag.

---

## More Information

### Related ADRs

- Predecessors: [ADR-0001](0001-pivot-to-educational-named-axis-dsl.md), [ADR-0002](0002-rewrite-on-cpp20-baseline-with-mdspan-interop.md), [ADR-0004](0004-adopt-hybrid-named-axis-api.md), [ADR-0005](0005-adopt-tex-lyx-as-authoring-surface.md), [ADR-0006](0006-adopt-webgpu-as-gpu-backend.md), [ADR-0007](0007-adopt-autograd-as-first-class-subsystem.md)

### References

- [xeus-cling — C++ Jupyter kernel](https://github.com/jupyter-xeus/xeus-cling)
- [Jupyter Book](https://jupyterbook.org/)
- [GitHub Pages docs](https://docs.github.com/en/pages)
- Author's [2016 Qiita blog post](http://qiita.com/uyuutosa/items/12e87f4695bd151b1d74) (becomes intro chapter)
- [tinygrad's tutorial-driven onboarding](https://github.com/tinygrad/tinygrad/tree/master/docs)
- Research brief: `docs/reports/2026-05-10_tensor-revival-landscape.md` §5
