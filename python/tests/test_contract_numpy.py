"""P6.M3 — contract + NumPy interop cross-validation.

Two surfaces under test:

1. ``tensor.contract(a, b)`` — Einstein-sum over shared axis labels.
   Cross-validated against ``tests/test_contract.cpp`` TEST_CASE blocks
   (matvec, matmul, no-shared-axis outer product, inner-product scalar).

2. NumPy interop — ``tensor.from_numpy(arr, labels)`` + ``.numpy()`` round
   trip. Cross-validated against ``np.einsum`` for representative
   contractions.

Exit criterion per docs/impl-plans/2026-05-12_phase-6-python-sdk.md
P6.M3: ``np.einsum("ij,jk->ik", A, B)`` equals ``tensor.contract(tA, tB)``
element-wise where ``tA.labels == ("i","j")`` and ``tB.labels == ("j","k")``.
"""

import numpy as np
import pytest

import tensor


# ─── contract: cross-validation against tests/test_contract.cpp ──────────


def test_contract_matvec_y_eq_W_x():
    """C++ test: y_i = Σ_j W_{ij} x_j (matrix-vector product)."""
    W = tensor.DynamicTensor(
        tensor.DynamicShape([tensor.Axis("i", 2), tensor.Axis("j", 3)]),
        [1.0, 2.0, 3.0, 4.0, 5.0, 6.0],
    )
    x = tensor.DynamicTensor(
        tensor.DynamicShape([tensor.Axis("j", 3)]),
        [10.0, 20.0, 30.0],
    )
    y = tensor.contract(W, x)
    assert y.shape.rank() == 1
    assert y.size() == 2
    assert y[0] == pytest.approx(140.0, abs=1e-12)  # 1·10 + 2·20 + 3·30
    assert y[1] == pytest.approx(320.0, abs=1e-12)  # 4·10 + 5·20 + 6·30


def test_contract_matmul_C_eq_A_B():
    """C++ test: C_{ik} = Σ_j A_{ij} B_{jk} (matrix-matrix product)."""
    A = tensor.DynamicTensor(
        tensor.DynamicShape([tensor.Axis("i", 2), tensor.Axis("j", 3)]),
        [1.0, 2.0, 3.0, 4.0, 5.0, 6.0],
    )
    B = tensor.DynamicTensor(
        tensor.DynamicShape([tensor.Axis("j", 3), tensor.Axis("k", 2)]),
        [1.0, 0.0, 0.0, 1.0, 1.0, 1.0],
    )
    C = tensor.contract(A, B)
    assert C.shape.rank() == 2
    assert C.size() == 4
    assert C[0] == pytest.approx(4.0, abs=1e-12)   # 1·1+2·0+3·1
    assert C[1] == pytest.approx(5.0, abs=1e-12)   # 1·0+2·1+3·1
    assert C[2] == pytest.approx(10.0, abs=1e-12)  # 4·1+5·0+6·1
    assert C[3] == pytest.approx(11.0, abs=1e-12)  # 4·0+5·1+6·1


def test_contract_no_shared_axis_is_outer_product():
    """C++ test: when no labels are shared, contract == outer product."""
    a = tensor.DynamicTensor(
        tensor.DynamicShape([tensor.Axis("i", 2)]),
        [1.0, 2.0],
    )
    b = tensor.DynamicTensor(
        tensor.DynamicShape([tensor.Axis("j", 3)]),
        [10.0, 20.0, 30.0],
    )
    o = tensor.contract(a, b)
    assert o.shape.rank() == 2
    assert o[0] == pytest.approx(10.0, abs=1e-12)  # 1·10
    assert o[1] == pytest.approx(20.0, abs=1e-12)
    assert o[5] == pytest.approx(60.0, abs=1e-12)  # 2·30


def test_contract_all_axes_shared_yields_scalar():
    """C++ test: inner product (all axes shared) → rank-0 scalar."""
    a = tensor.DynamicTensor(
        tensor.DynamicShape([tensor.Axis("i", 3)]),
        [1.0, 2.0, 3.0],
    )
    b = tensor.DynamicTensor(
        tensor.DynamicShape([tensor.Axis("i", 3)]),
        [4.0, 5.0, 6.0],
    )
    s = tensor.contract(a, b)
    assert s.shape.rank() == 0
    assert s.size() == 1
    assert s[0] == pytest.approx(32.0, abs=1e-12)  # 1·4 + 2·5 + 3·6


# ─── NumPy interop — round-trip + einsum cross-validation ────────────────


def test_from_numpy_then_numpy_round_trips_data():
    arr = np.arange(6, dtype=np.float64).reshape(2, 3)
    t = tensor.from_numpy(arr, ["i", "j"])
    assert t.shape.rank() == 2
    assert t.shape.axes()[0].label == "i"
    assert t.shape.axes()[0].extent == 2
    assert t.shape.axes()[1].label == "j"
    assert t.shape.axes()[1].extent == 3

    back = t.numpy()
    np.testing.assert_array_equal(back, arr)
    # Independent buffers — mutating the round-trip copy must not affect
    # the originating tensor.
    back[0, 0] = 999.0
    assert t[0] == 0.0  # the tensor's first element is still the original


def test_from_numpy_label_count_must_match_rank():
    arr = np.zeros((2, 3), dtype=np.float64)
    with pytest.raises(Exception):
        tensor.from_numpy(arr, ["i"])  # 1 label, 2-D array


def test_from_numpy_float32_overload():
    arr = np.arange(4, dtype=np.float32).reshape(2, 2)
    t = tensor.from_numpy(arr, ["i", "j"])
    # The overload selected by dtype should give back a DynamicTensorF32.
    assert isinstance(t, tensor.DynamicTensorF32)
    back = t.numpy()
    np.testing.assert_array_equal(back, arr)


def test_contract_matches_np_einsum_ij_jk_to_ik():
    """**Exit-criterion check**: contract == np.einsum('ij,jk->ik', A, B).

    Per docs/impl-plans/2026-05-12_phase-6-python-sdk.md P6.M3 exit:
    contracting along the shared axis 'j' must produce the same result
    as NumPy's einsum on the underlying float64 buffers.
    """
    rng = np.random.default_rng(seed=42)
    A_np = rng.standard_normal((4, 7))
    B_np = rng.standard_normal((7, 5))
    expected = np.einsum("ij,jk->ik", A_np, B_np)

    A_t = tensor.from_numpy(A_np, ["i", "j"])
    B_t = tensor.from_numpy(B_np, ["j", "k"])
    C_t = tensor.contract(A_t, B_t)
    C_np = C_t.numpy()

    assert C_np.shape == expected.shape
    np.testing.assert_allclose(C_np, expected, rtol=1e-12, atol=1e-12)


def test_contract_inner_product_matches_np_dot():
    """Inner product (single shared axis, rank 1 × rank 1) → scalar.
    Cross-validated against ``np.dot``."""
    a_np = np.array([1.5, -2.0, 3.25, 4.0], dtype=np.float64)
    b_np = np.array([0.5, 1.0, -1.5, 2.0], dtype=np.float64)
    expected = float(np.dot(a_np, b_np))

    a_t = tensor.from_numpy(a_np, ["i"])
    b_t = tensor.from_numpy(b_np, ["i"])
    s_t = tensor.contract(a_t, b_t)
    assert s_t.shape.rank() == 0
    assert s_t[0] == pytest.approx(expected, abs=1e-12)
