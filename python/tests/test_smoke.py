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


def test_public_surface_is_minimal():
    """P6.M1 keeps the public surface deliberately small. The set grows
    in P6.M2 onward; this test catches accidental leakage from the
    nanobind extension module's namespace.

    `__version__` is a dunder attribute and is correctly filtered by the
    underscore rule; the visible non-dunder public surface at P6.M1 is
    just `hello`.
    """
    public = {name for name in dir(tensor) if not name.startswith("_")}
    assert public == {"hello"}, (
        f"unexpected non-dunder public symbols at P6.M1: {sorted(public)}"
    )
    # `__version__` is exposed but lives under the dunder filter; check
    # it separately to make sure the import binding succeeded.
    assert hasattr(tensor, "__version__")
