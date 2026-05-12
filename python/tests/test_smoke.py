"""P6.M1 smoke tests — verify the scikit-build-core + nanobind toolchain.

These tests run on the editable install (``pip install -e .``) and
deliberately exercise only the scaffold surface. Subsequent milestones'
tests live in dedicated files (``test_arithmetic.py``, ``test_autograd.py``,
``test_tex.py``, ``test_backend_parity.py``) per the impl-plan.
"""

import tensor


def test_version_attribute_present():
    """The module exposes a string ``__version__`` containing ``0.1.0``."""
    assert isinstance(tensor.__version__, str), (
        f"expected string, got {type(tensor.__version__).__name__}"
    )
    assert "0.1.0" in tensor.__version__


def test_hello_returns_expected_greeting():
    """``tensor.hello()`` round-trips Python → C++ → Python with the
    canonical greeting string. Proves the nanobind extension loaded and
    a function call succeeded."""
    assert tensor.hello() == "hello from tensor::core"


def test_public_surface_is_at_or_above_m2_baseline():
    """The public surface grows milestone by milestone. This test pins the
    P6.M2 baseline so accidental removals are caught; subsequent
    milestones extend the expected set.

    P6.M2 baseline (non-dunder public symbols):

    - ``hello``               — P6.M1 smoke binding, kept as diagnostic.
    - ``Axis``                — named-axis primitive.
    - ``DynamicShape``        — ordered list of axes.
    - ``DynamicTensor``       — float64 tensor (the educational default).
    - ``DynamicTensorF32``    — float32 tensor (for the WebGPU-bound path).

    P6.M3 will add ``contract`` and NumPy interop helpers; P6.M4 will add
    ``autograd``; P6.M5 will add the ``tex`` submodule. Each milestone
    updates this set.
    """
    public = {name for name in dir(tensor) if not name.startswith("_")}
    expected_m3 = {
        "hello",
        "Axis",
        "DynamicShape",
        "DynamicTensor",
        "DynamicTensorF32",
        "contract",
        "from_numpy",
    }
    assert expected_m3.issubset(public), (
        f"missing P6.M3 baseline symbols: {sorted(expected_m3 - public)}"
    )
    # `__version__` is exposed but lives under the dunder filter; check
    # it separately to make sure the import binding succeeded.
    assert hasattr(tensor, "__version__")
