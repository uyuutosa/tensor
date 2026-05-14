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


def test_default_backend_respects_env_var_or_falls_back_to_reference():
    # Per the discovery logic in tensor/__init__.py:
    #   TENSOR_BACKEND env var > reference > eigen > webgpu.
    import os
    env_choice = os.environ.get("TENSOR_BACKEND", "").strip().lower()
    if env_choice and env_choice in tensor.list_available_backends():
        assert tensor.current_backend() == env_choice
    elif "reference" in tensor.list_available_backends():
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


def test_set_backend_to_other_installed_raises_with_env_var_hint():
    """Per R-P6.5.5, switching backends within a process is unsupported;
    must raise with the TENSOR_BACKEND env-var workaround."""
    available = tensor.list_available_backends()
    current = tensor.current_backend()
    others = [b for b in available if b != current]
    if not others:
        pytest.skip("only one backend installed; nothing to switch from")

    other = others[0]
    with pytest.raises(RuntimeError) as exc_info:
        tensor.set_backend(other)

    msg = str(exc_info.value)
    assert current in msg or f"{current!r}" in msg
    assert other in msg or f"{other!r}" in msg
    assert "TENSOR_BACKEND" in msg
    assert "restart Python" in msg.lower() or "fresh process" in msg.lower() \
        or "restart python" in msg.lower()


def test_dotted_imports_work():
    """`import tensor.autograd as ag` keeps working (per the sys.modules
    registration in __init__.py)."""
    import tensor.autograd as ag  # noqa: F401
    import tensor.tex as tex  # noqa: F401
    assert ag is tensor.autograd
    assert tex is tensor.tex


def test_native_backend_attribute_matches_current_backend():
    """The native extension exposes `__backend__` as a string attribute
    (per python/src/_tensor_native.cpp's `m.attr(\"__backend__\")`).
    The Python-side `current_backend()` must agree."""
    from tensor import _NATIVE  # type: ignore[attr-defined]
    assert _NATIVE.__backend__ == tensor.current_backend()


def test_public_surface_still_works_after_set_backend_noop():
    """`set_backend(current_backend())` is a no-op and the public
    surface continues to function."""
    tensor.set_backend(tensor.current_backend())
    a = tensor.from_numpy(np.array([1.0, 2.0, 3.0]), labels=["i"])
    b = tensor.from_numpy(np.array([10.0, 20.0]), labels=["j"])
    c = a + b
    assert c.shape.rank() == 2
