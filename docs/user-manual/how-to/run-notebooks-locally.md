---
status: Stable
owner: tensor
last-reviewed: 2026-05-13
---

# How to — run the Python notebooks locally

This guide walks through getting the `python/notebooks/00`–`04` notebooks running on your laptop. Total time on a clean machine is ~5 minutes (most of it the nanobind compile).

If you only want to *read* the notebooks, the rendered HTML is at <https://uyuutosa.github.io/tensor/> — no install needed. If you want to *modify and re-execute*, follow this guide.

## 1. Prerequisites

- Python ≥ 3.9 (we test 3.9–3.13).
- A C++20 compiler: GCC ≥ 11 / Clang ≥ 15 / MSVC ≥ 19.30 / AppleClang ≥ 14.
- CMake ≥ 3.25.
- Ninja (or pass `--config-settings=cmake.generator=...` to use Make).
- Network access to PyPI for `nanobind`, `scikit-build-core`, `numpy`, `plotly`, `jupyter`.

## 2. Clone + build

```bash
git clone https://github.com/uyuutosa/tensor.git
cd tensor
pip install -e . -v
```

What `pip install -e . -v` does:

- Reads [`pyproject.toml`](../../../pyproject.toml).
- Triggers `scikit-build-core` to invoke CMake against [`python/CMakeLists.txt`](../../../python/CMakeLists.txt).
- CMake calls `nanobind_add_module(_tensor_native ...)` which compiles the C++20 sources under `python/src/` against the headers in `include/tensor/`.
- The compiled `_tensor_native.so` lands inside `python/tensor/` for the editable install.

Cold-build time: 2–4 minutes on a modern laptop. Subsequent edits to `python/src/_tensor_native.cpp` trigger an incremental rebuild via `pip install -e .` (or `cmake --build python/build`).

Smoke test:

```bash
python -c "import tensor; print(tensor.__version__); print(tensor.hello())"
# → 0.2.0  (or whatever the current version is)
# → hello from tensor::core
```

## 3. Install notebook deps

```bash
pip install jupyter nbconvert numpy plotly matplotlib
```

The first two are for execution; `plotly` is for the 3D figures in `03_multifocal-tensors.ipynb` and `04_python-bundle-adjustment-perspective.ipynb`; `matplotlib` for the loss curves.

## 4. Execute a notebook

```bash
jupyter nbconvert --to notebook --inplace --execute \
    --ExecutePreprocessor.timeout=900 \
    python/notebooks/04_python-bundle-adjustment-perspective.ipynb
```

`--inplace --execute` runs the notebook and writes the executed JSON back to the same file. `--ExecutePreprocessor.timeout=900` allows 15 min per cell — relevant for the MVG notebook 03 which runs autograd loops.

Or open it interactively:

```bash
jupyter lab python/notebooks/
```

Then run cells with `Shift+Enter`.

## 5. Build the Jupyter Book locally (optional)

If you want to preview how your changes will look on the published site:

```bash
bash book/stage.sh                # one-off: symlinks ../{tutorials,python,docs} into book/
pip install 'jupyter-book>=0.15,<1'
jupyter-book build book           # output under book/_build/html/
xdg-open book/_build/html/index.html   # or `open` on macOS
```

The `stage.sh` step is required because Sphinx's source root is `book/` and the TOC references `../python/notebooks/...` paths that lie outside that root. Documented in [`book/stage.sh`](../../../book/stage.sh) and the [`python-notebook-authoring`](../../design-guide/python-notebook-authoring.md) design guide.

## 6. Committing executed notebooks

If you edit a notebook in `python/notebooks/`, you must commit it executed:

```bash
jupyter nbconvert --to notebook --inplace --execute \
    --ExecutePreprocessor.timeout=900 \
    python/notebooks/<file>.ipynb
git add python/notebooks/<file>.ipynb
git commit -m "<conventional-commits message>"
```

The notebook-output gate (`.github/workflows/notebook-ci.yml` `validate` job) fails CI if any `python/notebooks/*.ipynb` has `execution_count: None` or zero outputs — see the [`python-notebook-authoring`](../../design-guide/python-notebook-authoring.md) design guide for the full rationale.

## 7. Troubleshooting

| Symptom                                                                              | Cause / fix                                                                                                                          |
| ------------------------------------------------------------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------ |
| `pip install -e .` fails with `error: Microsoft Visual C++ 14.0 or greater is required`. | Install [Visual Studio Build Tools 2022](https://visualstudio.microsoft.com/downloads/?q=build+tools) with the "Desktop development with C++" workload. |
| `pip install -e .` fails with `CMake was unable to find a build program corresponding to "Ninja"`. | `pip install ninja` (pip-shipped Ninja is on PATH for the current venv).                                                              |
| `import tensor` raises `ModuleNotFoundError: No module named 'tensor.autograd'`.     | Re-run `pip install -e .` after pulling `develop`; PR #104 fixed this via `sys.modules` registration but a stale install can lag.    |
| Plotly figures don't appear in a re-executed notebook.                              | Likely the MathJax-v2 trap. Verify the setup cell sets `pio.to_html = _no_mj`-style monkey-patch (`03` and `04` already do; copy from them when adding new plotly notebooks). |
| Jupyter Book build emits "WARNING: toctree contains reference to nonexisting document 'python/notebooks/...'". | `bash book/stage.sh` first.                                                                                                          |

## 8. After Phase 6.5 — the install path collapses

Once the maintainer publishes `0.3.0` to PyPI, this whole guide collapses to:

```bash
pip install tensor-named-axis[all]       # or [eigen] / [webgpu]
jupyter lab path/to/your/notebooks/
```

No C++ toolchain required, no source clone. The `pip install -e .` flow above remains the right approach when you're actively modifying the C++ Domain or the Python bindings.

## 9. Cross-references

- [`docs/design-guide/python-notebook-authoring.md`](../../design-guide/python-notebook-authoring.md) — the deeper "why" + edge cases.
- [`README.md`](../../../README.md) — the project-wide "Try it without installing" Colab + Binder badges.
- [`docs/detailed-design/python-sdk-binding-surface.md`](../../detailed-design/python-sdk-binding-surface.md) — what's actually being compiled.
- [`pyproject.toml`](../../../pyproject.toml) — the project metadata.
