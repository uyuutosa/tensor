---
status: Active
owner: tensor
date: 2026-05-12
type: Layer B implementation plan
---

# Phase 6 — Python SDK (2026-05-12)

> Brings the Python SDK forward from "Phase 6, 最後で良い" (Axis H, 2026-05-11) to **first post-`0.1.0` phase** per maintainer 2026-05-12 directive. Decisions captured in [ADR-0018](../arc42/09-decisions/0018-phase-6-python-sdk-entry-via-nanobind.md): nanobind + in-tree `python/` + scikit-build-core + individual PyPI identity + conda-forge + minimum-viable surface.

| Metadata     | Value                                                                            |
| ------------ | -------------------------------------------------------------------------------- |
| Layer        | B (volatile working material — dated, append-only)                              |
| Predecessor  | [`docs/reports/2026-05-11_open-discussion-points.md`](../reports/2026-05-11_open-discussion-points.md) §H |
| Anchor ADR   | [ADR-0018](../arc42/09-decisions/0018-phase-6-python-sdk-entry-via-nanobind.md) |
| Target window | 2026-05-13 → 2026-07 (six milestones; ~one PR per milestone)                    |
| Exit         | `0.2.0` tag — first PyPI publish (`pip install tensor-named-axis`) + conda-forge submission |

---

## Milestones

The order is dependency-driven; each milestone exits when its tests pass on Linux + macOS + Windows.

### P6.M1 — Scaffold + smoke binding

**Goal**: prove the toolchain works. End state: `pip install -e .` builds; `import tensor` succeeds; `tensor.__version__` returns "0.1.0+post6.m1.dev".

**Deliverables**:

- `python/` subdirectory at repo root.
- `pyproject.toml` at repo root naming `tensor` (or `tensor-named-axis` if PyPI taken — check on M1 spike) with scikit-build-core backend, declaring `nanobind>=2.0`, `scikit-build-core>=0.10` build deps, Python ≥ 3.9.
- `python/CMakeLists.txt` finds nanobind, builds one extension module `_tensor_native`.
- `python/src/_tensor_native.cpp` — minimal nanobind boilerplate exposing `__version__` and a `hello() -> str` function.
- `python/tensor/__init__.py` re-exports.
- `python/tests/test_smoke.py` — `assert tensor.hello() == "hello from tensor::core"`.
- Update arc42 §5 building-blocks: new row `python/` classified as DrivingAdapter (P6 introduced).
- Update C4 `workspace.dsl`: new `pythonSdk` container.

**Exit criteria**: GitHub Actions `python-wheel-smoke` job (new) builds + tests on `ubuntu-latest` (CPython 3.11 minimum).

**Effort**: S (≤ 2 hr).

### P6.M2 — `DynamicTensor` + arithmetic with Einstein-style broadcast

**Goal**: the headline named-axis feature in Python.

**Deliverables**:

- nanobind bindings for `tensor::core::Axis`, `tensor::core::DynamicShape`, `tensor::core::DynamicTensor<double>` + `DynamicTensor<float>`.
- Python: `t = tensor.DynamicTensor.zeros(tensor.DynamicShape([tensor.Axis("i", 3), tensor.Axis("j", 2)]))` works.
- Operators: `__add__`, `__sub__`, `__mul__`, `__truediv__` dispatch through `tensor::core::DynamicTensor::operator+` etc. Named-axis broadcast verified.
- `__repr__` / `__str__` matching the C++ `format.hpp` output.
- `python/tests/test_arithmetic.py` — round-trips 4-5 cases from `tutorials/00_intro.ipynb` §2 (e.g. `a_i + b_j → c_{ij}`).

**Exit**: cross-validate one outer product + one same-axis broadcast against the C++ test suite's expected values, within `1e-12` for `double`.

**Effort**: M (≤ 1 day).

### P6.M3 — `contract` + NumPy interop

**Goal**: the named-axis contraction primitive accessible from Python; `numpy.asarray(t)` and `tensor.from_numpy(arr, labels=("i","j"))` work.

**Deliverables**:

- Bind `tensor::core::contract` as `tensor.contract(a, b)` (Einstein-sum over shared labels).
- nanobind buffer protocol on `DynamicTensor` (zero-copy view into NumPy where possible; explicit copy otherwise).
- `tensor.from_numpy(arr: np.ndarray, labels: Tuple[str, ...]) -> DynamicTensor`.
- `python/tests/test_numpy_interop.py` — round-trip `np.ndarray ↔ DynamicTensor`, equality with `np.einsum` for representative contractions.

**Exit**: `np.einsum("ij,jk->ik", A, B)` equals `tensor.contract(tA, tB)` element-wise where `tA.labels == ("i","j")` and `tB.labels == ("j","k")`.

**Effort**: M (≤ 1 day).

### P6.M4 — Autograd

**Goal**: tape-based reverse-mode autograd usable from Python.

**Deliverables**:

- nanobind bindings for `tensor::autograd::DynamicVariable<double>` / `DynamicVariable<float>`.
- Python: `v = tensor.autograd.DynamicVariable(t)`, `loss = (v * v).sum()`, `loss.backward()`, `v.grad` → DynamicTensor.
- Bind `tensor::autograd::gradient_check` as `tensor.autograd.gradient_check(fn, vars)`.
- Tape lifecycle: `with tensor.autograd.tape(): ...` context manager wrapping the thread_local tape.
- `python/tests/test_autograd.py` — gradient_check passes on 5 representative expressions (matching `tests/test_autograd_*.cpp` smoke cases).

**Exit**: a one-liner MLP-on-toy training loop (mirroring `tutorials/07_mlp-on-toy.ipynb` §4 in Python) converges to the same `W ≈ 2, b ≈ 1` within `1e-3`.

**Effort**: M-L (1-2 days).

### P6.M5 — `tensor::tex::Evaluator` + Python `_tex`-equivalent helper

**Goal**: "the formula is the program" reaches Python.

**Deliverables**:

- Bind `tensor::tex::parse(s) -> Expression` + `tensor::tex::Evaluator<T>` (bind tensors → evaluate to `DynamicTensor`).
- Python helper: `tensor.tex(r"c_{ij} = a_i + b_j")` parses + returns an `Evaluator`-ready handle; `tensor.tex(r"...").bind(a=..., b=...).evaluate()` yields a `DynamicTensor`. (Python has no UDLs; the helper is the equivalent.)
- `python/tests/test_tex.py` — round-trip parse → to_latex; binding + evaluation matches `tutorials/01_formula-is-the-program.ipynb` §4 / §5 outputs.

**Exit**: outer product + Einstein-sum examples from notebook 01 reproduce identically in Python.

**Effort**: M (≤ 1 day).

### P6.M6 — Backend selection + cross-validation suite + `0.2.0` release

**Goal**: Hexagonal-lite payoff visible to Python users; ship-ready.

**Deliverables**:

- `tensor.set_backend("reference" | "eigen" | "webgpu")` at module level — selects the linked `KernelBackend` adapter at runtime (the Python module loads three pre-built `_tensor_native_{reference,eigen,webgpu}.so` and dispatches via a Python-side switch). *Note*: this changes the C++-side `TENSOR_KERNEL_BACKEND` configure-time choice into a Python-side runtime choice; the build packages all three.
- `python/tests/test_backend_parity.py` — same input through each backend yields the same output within `1e-5` (`float`) / `1e-12` (`double`).
- conda-forge submission scaffold: `recipe/meta.yaml` (in a separate `tensor-feedstock` clone, tracked via a follow-up impl-plan).
- PyPI publishing: GitHub Action workflow `python-publish.yml` triggered on tag, uploads wheels for {Linux × {x86_64, aarch64}} ∪ {macOS × {x86_64, arm64}} ∪ {Windows × x86_64} × Python {3.9, 3.10, 3.11, 3.12, 3.13}.

**Exit**: `0.2.0` tag on `main` with the same release ceremony as `0.1.0`; first PyPI publish; conda-forge submission opened.

**Effort**: L (2-3 days).

---

## Out of scope (recap from ADR-0018)

- **Compile-time-NTTP types** (`TypedTensor<T, "i", "j">`, `LabelTag<S>`) — Python is rank-erased; no clean way to surface.
- **Static-rank `Tensor<T, N>`** — same reason as above; users see `DynamicTensor` only.
- **Phase 5 `tensor::linalg` shim** — independent track; can land in parallel.
- **Performance benchmarks** — the perf story for the Python surface mirrors the C++ measurements ([2026-05-11 perf comparison report](../reports/2026-05-11_backend-performance-comparison.md)); no new bench infrastructure.
- **GPU kernel JITting from Python** — the `webgpu` backend dispatches the same C++-side kernels; no Python-side WGSL editing.

---

## CI matrix changes

Phase 6 adds one new dimension (Python version × OS). The combined matrix once Phase 6 lands:

- **C++ side** — unchanged: Linux / macOS / Windows × Debug / Release × reference / eigen / webgpu.
- **Python wheels** — Linux × {x86_64, aarch64} + macOS × {x86_64, arm64} + Windows × x86_64, all × CPython {3.9, 3.10, 3.11, 3.12, 3.13}. Use `cibuildwheel`.
- **Python tests** — one ubuntu-latest job running pytest on the editable install.

Existing jobs (`ci.yml`, `notebook-ci.yml`, `lyx-export-ci.yml`, `deploy-book.yml`) are untouched.

---

## Risks

- **R-P1 — nanobind 2.x API churn**: nanobind is mature but still evolving. Pin `nanobind>=2.0,<3` in `pyproject.toml`; revisit at half-yearly bibliography audit (next: 2026-11-11).
- **R-P2 — Stable ABI maturity**: relying on Stable ABI (Python 3.12+) trades small per-version perf overhead for one wheel covering 3.12+. Acceptable per [ADR-0001](../arc42/09-decisions/0001-pivot-to-educational-named-axis-dsl.md) clarity > performance.
- **R-P3 — PyPI package name collision**: `tensor` is likely taken. M1 spike confirms or proposes `tensor-named-axis` / `named-tensor-cpp`.
- **R-P4 — `set_backend()` packaging**: distributing three pre-built `.so` per wheel inflates wheel size (each ~5-15 MB). Acceptable for the educational headline; if it becomes a problem, fall back to one-backend-per-wheel with `tensor[eigen]` / `tensor[webgpu]` extras.

---

## Sequencing relative to other work

| Phase | Status (2026-05-12) | Relation to Phase 6 |
| ----- | -------------------- | ------------------- |
| Phase 4 | ✅ closed (PRs #91 / #92 / tag `0.1.0` / Pages live) | predecessor |
| **Phase 6** | **active (this plan)** | next |
| Phase 5 (`tensor::linalg` shim) | unblocked but paused — `<linalg>` not shipped | parallel-able; no dependency from Phase 6 |
| Bibliography audit | first audit 2026-11-11 | independent |

---

## Cross-references

- Anchor ADR: [ADR-0018](../arc42/09-decisions/0018-phase-6-python-sdk-entry-via-nanobind.md).
- Directional capture: [`docs/reports/2026-05-11_open-discussion-points.md`](../reports/2026-05-11_open-discussion-points.md) §H.
- Landscape signal: [`docs/reports/2026-05-12_landscape-recheck-and-adversarial-review.md`](../reports/2026-05-12_landscape-recheck-and-adversarial-review.md) §A.5 (no first-class named-axis tensor exists in production Python ML — the wedge this phase opens).
- `KernelBackend` port (what the Python SDK consumes): [`docs/detailed-design/kernel-backend-port.md`](../detailed-design/kernel-backend-port.md).
- The C++ surfaces the Python SDK mirrors: [`docs/detailed-design/tensor-core.md`](../detailed-design/tensor-core.md), [`docs/detailed-design/tensor-autograd.md`](../detailed-design/tensor-autograd.md), [`docs/detailed-design/tensor-tex.md`](../detailed-design/tensor-tex.md).
