---
status: Stable
owner: tensor
last-reviewed: 2026-05-13
---

# Detailed Design — `tensor` Python SDK binding surface

| Metadata     | Value                                                                                                                    |
| ------------ | ------------------------------------------------------------------------------------------------------------------------ |
| Status       | Stable                                                                                                                   |
| Type         | Detailed Design (Template-3, Module Detailed Design)                                                                     |
| Module       | `tensor` package (Python) wrapping the C++ `tensor::core` + `tensor::autograd` + `tensor::tex` Domain via nanobind        |
| Hexagonal    | **DrivingAdapter** (consumes the C++ Domain; no algebra logic in Python)                                                 |
| Anchor ADRs  | [ADR-0018](../arc42/09-decisions/0018-phase-6-python-sdk-entry-via-nanobind.md) + [ADR-0019](../arc42/09-decisions/0019-phase-6-5-runtime-backend-selection-via-extras.md) |
| Sibling DDs  | [`tensor-core.md`](./tensor-core.md), [`tensor-autograd.md`](./tensor-autograd.md), [`tensor-tex.md`](./tensor-tex.md), [`kernel-backend-port.md`](./kernel-backend-port.md) |

## 1. Purpose

Document the **nanobind ↔ idiomatic-Python boundary patterns** that the Phase 6 implementation learned by tripping on them. These are not nanobind bugs; they are conventions that don't surface until the binding is wired and tested. Recording them here keeps the next nanobind binding work (e.g. Phase 6.5's `set_backend()`) from re-paying the same papercut cost.

The audience is a contributor adding a new C++ → Python entry point or fixing a regression in an existing one. Read the [Phase 6 retrospective](../reports/2026-05-13_phase-6-python-sdk-retrospective.md) for the change-history narrative; this document is the design-decision record per pattern.

## 2. Public surface (read-only reference)

The Python public surface is pinned in `python/tests/test_smoke.py::test_public_surface_is_minimal` and listed authoritatively in [`../api-contract/python-public-surface.md`](../api-contract/python-public-surface.md). At a glance (post Phase 6 M5 + Bundle B):

- Top-level: `__version__`, `hello`, `Axis`, `DynamicShape`, `DynamicTensor`, `DynamicTensorF32`, `contract`, `from_numpy`, `autograd`, `tex`.
- `tensor.autograd`: `DynamicVariable`, `DynamicVariableF32`, `exp`, `log`, `relu`, `neg`, `sin`, `cos`, `sqrt`, `dot`, `sum_all`, `reduce_along_label`, `backward`, `sgd_update`, `zero_grad`.
- `tensor.tex`: `parse`, `to_latex`, `Expression`, `Evaluator`, `EvaluatorF32`.

Phase 6.5 adds `tensor.set_backend`, `tensor.current_backend`, `tensor.list_available_backends`.

## 3. Boundary patterns (the four papercuts)

### 3.1 `m.attr` rejects `std::string` — pass `const char*`

**Symptom**: `ImportError: bad_cast` raised at `import tensor`. Caught in PR #98.

**Why**: nanobind's `nb::module_::attr(...)` expects `nb::handle` / scalar arguments at module-init time. A `std::string` does not auto-convert in the cold-path module-init context (the Python type system isn't fully bootstrapped yet for arbitrary type coercion).

**Convention**: ALL `m.attr(...)` assignments at module init use C-string literals.

```cpp
// WRONG — std::bad_cast at import time
m.attr("__version__") = std::string{"0.1.0+dev"};

// RIGHT
m.attr("__version__") = "0.2.0";
```

If the value comes from a build-time constant (e.g. CMake-generated `PROJECT_VERSION`), pipe it through a `#define` and stringify, not through `std::string`.

### 3.2 `nb::init` lambda CTAD fails on GCC 11 — use placement-new `__init__`

**Symptom**: `error: no matching function for call to 'class_<...>::def(const char*, lambda)'` on GCC 11; Clang 15+ accepts it. Caught in PR #101 for the `Axis` type.

**Why**: `nb::init([](double x){ return Foo{x}; })` requires CTAD on `nb::init<deduced-args>` which GCC 11 fails on `auto` lambdas with template arguments. Clang's CTAD implementation is more forgiving.

**Convention**: ALL stateful constructors use the placement-new `__init__` form:

```cpp
// WRONG — GCC 11 CTAD failure
nb::class_<Axis>(m, "Axis")
    .def(nb::init([](std::string label, std::size_t extent) {
        return Axis{std::move(label), extent};
    }));

// RIGHT — placement-new, no CTAD
nb::class_<Axis>(m, "Axis")
    .def("__init__", [](Axis* self, std::string label, std::size_t extent) {
        new (self) Axis{std::move(label), extent};
    },
    nb::arg("label"), nb::arg("extent"));
```

Default constructors stay with `nb::init<>()` because there's nothing to deduce.

### 3.3 Submodules need `sys.modules` registration

**Symptom**: `import tensor.autograd` raises `ModuleNotFoundError`, even though `from tensor import autograd` works. Caught in PR #104.

**Why**: nanobind submodules (`nb::module_::def_submodule(...)`) are exposed as **attributes** of the parent module, not as Python submodules. Python's import system looks at `sys.modules["tensor.autograd"]` for the dotted import path; nanobind doesn't populate that entry.

**Convention**: `python/tensor/__init__.py` registers every nanobind submodule against its dotted name:

```python
import sys as _sys
from ._tensor_native import autograd, tex

_sys.modules[__name__ + ".autograd"] = autograd
_sys.modules[__name__ + ".tex"] = tex
```

Future submodules (`set_backend`-related plumbing in Phase 6.5) must follow the same pattern.

### 3.4 Default-constructible nanobind classes need an explicit `nb::init<>()`

**Symptom**: `TypeError: Foo() takes no arguments` when calling the default constructor from Python. Caught in PR #106 for `tex.Expression`.

**Why**: nanobind does **not** auto-expose a default constructor even when the C++ type has one. The Python type ends up with no callable form unless `.def(nb::init<>())` (or a placement-new `__init__` overload taking zero args) is registered.

**Convention**: every `nb::class_` that the user is expected to default-construct gets an explicit `nb::init<>()`:

```cpp
nb::class_<tex::Expression>(tex, "Expression")
    .def(nb::init<>())   // ← required even though Expression{} works in C++
    .def("empty", &tex::Expression::empty);
```

### 3.5 Phase 6.5 `set_backend()` boundary patterns (forward-anchor)

The Phase 6.5 implementation per [ADR-0019](../arc42/09-decisions/0019-phase-6-5-runtime-backend-selection-via-extras.md) + [Phase 6.5 impl-plan](../impl-plans/2026-05-13_phase-6-5-set-backend.md) introduces two new boundary patterns that will live in this DD's §3 section once the code lands:

#### 3.5.1 PEP-420 namespace package — no `__init__.py` in the shared root

**Convention** (planned, Phase 6.5.M1):

```
site-packages/
  tensor/                     # PEP-420 namespace package — NO __init__.py here
    _tensor_native_reference.so      # from tensor-named-axis
    _tensor_native_eigen.so          # from tensor-named-axis-eigen  (optional)
    _tensor_native_webgpu.so         # from tensor-named-axis-webgpu (optional)
    __init__.py                  # ← this lives in tensor-named-axis only
```

The `__init__.py` lives in the base distribution. Companion distributions ship `.so` files only; they share the `tensor/` directory via PEP-420.

**WRONG**: putting `__init__.py` in every distribution would shadow the base distribution's setup logic and break `set_backend()` rebinding.

#### 3.5.2 Lazy module loading via `importlib`

**Convention** (planned, Phase 6.5.M2):

```python
# In tensor/__init__.py — runs on `import tensor`:
import importlib
_AVAILABLE = {}
for _backend in ("reference", "eigen", "webgpu"):
    try:
        _mod = importlib.import_module(f"._tensor_native_{_backend}", __name__)
        _AVAILABLE[_backend] = _mod
    except ImportError:
        pass

if not _AVAILABLE:
    raise ImportError(
        "No tensor backends installed. "
        "Install with: pip install tensor-named-axis"
    )
```

Loop over the backends, catch `ImportError` per backend, populate `_AVAILABLE`. Missing-backend errors at `set_backend()` time know which backends ARE available because of this dictionary.

**WRONG**: failing fast on the first missing backend would force users to install all three even when they only need reference. The Phase 6.5 forwarding-doc [`../user-manual/how-to/use-set-backend.md`](../user-manual/how-to/use-set-backend.md) documents the expected error message.

## 4. NumPy interop

`tensor.from_numpy(arr, labels)` and `t.numpy()` both **copy** by design (per [ADR-0018](../arc42/09-decisions/0018-phase-6-python-sdk-entry-via-nanobind.md) §F). Buffer-protocol zero-copy is deferred:

- Zero-copy `numpy → DynamicTensor` would require the C++ side to honour external memory ownership semantics (lifetime extension via capsule). The Domain's existing `DynamicTensor<T>` owns its buffer; changing that would couple the Domain to a foreign lifetime model.
- Zero-copy `DynamicTensor → numpy` is technically possible via `nb::ndarray<>` with capsule deleter, but the asymmetric semantics (write-after-read on either side mutates the other) is the kind of footgun the educational frame should avoid.

Future work could ship a `tensor.from_numpy_zerocopy()` / `t.numpy_zerocopy()` pair as an explicit opt-in, but the default surface stays copy-semantics so the mental model matches what learners expect.

## 5. Plotly + Jupyter Book interaction

The Python notebooks (`python/notebooks/03_*.ipynb`, `04_*.ipynb`) that render interactive 3D figures via Plotly hit two interaction-layer constraints that aren't binding-surface decisions per se but live close enough to record:

1. **Plotly's `notebook_connected` renderer bundles MathJax v2** via CDN by default. The bundle collides with Jupyter Book's MathJax v3 page-level loader and breaks `$…$` typesetting. Workaround: monkey-patch `pio.to_html` to force `include_mathjax=False`. Documented at length in [`../design-guide/python-notebook-authoring.md`](../design-guide/python-notebook-authoring.md).
2. **Plotly's default `text/html` output embeds the full plotly.min.js (~3.7 MB)** per cell. Setting `pio.renderers.default = "notebook_connected"` keeps plotly.js loaded once from CDN per page — notebook size drops 100×+.

Both workarounds are in the setup cell of every plotly-bearing notebook. The CI gate from PR #118 prevents the related "notebook committed un-executed" failure mode.

## 6. Test surface

| Test file                                  | Covers                                                              |
| ------------------------------------------ | ------------------------------------------------------------------- |
| `python/tests/test_smoke.py`               | `__version__` present; `hello()` returns the expected string; public-surface set is exactly the M5 + Bundle B baseline. |
| `python/tests/test_arithmetic.py`          | M2 — `DynamicTensor` + arithmetic with Einstein-style broadcast.   |
| `python/tests/test_contract_numpy.py`      | M3 — `contract` + NumPy interop round-trip.                        |
| `python/tests/test_autograd.py`            | M4 — autograd primitives + linear-regression convergence exit.     |
| `python/tests/test_autograd_extensions.py` | Bundle B — `sin`/`cos`/`sqrt`/`__truediv__`/`reduce_along_label`.  |
| `python/tests/test_tex.py`                 | M5 — `parse` + round-trip + `Evaluator`.                           |
| `python/tests/test_set_backend.py` (planned, Phase 6.5) | M2 of Phase 6.5 — runtime backend switching + missing-backend error. |

All Python tests cross-validate against either the C++ canonical output (where applicable) or NumPy (for general numerical agreement). Tolerances: `1e-12` for `double`, `1e-5` for `float`.

## 7. Future work (incomplete)

- **Phase 6.5 `set_backend()`** — per [ADR-0019](../arc42/09-decisions/0019-phase-6-5-runtime-backend-selection-via-extras.md). Three companion distributions under a PEP-420 namespace package; runtime switching between installed backends.
- **Zero-copy buffer protocol** — opt-in `from_numpy_zerocopy()` / `numpy_zerocopy()` once a clear use case lands.
- **Type stubs** — `tensor/py.typed` + `.pyi` stubs for IDE completion. Deferred until first user feedback signals friction.
- **PyPy support** — currently skipped in `[tool.cibuildwheel] skip = "*-musllinux_*"` plus implicit no-PyPy via `CIBW_BUILD = "cp39-*"…`. Reconsider if a community signal asks for it.

## 8. Cross-references

- [`tensor-core.md`](./tensor-core.md) — the C++ Domain centerpiece this SDK wraps.
- [`tensor-autograd.md`](./tensor-autograd.md) — the C++ autograd module the Python `tensor.autograd` mirrors.
- [`tensor-tex.md`](./tensor-tex.md) — the C++ `_tex` module the Python `tensor.tex` mirrors.
- [`kernel-backend-port.md`](./kernel-backend-port.md) — the port that `set_backend()` selects at runtime.
- [Phase 6 retrospective](../reports/2026-05-13_phase-6-python-sdk-retrospective.md) — narrative source for the four boundary papercuts.
- [Phase 6.5 impl-plan](../impl-plans/2026-05-13_phase-6-5-set-backend.md) — the upcoming work that this DD anchors.
