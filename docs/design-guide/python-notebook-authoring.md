---
status: Stable
owner: tensor
last-reviewed: 2026-05-13
---

# Python notebook authoring — `python/notebooks/`

> Every `python/notebooks/*.ipynb` flows through three layers: the notebook source, the executed JSON committed to Git, and the Jupyter Book HTML rendered to <https://uyuutosa.github.io/tensor/>. Getting from a working source to a working published page has surprising failure modes; this guide collects what was learned the hard way during Phase 6.

## 1. The one absolute rule — execute before you commit

`book/_config.yml` sets `execute_notebooks: "off"`. The deployed Jupyter Book renders the JSON *as committed*. A notebook with `execution_count: None` and zero outputs renders as source-only on the published site — code blocks visible, nothing else.

CI enforces this via the **notebook output gate** added in PR #118 (`notebook-ci.yml` `validate` step). Every `python/notebooks/*.ipynb` must satisfy:

- At least one code cell has a non-null `execution_count`.
- At least one cell has non-empty `outputs`.

Either signal proves the committer ran the notebook before staging.

To execute and commit in one shot:

```bash
jupyter nbconvert --to notebook --inplace --execute \
    --ExecutePreprocessor.timeout=900 \
    python/notebooks/<file>.ipynb
git add python/notebooks/<file>.ipynb
```

C++ tutorials under `tutorials/` are *not* gated because their `xcpp20` kernel requires a conda environment — the weekly cron in `notebook-ci.yml` covers their execution separately.

## 2. Plotly + MathJax — the conflict you'll hit twice

Plotly's `notebook_connected` renderer hard-codes `include_mathjax="cdn"`, which injects `<script src="https://cdnjs.cloudflare.com/ajax/libs/mathjax/2.7.5/MathJax.js?config=TeX-AMS-MML_SVG">` into every figure's `text/html` output. Loading MathJax v2 onto a page that already has MathJax v3 (Jupyter Book's loader) breaks v3's typesetting — `window.MathJax` gets clobbered, `$…$` math renders as raw `\(…\)` text. Caught in PR #120.

**Convention**: every notebook that imports Plotly must monkey-patch `pio.to_html` in its setup cell:

```python
import plotly.io as pio
# Plotly's notebook_connected renderer bundles MathJax v2 via CDN.
# On the Jupyter Book site this collides with the page-level MathJax v3
# loader and breaks $…$ math rendering. Force include_mathjax=False on
# every to_html call so plotly never injects its own MathJax script —
# the page's MathJax v3 handles all math.
_orig_to_html = pio.to_html
def _to_html_no_mj(*a, **kw):
    kw["include_mathjax"] = False
    return _orig_to_html(*a, **kw)
pio.to_html = _to_html_no_mj
pio.renderers.default = "notebook_connected"

import plotly.graph_objects as go
# …rest of notebook…
```

The `pio.renderers.default = "notebook_connected"` part also keeps `plotly.min.js` loaded once from CDN per page (~70 KB), avoiding the default behavior that embeds the full 3.7 MB inline per cell.

## 3. Math syntax — `$…$` not `\(…\)`

Use `$inline$` for inline math and `$$display$$` for block math throughout the notebook source. Both are MyST-flavoured Markdown delimiters; Sphinx renders them into `\(…\)` / `\[…\]` HTML for MathJax v3 to typeset.

Avoid using `\(…\)` / `\[…\]` directly in notebook source — while functionally equivalent, they bypass MyST's preprocessing and don't get the `<span class="math notranslate nohighlight">` wrapper that the `processHtmlClass` MathJax filter expects.

## 4. Colab / Binder fallback

Every Python notebook gets a setup cell as the first cell that imports `tensor` with a fallback:

```python
try:
    import tensor  # noqa: F401
except ImportError:
    import subprocess as _sp
    _sp.run(
        ["pip", "install", "--quiet",
         "git+https://github.com/uyuutosa/tensor.git"],
        check=True,
    )
    import tensor  # noqa: F401
```

This lets the notebook open via Colab badge / Binder without a per-platform install step. After Phase 6.5 (`0.3.0`) the `git+...` path can switch to `tensor-named-axis>=0.3.0[<extras>]` from PyPI.

## 5. Sphinx source-tree limit — symlink staging

`book/_toc.yml` chapter paths are resolved relative to Sphinx's source root (`book/`). External paths starting with `../python/notebooks/…` or `../tutorials/…` won't resolve unless `book/stage.sh` is run first to symlink them under `book/`. The `deploy-book.yml` workflow runs `bash book/stage.sh` before `jupyter-book build book`; local builds need the same one-time step.

```bash
bash book/stage.sh           # idempotent — symlinks book/{tutorials,python,docs}
pip install 'jupyter-book>=0.15,<1'
jupyter-book build book      # output under book/_build/html
```

Pre-PR #116 this layer silently dropped every notebook + report chapter — the build still emitted `_build/html/{intro,architecture,roadmap}.html` but every other path 404'd on the live site. Documented under arc42 §11 R-P8.

## 6. Notebook-size sanity check

A notebook with embedded Plotly figures can balloon to 10+ MB if `include_mathjax="cdn"` (or any other inline-MathJax flag) is left enabled. Post-monkey-patch sizes (M3 of Phase 6 + Bundle B):

| Notebook                                              | Size  | Plotly figures |
| ----------------------------------------------------- | ----- | -------------- |
| `00_python-sdk-tour.ipynb`                            | 20 KB | 0              |
| `01_python-autograd.ipynb`                            | 16 KB | 0              |
| `02_python-tex.ipynb`                                 | 16 KB | 0              |
| `03_multifocal-tensors.ipynb`                         | 52 KB | 1              |
| `04_python-bundle-adjustment-perspective.ipynb`       | 88 KB | 2              |

If a notebook commits at >500 KB without dense numerical output, suspect a regression on the MathJax monkey-patch.

## 7. Cross-references

- [`CONTRIBUTING.md` §Python notebooks](../../CONTRIBUTING.md) — the same gate, summarised for contributors.
- [`docs/detailed-design/python-sdk-binding-surface.md`](../detailed-design/python-sdk-binding-surface.md) — the four nanobind boundary papercuts (sibling-rank conventions).
- [`docs/arc42/11-risks/overview.md`](../arc42/11-risks/overview.md) §5 — R-P6 (MathJax conflict), R-P7 (un-executed gate), R-P8 (Sphinx source-tree limit).
- [`book/stage.sh`](../../book/stage.sh) — the symlink staging helper.
- [`.github/workflows/notebook-ci.yml`](../../.github/workflows/notebook-ci.yml) — the validate gate that catches violations.
- [Phase 6 retrospective](../reports/2026-05-13_phase-6-python-sdk-retrospective.md) — the change-history narrative.
