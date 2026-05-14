"""Phase 6.5 M2 — `tensor.set_backend()` + `current_backend()` +
`list_available_backends()` Python surface tests.

Per [ADR-0019] + [the Phase 6.5 impl-plan §P6.5.M2]. M2's exit criterion
is "backend parity test passes on a local `pip install -e .[all]`
build (all three `.so` files coexist; switching is observable)".

This test file covers the M2 surface even on a default install where
only `reference` is available — most assertions are conditioned on the
installed backend set so the test suite passes against any extras
combination.

[ADR-0019]: ../../docs/arc42/09-decisions/0019-phase-6-5-runtime-backend-selection-via-extras.md
[the Phase 6.5 impl-plan §P6.5.M2]: ../../docs/impl-plans/2026-05-13_phase-6-5-set-backend.md
"""

from __future__ import annotations

import numpy as np
import pytest

import tensor


def test_list_available_backends_returns_a_list_of_strings():
    backends = tensor.list_available_backends()
    assert isinstance(backends, list)
    assert all(isinstance(b, str) for b in backends)
    # The base wheel always ships reference; the test fails if it isn't
    # installed because that's the only "well-formed install" baseline.
    assert "reference" in backends, (
        f"reference backend must always be installed; got {backends}"
    )


def test_list_available_backends_is_sorted():
    backends = tensor.list_available_backends()
    assert backends == sorted(backends)


def test_current_backend_returns_a_known_backend_name():
    name = tensor.current_backend()
    assert isinstance(name, str)
    assert name in tensor.list_available_backends()
    assert name in {"reference", "eigen", "webgpu"}


def test_default_backend_is_reference_when_installed():
    # The Python adapter picks `reference` as the default if it's
    # installed (per the discovery logic in tensor/__init__.py).
    if "reference" in tensor.list_available_backends():
        assert tensor.current_backend() == "reference"


def test_set_backend_to_already_active_is_a_noop():
    current = tensor.current_backend()
    tensor.set_backend(current)
    assert tensor.current_backend() == current


def test_set_backend_to_missing_raises_with_install_instructions():
    # Find a backend that's NOT installed; if all three are installed
    # (rare — only the `[all]` install path), skip this test.
    all_known = {"reference", "eigen", "webgpu"}
    missing = all_known - set(tensor.list_available_backends())
    if not missing:
        pytest.skip("all three backends are installed; nothing to probe")
    not_installed = next(iter(missing))

    with pytest.raises(RuntimeError) as exc_info:
        tensor.set_backend(not_installed)

    msg = str(exc_info.value)
    assert not_installed in msg
    assert "pip install" in msg
    # Per the errors-as-docs discipline (arc42 §8 §5.2), the error must
    # name the exact install command.
    assert f"tensor-named-axis[{not_installed}]" in msg
    assert "Currently available:" in msg


def test_set_backend_switches_observably():
    # If multiple backends are installed, switching must change
    # current_backend() AND must not break the public surface.
    available = tensor.list_available_backends()
    if len(available) < 2:
        pytest.skip("need ≥ 2 installed backends to test switching")

    for backend in available:
        tensor.set_backend(backend)
        assert tensor.current_backend() == backend

        # The public surface still works after the switch.
        a = tensor.from_numpy(np.array([1.0, 2.0, 3.0]), labels=["i"])
        b = tensor.from_numpy(np.array([10.0, 20.0]), labels=["j"])
        c = a + b
        assert c.shape.rank() == 2  # i × j outer-sum table

    # Reset to reference for downstream tests.
    if "reference" in available:
        tensor.set_backend("reference")


def test_set_backend_rebinds_top_level_symbols():
    # After set_backend(), tensor.DynamicTensor should point at the new
    # backend's class. We can't easily compare type identities across
    # nanobind modules, but we can at least verify the symbol still
    # exists and is callable.
    available = tensor.list_available_backends()
    for backend in available:
        tensor.set_backend(backend)
        # Every public symbol survives the rebind:
        assert callable(getattr(tensor, "DynamicTensor", None))
        assert callable(getattr(tensor, "contract", None))
        assert callable(getattr(tensor, "from_numpy", None))
        # Submodules survive too:
        assert hasattr(tensor, "autograd")
        assert hasattr(tensor, "tex")


def test_dotted_imports_work_after_set_backend():
    """`import tensor.autograd as ag` keeps working after each switch
    (the sys.modules registration trick from the Phase 6 retrospective
    has to re-run on each set_backend call)."""
    available = tensor.list_available_backends()
    for backend in available:
        tensor.set_backend(backend)
        import tensor.autograd as ag  # noqa: F401
        import tensor.tex as tex  # noqa: F401
        assert ag is tensor.autograd
        assert tex is tensor.tex


def test_native_backend_attribute_matches_current_backend():
    """The native extension exposes `__backend__` as a string attribute
    (per python/src/_tensor_native.cpp's `m.attr(\"__backend__\")`).
    The Python-side `current_backend()` must agree."""
    available = tensor.list_available_backends()
    for backend in available:
        tensor.set_backend(backend)
        # Look up the loaded native module via the private dict; we
        # only use this for the test introspection, not as public API.
        from tensor import _AVAILABLE_BACKENDS  # type: ignore[attr-defined]
        native = _AVAILABLE_BACKENDS[backend]
        assert native.__backend__ == backend, (
            f"native module reports __backend__={native.__backend__!r} "
            f"but Python adapter selected {backend!r}"
        )
