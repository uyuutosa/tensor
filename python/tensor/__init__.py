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
- P6.5.M2 ✅ — ``tensor.set_backend(name)`` / ``current_backend()`` /
  ``list_available_backends()`` Python surface.
- P6.5.M3 ✅ — cibuildwheel matrix grows to 3 backends × 4 OS/arch ×
  5 CPython = ~55 wheels per release; PEP-508 extras
  (``tensor-named-axis[eigen]`` / ``[webgpu]`` / ``[all]``) pull in the
  matching companion distribution.

  **Known limitation per R-P6.5.5**: nanobind 2.x's type registry is
  global per process. Loading more than one ``_tensor_native_*.so`` in
  the same Python process produces duplicate-registration warnings and
  partial bindings. As a workaround, this module loads exactly ONE
  backend per process (preference: ``TENSOR_BACKEND`` env var >
  reference > eigen > webgpu). ``set_backend()`` to a different
  installed backend raises with instructions to restart the process
  with the env var set. Future work tracked as a Phase 6.5 follow-up.

- P6.5.M4 🚧 — ``0.3.0`` release ceremony (planned).

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
>>> tensor.current_backend()
'reference'
>>> tensor.list_available_backends()
['reference']
"""

import importlib as _importlib
import importlib.util as _importlib_util
import os as _os
import sys as _sys
from typing import Literal as _Literal

# ─── Phase 6.5 — single-backend-per-process discovery ──────────────
# Per R-P6.5.5 (nanobind type registry is global per process), we
# probe which backends are *installed* but lazy-load exactly one.
# Preference: TENSOR_BACKEND env var > "reference" > "eigen" > "webgpu".

_BACKEND_MODULE_NAMES = {
    "reference": "_tensor_native",
    "eigen": "_tensor_native_eigen",
    "webgpu": "_tensor_native_webgpu",
}

_BackendName = _Literal["reference", "eigen", "webgpu"]

# Detect which backend extensions are *installed* (i.e. .so file exists)
# without actually importing them.
_INSTALLED_BACKENDS: list = []
for _name, _mod_name in _BACKEND_MODULE_NAMES.items():
    _spec = _importlib_util.find_spec(f"{__name__}.{_mod_name}")
    if _spec is not None:
        _INSTALLED_BACKENDS.append(_name)

if not _INSTALLED_BACKENDS:
    raise ImportError(
        "No tensor backends are installed. The reference backend is "
        "always shipped with `tensor-named-axis`; this error means the "
        "wheel's native extension failed to load. Try:\n"
        "  pip install --force-reinstall tensor-named-axis"
    )

# Choose ONE backend to actually load.
_env_choice = _os.environ.get("TENSOR_BACKEND", "").strip().lower()
if _env_choice and _env_choice not in _INSTALLED_BACKENDS:
    raise ImportError(
        f"TENSOR_BACKEND={_env_choice!r} but that backend is not "
        f"installed. Installed: {_INSTALLED_BACKENDS}.\n"
        f"Install with:  pip install tensor-named-axis[{_env_choice}]"
    )

# Preference order: env var > reference > eigen > webgpu.
if _env_choice:
    _CHOSEN_BACKEND: _BackendName = _env_choice  # type: ignore[assignment]
else:
    for _preferred in ("reference", "eigen", "webgpu"):
        if _preferred in _INSTALLED_BACKENDS:
            _CHOSEN_BACKEND = _preferred  # type: ignore[assignment]
            break

_NATIVE = _importlib.import_module(
    f".{_BACKEND_MODULE_NAMES[_CHOSEN_BACKEND]}", __name__
)
_CURRENT_BACKEND: _BackendName = _CHOSEN_BACKEND

# Bind public surface against the loaded backend's module.
__version__ = _NATIVE.__version__
Axis = _NATIVE.Axis
DynamicShape = _NATIVE.DynamicShape
DynamicTensor = _NATIVE.DynamicTensor
DynamicTensorF32 = _NATIVE.DynamicTensorF32
contract = _NATIVE.contract
from_numpy = _NATIVE.from_numpy
autograd = _NATIVE.autograd
tex = _NATIVE.tex
_native_hello = _NATIVE.hello

# nanobind submodules are attributes, not Python submodules. Register
# the dotted forms so `import tensor.autograd as ag` works (per the
# fix in PR #104).
_sys.modules[__name__ + ".autograd"] = autograd
_sys.modules[__name__ + ".tex"] = tex


def set_backend(name: _BackendName) -> None:
    """Select the active `KernelBackend` adapter (Phase 6.5 / ADR-0019).

    The base install ships only ``reference`` (5 MB wheel). Add Eigen
    and / or WebGPU via ``pip install tensor-named-axis[eigen]`` /
    ``[webgpu]`` / ``[all]``.

    **Known limitation (R-P6.5.5)**: nanobind 2.x's type registry is
    global per process. Switching to a different backend within an
    existing Python process is not supported — the new backend's
    extension module would conflict with the already-loaded one. Use
    one of:

    1. Set ``TENSOR_BACKEND=<name>`` before launching Python.
    2. Restart Python and call ``set_backend(name)`` (or use the env var).
    3. ``pip uninstall`` one backend's extras + install the other.

    Calling ``set_backend(current_backend())`` is always a no-op.
    Calling ``set_backend(other)`` raises ``RuntimeError`` with these
    instructions.

    Parameters
    ----------
    name
        One of ``"reference"`` / ``"eigen"`` / ``"webgpu"``.

    Raises
    ------
    RuntimeError
        If ``name`` is not in ``list_available_backends()`` — error
        message includes the ``pip install`` command.
        If ``name`` is installed but differs from the currently-loaded
        backend — error message includes the env-var workaround.

    Examples
    --------
    >>> import tensor
    >>> tensor.set_backend(tensor.current_backend())   # no-op
    """
    if name not in _INSTALLED_BACKENDS:
        raise RuntimeError(
            f"{name} backend is not installed.\n"
            f"Install with:  pip install tensor-named-axis[{name}]\n"
            f"Or install all backends:  pip install tensor-named-axis[all]\n"
            f"Currently available: {sorted(_INSTALLED_BACKENDS)}"
        )
    if name != _CURRENT_BACKEND:
        raise RuntimeError(
            f"Cannot switch from {_CURRENT_BACKEND!r} to {name!r} within "
            f"an existing Python process — nanobind's type registry is "
            f"process-global (Phase 6.5 R-P6.5.5).\n"
            f"To use {name!r} instead, restart Python with:\n"
            f"  TENSOR_BACKEND={name} python ...\n"
            f"Or pin the backend at install time and uninstall the "
            f"unwanted extra."
        )
    # No-op — already on the requested backend.


def current_backend() -> str:
    """Return the name of the currently active backend."""
    return _CURRENT_BACKEND


def list_available_backends() -> list:
    """Return the list of backends *installed* in this Python environment.

    Note: only ONE of these is actually loaded — see ``current_backend()``.
    Switching requires a fresh process (Phase 6.5 R-P6.5.5).
    """
    return sorted(_INSTALLED_BACKENDS)


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
