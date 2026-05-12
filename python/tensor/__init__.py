"""tensor — Python SDK for the named-axis differentiable C++ tensor library.

The Python surface is a DrivingAdapter (per ADR-0009 + ADR-0018) consuming
the same C++ Domain (`tensor::core` + `tensor::autograd` + `tensor::tex`)
that the C++ tests and tutorials exercise.

Phase 6 status (2026-05-12):

- P6.M1 — scaffold + smoke binding (this milestone).
- P6.M2 — `DynamicTensor` + arithmetic with Einstein-style broadcast.
- P6.M3 — `contract` + NumPy interop.
- P6.M4 — autograd (`DynamicVariable`, `backward`, `gradient_check`).
- P6.M5 — `tex.parse` + `Evaluator` (the `_tex` UDL equivalent in Python).
- P6.M6 — runtime backend selection (`reference` / `eigen` / `webgpu`);
  `0.2.0` release with first PyPI publish + conda-forge submission.

See ``docs/impl-plans/2026-05-12_phase-6-python-sdk.md`` for the full
milestone breakdown and ``docs/arc42/09-decisions/0018-phase-6-python-sdk-entry-via-nanobind.md``
for the architectural decisions.

The project's documentation site (``https://uyuutosa.github.io/tensor/``)
is the canonical entry point.
"""

from ._tensor_native import __version__
from ._tensor_native import hello as _native_hello

__all__ = ["__version__", "hello"]


def hello() -> str:
    """Return a fixed greeting from the C++ side.

    P6.M1 smoke surface. Subsequent milestones replace this with the
    actual ``DynamicTensor`` / ``DynamicVariable`` / ``Evaluator``
    bindings. Kept as a public symbol for the initial release's
    "did the install work?" diagnostic.

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
