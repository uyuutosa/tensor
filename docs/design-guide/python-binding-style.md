---
status: Stable
owner: tensor
last-reviewed: 2026-05-14
---

# Python binding style — nanobind conventions for `tensor`

> The conventions for adding new entry points to the Python SDK via nanobind. The HOW per binding-call lives in [`../detailed-design/python-sdk-binding-surface.md`](../detailed-design/python-sdk-binding-surface.md); this file is the contributor-facing style rulebook.

## 1. Where to bind

- All nanobind code lives in [`python/src/_tensor_native.cpp`](../../python/src/_tensor_native.cpp).
- Module structure mirrors the C++ namespace structure: `tensor::core` → top-level Python, `tensor::autograd` → `autograd` submodule, `tensor::tex` → `tex` submodule.
- New bindings are added to the same file unless the file exceeds ~1500 lines, at which point split by submodule into `_tensor_native_core.cpp`, etc.

## 2. The four boundary patterns

Repeated here from [`../detailed-design/python-sdk-binding-surface.md` §3](../detailed-design/python-sdk-binding-surface.md) because contributors will read this file before the DD.

### 2.1 `m.attr(...)` takes `const char*`, not `std::string`

```cpp
// RIGHT
m.attr("__version__") = "0.2.0";

// WRONG (std::bad_cast at import time)
m.attr("__version__") = std::string{"0.2.0"};
```

### 2.2 `__init__` is placement-new on GCC 11

```cpp
// RIGHT — GCC 11-friendly placement-new form
nb::class_<Axis>(m, "Axis")
    .def("__init__", [](Axis* self, std::string label, std::size_t extent) {
        new (self) Axis{std::move(label), extent};
    },
    nb::arg("label"), nb::arg("extent"));

// WRONG — GCC 11 CTAD failure on the auto lambda
nb::class_<Axis>(m, "Axis")
    .def(nb::init([](std::string label, std::size_t extent) {
        return Axis{std::move(label), extent};
    }));
```

### 2.3 Submodules register in `sys.modules`

```python
# python/tensor/__init__.py
import sys as _sys
from ._tensor_native import autograd, tex

_sys.modules[__name__ + ".autograd"] = autograd
_sys.modules[__name__ + ".tex"] = tex
```

### 2.4 Default-constructible classes need an explicit `nb::init<>()`

```cpp
// RIGHT
nb::class_<tex::Expression>(tex, "Expression")
    .def(nb::init<>())
    .def("empty", &tex::Expression::empty);
```

## 3. Argument naming

- **`nb::arg("name")` is required**, not optional. Every parameter gets a Python-side keyword name.
- **Snake_case for parameters**, matching C++ side: `nb::arg("axis"), nb::arg("extent")`.
- **`nb::arg().none(false)`** to reject `None` for parameters that don't have a default. Improves the error message.

## 4. Type conversion

- **NumPy interop**: `from_numpy(arr, labels)` copies. Do NOT add a zero-copy variant without an ADR (per [ADR-0018 §F](../arc42/09-decisions/0018-phase-6-python-sdk-entry-via-nanobind.md)).
- **`std::string` ↔ `str`**: nanobind handles this; no manual conversion needed for function args.
- **`std::vector<T>` ↔ `list[T]`**: nanobind handles this for primitive `T`; for custom `T` you may need `nb::make_value_caster<T>()`.
- **Numeric types**: `double` ↔ `float`; `float` ↔ `float`. Python int → C++ size_t is silent (clamps at runtime if out of range — guard at the C++ side).

## 5. Submodule layout

Each submodule has its own scope:

```cpp
NB_MODULE(_tensor_native, m) {
    m.doc() = "Phase 6 Python SDK — wraps tensor::core, ::autograd, ::tex";
    m.attr("__version__") = "0.2.0";

    bind_core(m);   // binds top-level Axis, DynamicShape, DynamicTensor, contract, etc.

    auto autograd = m.def_submodule("autograd");
    bind_autograd(autograd);

    auto tex = m.def_submodule("tex");
    bind_tex(tex);
}
```

Each `bind_<submodule>` function lives in its own helper section of `_tensor_native.cpp` (or in a split file). The submodule docstring goes on `m.doc()` of the submodule.

## 6. Test surface

Every new binding has:

1. A line added to [`../api-contract/python-public-surface.md`](../api-contract/python-public-surface.md) under the matching §2-§4.
2. A line added to `python/tests/test_smoke.py::test_public_surface_is_minimal`'s expected set.
3. At least one cross-validation test in `python/tests/test_<surface>.py` that asserts the Python output agrees with the C++ canonical answer within QO-4 tolerance.

All three are required in the same PR — the smoke test enforces #1 and #2 together.

## 7. Plotly + Jupyter Book interaction (notebook authoring)

This is technically not a binding concern but bites contributors adding plotly notebooks. See [`./python-notebook-authoring.md` §2](./python-notebook-authoring.md) for the MathJax v2 monkey-patch. Failure to apply it in a notebook that imports plotly breaks `$…$` rendering on the published site.

## 8. Error messages

Python-side errors include the fix per [arc42 §8 §5.2 errors-as-docs](../arc42/08-crosscutting/overview.md). Examples:

```python
RuntimeError: webgpu backend is not installed.
Install with:  pip install tensor-named-axis[webgpu]
Or install all backends:  pip install tensor-named-axis[all]
Currently available: ['reference']
```

```python
RuntimeError: subscript count mismatch for 'a': expected 2, got 1
```

No `RuntimeError("something went wrong")` — every error message names the failing condition and points at the fix.

## 9. When NOT to add a Python binding

- **Compile-time-NTTP types** (`Tensor<T, N>`, `TypedTensor<T, ...>`, `LabelTag<S>`). These don't have meaningful Python equivalents because Python is rank-erased. [ADR-0018 §F](../arc42/09-decisions/0018-phase-6-python-sdk-entry-via-nanobind.md) records this as out-of-scope.
- **Implementation-detail types** (`BroadcastPlan`, `ContractPlan`, internal helpers). Users don't need to construct these; the public functions consume them internally.
- **Adapter-specific types** (`tensor::core::backend::reference::Backend`, etc.). Users select adapters via the planned `tensor.set_backend(name)` API, not by constructing backend instances directly.

## 10. Cross-references

- [`../detailed-design/python-sdk-binding-surface.md`](../detailed-design/python-sdk-binding-surface.md) — the four boundary patterns with WRONG/RIGHT code.
- [`../api-contract/python-public-surface.md`](../api-contract/python-public-surface.md) — the symbol contract.
- [`./python-notebook-authoring.md`](./python-notebook-authoring.md) — notebook-side conventions.
- [`./cpp-style-guide.md`](./cpp-style-guide.md) — the C++ side of the binding.
- [ADR-0018](../arc42/09-decisions/0018-phase-6-python-sdk-entry-via-nanobind.md), [ADR-0019](../arc42/09-decisions/0019-phase-6-5-runtime-backend-selection-via-extras.md).
- nanobind documentation: <https://nanobind.readthedocs.io/>.
