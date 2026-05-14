"""tensor — Python SDK for the named-axis differentiable C++ tensor library.

The Python surface is a DrivingAdapter (per ADR-0009 + ADR-0018) consuming
the same C++ Domain (`tensor::core` + `tensor::autograd` + `tensor::tex`)
that the C++ tests and tutorials exercise.

Phase 6 status (2026-05-13):

- P6.M1 ✅ — scaffold + smoke binding.
- P6.M2 ✅ — `Axis` + `DynamicShape` + `DynamicTensor` (float64) +
  `DynamicTensorF32` (float32) with the four arithmetic operators
  under Einstein-style label broadcast.
- P6.M3 ✅ — `contract` (named-axis Einstein-sum) + NumPy interop
  (`from_numpy(arr, labels)` + `t.numpy()`).
- P6.M4 ✅ — `tensor.autograd` submodule: `DynamicVariable`, arithmetic +
  activations (`exp` / `log` / `relu` / `neg`), `dot`, `sum_all`,
  `backward`, `sgd_update`.
- P6.M5 ✅ — `tensor.tex` submodule: `parse(s) -> Expression`, `to_latex(expr)`,
  `Evaluator` / `EvaluatorF32` (bind tensors to AST leaves and evaluate).
- Bundle B ✅ — autograd surface extensions: `sin` / `cos` / `sqrt`,
  `__truediv__` on `DynamicVariable`, `reduce_along_label`.
- P6.M6 ✅ — release-prep: cibuildwheel matrix + PyPA trusted-publishing.

Phase 6.5 status (2026-05-14):

- P6.5.M1 ✅ — multi-backend build pipeline: ``TENSOR_KERNEL_BACKEND``
  env var routes the build into ``_tensor_native_{reference,eigen,webgpu}.so``.
  The reference build still produces ``_tensor_native.so`` for backward
  compatibility.
- P6.5.M2 ✅ — ``tensor.set_backend(name)`` / ``current_backend()`` /
  ``list_available_backends()`` with lazy ``importlib`` loading of all
  installed backends + rebinding of top-level exports.
- P6.5.M3-M4 🚧 — cibuildwheel extras pipeline + ``0.3.0`` release
  ceremony (planned).

See ``docs/impl-plans/2026-05-13_phase-6-5-set-backend.md`` and
``docs/arc42/09-decisions/0019-phase-6-5-runtime-backend-selection-via-extras.md``
for the Phase 6.5 details.

Example
-------
>>> import tensor
>>> a = tensor.DynamicTensor(
...     tensor.DynamicShape([tensor.Axis("i", 5)]),
...     [1.0, 2.0, 3.0, 4.0, 5.0],
... )
>>> b = tensor.DynamicTensor(
...     tensor.DynamicShape([tensor.Axis("j", 5)]),
...     [1.0, 2.0, 3.0, 4.0, 5.0],
... )
>>> c = a + b
>>> c.shape.rank()
2
>>> # c is a 5×5 outer-sum table — c_{ij} = a_i + b_j.

>>> import tensor
>>> tensor.list_available_backends()  # what's installed
['reference']
>>> tensor.current_backend()
'reference'
>>> # tensor.set_backend("webgpu")
>>> # RuntimeError: webgpu backend is not installed. ...
"""

import importlib as _importlib
import sys as _sys
from typing import Literal as _Literal

# ─── Phase 6.5 — multi-backend discovery ────────────────────────────
# Probe every backend the wheel matrix could ship. Each successful
# import lands the matching native module in `_AVAILABLE_BACKENDS`.
# The reference backend's compiled extension is named `_tensor_native`
# (Phase 6 backward-compatible name); the eigen / webgpu wheels ship
# `_tensor_native_eigen.so` / `_tensor_native_webgpu.so` (Phase 6.5).

_BACKEND_MODULE_NAMES = {
    "reference": "_tensor_native",
    "eigen": "_tensor_native_eigen",
    "webgpu": "_tensor_native_webgpu",
}

_BackendName = _Literal["reference", "eigen", "webgpu"]

_AVAILABLE_BACKENDS: dict = {}
for _name, _mod_name in _BACKEND_MODULE_NAMES.items():
    try:
        _AVAILABLE_BACKENDS[_name] = _importlib.import_module(
            f".{_mod_name}", __name__
        )
    except ImportError:
        pass

if not _AVAILABLE_BACKENDS:
    raise ImportError(
        "No tensor backends are installed. The reference backend is "
        "always shipped with `tensor-named-axis`; this error means the "
        "wheel's native extension failed to load. Try:\n"
        "  pip install --force-reinstall tensor-named-axis"
    )

# Default: the first backend the wheel matrix made available. Reference
# is always available in a well-formed install; if the user only has
# `[eigen]` somehow, it picks `eigen` instead of failing.
_DEFAULT_BACKEND: _BackendName = (
    "reference" if "reference" in _AVAILABLE_BACKENDS
    else next(iter(_AVAILABLE_BACKENDS))
)
_CURRENT_BACKEND: _BackendName = _DEFAULT_BACKEND


def _bind_to(backend: _BackendName) -> None:
    """Rebind the top-level public surface against the named backend's module."""
    mod = _AVAILABLE_BACKENDS[backend]

    # Rebind each public symbol against the active backend's module.
    g = globals()
    for _sym in (
        "__version__",
        "Axis",
        "DynamicShape",
        "DynamicTensor",
        "DynamicTensorF32",
        "contract",
        "from_numpy",
        "autograd",
        "tex",
    ):
        g[_sym] = getattr(mod, _sym)

    # `hello` is bound separately because `hello()` at the top of this
    # module is the Python-side wrapper; we keep the wrapper but route
    # it to the active backend's native `hello`.
    g["_native_hello"] = mod.hello

    # nanobind submodules are *attributes* of the parent module, not
    # importable as Python submodules without explicit registration.
    # Re-register every time `set_backend()` switches so the dotted
    # `import tensor.autograd` / `import tensor.tex` keep pointing at
    # the active backend's submodule.
    _sys.modules[__name__ + ".autograd"] = mod.autograd
    _sys.modules[__name__ + ".tex"] = mod.tex


# Activate the default backend at import time.
_bind_to(_DEFAULT_BACKEND)


def set_backend(name: _BackendName) -> None:
    """Switch the active `KernelBackend` adapter at runtime.

    Phase 6.5 (ADR-0019). Selects between *installed* backends only; the
    base install ships only ``reference`` (5 MB wheel). Add Eigen and / or
    WebGPU via ``pip install tensor-named-axis[eigen]`` / ``[webgpu]`` /
    ``[all]``.

    Subsequent ``tensor.DynamicTensor(...)`` constructions resolve to the
    new backend's implementation. Tensors built before the switch keep
    their construction-time backend; mid-computation rebinding is
    undefined (per [ADR-0019]).

    Parameters
    ----------
    name
        One of ``"reference"`` / ``"eigen"`` / ``"webgpu"``.

    Raises
    ------
    RuntimeError
        If ``name`` is not in ``list_available_backends()``. The error
        message includes the exact ``pip install`` command for the
        missing backend.

    Examples
    --------
    >>> import tensor
    >>> tensor.list_available_backends()  # doctest: +SKIP
    ['reference', 'eigen']
    >>> tensor.set_backend('eigen')  # doctest: +SKIP
    >>> tensor.current_backend()  # doctest: +SKIP
    'eigen'
    """
    if name not in _AVAILABLE_BACKENDS:
        installed = sorted(_AVAILABLE_BACKENDS.keys())
        raise RuntimeError(
            f"{name} backend is not installed.\n"
            f"Install with:  pip install tensor-named-axis[{name}]\n"
            f"Or install all backends:  pip install tensor-named-axis[all]\n"
            f"Currently available: {installed}"
        )
    global _CURRENT_BACKEND
    _CURRENT_BACKEND = name
    _bind_to(name)


def current_backend() -> str:
    """Return the name of the currently active backend.

    Examples
    --------
    >>> import tensor
    >>> tensor.current_backend()
    'reference'
    """
    return _CURRENT_BACKEND


def list_available_backends() -> list:
    """Return the list of backends installed in this Python environment.

    The base ``tensor-named-axis`` wheel ships only ``reference``. The
    extras (``[eigen]`` / ``[webgpu]`` / ``[all]``) install companion
    distributions that share the ``tensor/`` PEP-420 namespace package
    and add their backend to this list.

    Returns
    -------
    list[str]
        Sorted backend names — a subset of
        ``["reference", "eigen", "webgpu"]``.

    Examples
    --------
    >>> import tensor
    >>> tensor.list_available_backends()  # doctest: +SKIP
    ['reference']
    """
    return sorted(_AVAILABLE_BACKENDS.keys())


def hello() -> str:
    """Return a fixed greeting from the C++ side.

    P6.M1 smoke surface kept around for "did the install work?" diagnostics.

    Returns
    -------
    str
        Always ``"hello from tensor::core"``.

    Examples
    --------
    >>> import tensor
    >>> tensor.hello()
    'hello from tensor::core'
    """
    return _native_hello()


__all__ = [
    "__version__",
    "hello",
    "Axis",
    "DynamicShape",
    "DynamicTensor",
    "DynamicTensorF32",
    "contract",
    "from_numpy",
    "autograd",
    "tex",
    "set_backend",
    "current_backend",
    "list_available_backends",
]
