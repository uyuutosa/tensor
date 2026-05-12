"""tensor — Python SDK for the named-axis differentiable C++ tensor library.

The Python surface is a DrivingAdapter (per ADR-0009 + ADR-0018) consuming
the same C++ Domain (`tensor::core` + `tensor::autograd` + `tensor::tex`)
that the C++ tests and tutorials exercise.

Phase 6 status (2026-05-12):

- P6.M1 ✅ — scaffold + smoke binding.
- P6.M2 ✅ — `Axis` + `DynamicShape` + `DynamicTensor` (float64) +
  `DynamicTensorF32` (float32) with the four arithmetic operators
  under Einstein-style label broadcast.
- P6.M3 — `contract` + NumPy interop (next).
- P6.M4 — autograd (`DynamicVariable`, `backward`, `gradient_check`).
- P6.M5 — `tex.parse` + `Evaluator` (the `_tex` UDL equivalent).
- P6.M6 — runtime backend selection; `0.2.0` release with first PyPI
  publish + conda-forge submission.

See ``docs/impl-plans/2026-05-12_phase-6-python-sdk.md`` for the full
milestone breakdown and ``docs/arc42/09-decisions/0018-phase-6-python-sdk-entry-via-nanobind.md``
for the architectural decisions. The project's documentation site
(``https://uyuutosa.github.io/tensor/``) is the canonical entry point.

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
"""

from ._tensor_native import __version__
from ._tensor_native import hello as _native_hello
from ._tensor_native import (
    Axis,
    DynamicShape,
    DynamicTensor,
    DynamicTensorF32,
)

__all__ = [
    "__version__",
    "hello",
    "Axis",
    "DynamicShape",
    "DynamicTensor",
    "DynamicTensorF32",
]


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
