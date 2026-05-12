"""P6.M2 — arithmetic cross-validation against the C++ test suite.

Every case mirrors a `TEST_CASE` in ``tests/test_ops.cpp`` (which itself
reproduces the four-arithmetic block from the 2016 README on the new
API). The expected values are duplicated here verbatim from the C++
side so the Python binding fails fast if the two surfaces ever drift.

Exit criterion per ``docs/impl-plans/2026-05-12_phase-6-python-sdk.md``
P6.M2: cross-validate one outer product + one same-axis broadcast against
the C++ test suite's expected values, within ``1e-12`` for ``double``.
"""

import math

import pytest

import tensor


# ─── fixtures ────────────────────────────────────────────────────────────


def _vec(label: str, extent: int, values):
    """Build a rank-1 DynamicTensor with the given label / extent / values."""
    return tensor.DynamicTensor(
        tensor.DynamicShape([tensor.Axis(label, extent)]),
        list(values),
    )


def _readme_a():
    """README example: a_i = (1, 2, 3, 4, 5)."""
    return _vec("i", 5, [1.0, 2.0, 3.0, 4.0, 5.0])


def _readme_b():
    """README example: b_j = (1, 2, 3, 4, 5)."""
    return _vec("j", 5, [1.0, 2.0, 3.0, 4.0, 5.0])


# ─── Axis / DynamicShape ─────────────────────────────────────────────────


def test_axis_construction_and_fields():
    a = tensor.Axis("i", 5)
    assert a.label == "i"
    assert a.extent == 5
    assert a == tensor.Axis("i", 5)
    assert a != tensor.Axis("j", 5)
    assert "Axis(" in repr(a)


def test_dynamic_shape_construction_and_total():
    s = tensor.DynamicShape([tensor.Axis("i", 5), tensor.Axis("j", 3)])
    assert s.rank() == 2
    assert s.total() == 15
    assert len(s) == 2
    axes = s.axes()
    assert axes[0].label == "i"
    assert axes[0].extent == 5
    assert axes[1].label == "j"
    assert axes[1].extent == 3


def test_dynamic_shape_empty():
    s = tensor.DynamicShape()
    assert s.rank() == 0
    assert s.total() == 1  # empty product convention


# ─── DynamicTensor — construction + properties ───────────────────────────


def test_dynamic_tensor_default_zero_initialised():
    t = tensor.DynamicTensor(
        tensor.DynamicShape([tensor.Axis("i", 3)])
    )
    assert t.size() == 3
    assert t.shape.rank() == 1
    for i in range(3):
        assert t[i] == 0.0


def test_dynamic_tensor_from_buffer():
    t = tensor.DynamicTensor(
        tensor.DynamicShape([tensor.Axis("i", 3)]),
        [1.0, 2.0, 3.0],
    )
    assert t.size() == 3
    assert t[0] == 1.0
    assert t[2] == 3.0


def test_dynamic_tensor_buffer_size_mismatch_raises():
    with pytest.raises(Exception):
        tensor.DynamicTensor(
            tensor.DynamicShape([tensor.Axis("i", 3)]),
            [1.0, 2.0],  # too short
        )


def test_dynamic_tensor_repr_includes_class_name():
    t = tensor.DynamicTensor(
        tensor.DynamicShape([tensor.Axis("i", 2)]),
        [1.0, 2.0],
    )
    r = repr(t)
    assert "DynamicTensor" in r


# ─── Arithmetic with Einstein-style broadcast ────────────────────────────
#
# These four test cases mirror tests/test_ops.cpp TEST_CASE blocks
# "README a {+,-,*,/} b: distinct labels produce a 5x5 {sum,difference,
# product,ratio} table" — same expected values.


def test_readme_add_distinct_labels_outer_sum():
    """C++ test: a + b → c_{ij} = a_i + b_j, rank 2, 5×5 table."""
    a = _readme_a()
    b = _readme_b()
    c = a + b
    assert c.shape.rank() == 2
    axes = c.shape.axes()
    assert axes[0].label == "i"
    assert axes[1].label == "j"
    assert c.size() == 25
    # Spot-check the four corners (same as the C++ doctest assertions).
    assert c[0 * 5 + 0] == pytest.approx(2.0, abs=1e-12)   # 1 + 1
    assert c[0 * 5 + 4] == pytest.approx(6.0, abs=1e-12)   # 1 + 5
    assert c[4 * 5 + 0] == pytest.approx(6.0, abs=1e-12)   # 5 + 1
    assert c[4 * 5 + 4] == pytest.approx(10.0, abs=1e-12)  # 5 + 5


def test_readme_sub_distinct_labels_outer_difference():
    a = _readme_a()
    b = _readme_b()
    c = a - b
    assert c.size() == 25
    assert c[0 * 5 + 0] == pytest.approx(0.0, abs=1e-12)
    assert c[4 * 5 + 0] == pytest.approx(4.0, abs=1e-12)
    assert c[0 * 5 + 4] == pytest.approx(-4.0, abs=1e-12)
    assert c[4 * 5 + 4] == pytest.approx(0.0, abs=1e-12)


def test_readme_mul_distinct_labels_outer_product():
    a = _readme_a()
    b = _readme_b()
    c = a * b
    assert c.size() == 25
    assert c[2 * 5 + 2] == pytest.approx(9.0, abs=1e-12)
    assert c[4 * 5 + 4] == pytest.approx(25.0, abs=1e-12)
    assert c[1 * 5 + 3] == pytest.approx(8.0, abs=1e-12)


def test_readme_div_distinct_labels_outer_ratio():
    a = _readme_a()
    b = _readme_b()
    c = a / b
    assert c.size() == 25
    assert c[0 * 5 + 0] == pytest.approx(1.0, rel=1e-12)
    assert c[2 * 5 + 0] == pytest.approx(3.0, rel=1e-12)
    assert c[0 * 5 + 4] == pytest.approx(0.2, rel=1e-12)
    assert c[4 * 5 + 4] == pytest.approx(1.0, rel=1e-12)
    assert c[1 * 5 + 2] == pytest.approx(2.0 / 3.0, rel=1e-12)


def test_same_label_axes_collapse_to_elementwise():
    """C++ test: a + c with both labelled 'i' is rank-1 element-wise."""
    a = _readme_a()
    c = _vec("i", 5, [10.0, 20.0, 30.0, 40.0, 50.0])
    s = a + c
    assert s.shape.rank() == 1
    assert s.shape.axes()[0].label == "i"
    assert s.size() == 5
    assert s[0] == pytest.approx(11.0, abs=1e-12)
    assert s[4] == pytest.approx(55.0, abs=1e-12)


# ─── Float32 path — narrower tolerance because float ─────────────────────


def test_f32_path_runs_and_outer_product_correct():
    a = tensor.DynamicTensorF32(
        tensor.DynamicShape([tensor.Axis("i", 3)]),
        [1.0, 2.0, 3.0],
    )
    b = tensor.DynamicTensorF32(
        tensor.DynamicShape([tensor.Axis("j", 2)]),
        [10.0, 20.0],
    )
    c = a * b
    # c_{ij} = a_i * b_j → 3x2 = (10,20,20,40,30,60)
    assert c.size() == 6
    assert c[0] == pytest.approx(10.0, rel=1e-6)
    assert c[1] == pytest.approx(20.0, rel=1e-6)
    assert c[2] == pytest.approx(20.0, rel=1e-6)
    assert c[3] == pytest.approx(40.0, rel=1e-6)
    assert c[4] == pytest.approx(30.0, rel=1e-6)
    assert c[5] == pytest.approx(60.0, rel=1e-6)
