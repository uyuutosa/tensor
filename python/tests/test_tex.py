"""P6.M5 — `tex.parse` + `Evaluator` cross-validation.

Mirrors the C++ side's `tensor::tex::Evaluator` usage from
``tutorials/01_formula-is-the-program.ipynb`` plus selected
``tests/test_tex_evaluator.cpp`` cases. Exit criterion per the
Phase 6 impl-plan: outer product + Einstein-sum examples from
notebook 01 reproduce identically in Python.
"""

import numpy as np
import pytest

import tensor
import tensor.tex as tex


# ─── parse / to_latex round trip ─────────────────────────────────────────


@pytest.mark.parametrize(
    "src",
    [
        "a_i",
        "a_i + b_i",
        "a_i b_j",
        "c_{ij} = a_i + b_j",
        r"\sum_i {a_i b_i}",
    ],
)
def test_parse_to_latex_round_trips(src):
    """Per docs/detailed-design/tensor-tex.md the round-trip property is
    `parse(to_latex(e)) == e` for every supported expression. Here we
    check the slightly stronger string-level property: the second
    `to_latex` is idempotent."""
    once = tex.to_latex(tex.parse(src))
    twice = tex.to_latex(tex.parse(once))
    assert once == twice


def test_expression_repr_includes_latex():
    expr = tex.parse(r"c_{ij} = a_i + b_j")
    r = repr(expr)
    assert "Expression(R" in r
    assert "c_{ij}" in r


def test_empty_expression_is_empty():
    expr = tex.Expression()
    assert expr.empty() is True
    assert str(expr) == ""


# ─── Evaluator — outer product (reproduces tutorial 01 §5) ───────────────


def test_outer_product_c_ij_eq_a_i_b_j():
    """c_{ij} = a_i b_j — mirrors notebook 01 §5 + README headline."""
    a = tensor.from_numpy(np.array([1.0, 2.0, 3.0]), ["i"])
    b = tensor.from_numpy(np.array([10.0, 20.0]), ["j"])

    ev = tex.Evaluator()
    ev.bind("a", a)
    ev.bind("b", b)
    expr = tex.parse(r"c_{ij} = a_i b_j")
    c = ev.evaluate(expr)

    # Expected: c_{ij} = a_i * b_j → 3×2 = (10,20, 20,40, 30,60)
    expected = np.outer([1.0, 2.0, 3.0], [10.0, 20.0])
    np.testing.assert_allclose(c.numpy(), expected, atol=1e-12)
    assert c.shape.rank() == 2
    assert c.shape.axes()[0].label == "i"
    assert c.shape.axes()[1].label == "j"


# ─── Evaluator — Einstein-sum inner product (notebook 01 §6) ─────────────


def test_einstein_inner_product_sum_i_a_i_b_i():
    """\\sum_i {a_i b_i} — the one-line inner product."""
    a = tensor.from_numpy(np.array([1.0, 2.0, 3.0, 4.0]), ["i"])
    b = tensor.from_numpy(np.array([2.0, 3.0, 5.0, 7.0]), ["i"])

    ev = tex.Evaluator()
    ev.bind("a", a)
    ev.bind("b", b)
    expr = tex.parse(r"\sum_i {a_i b_i}")
    s = ev.evaluate(expr)

    expected = float(np.dot([1.0, 2.0, 3.0, 4.0], [2.0, 3.0, 5.0, 7.0]))
    assert s.shape.rank() == 0
    assert s[0] == pytest.approx(expected, abs=1e-12)


# ─── Evaluator — element-wise sum + equation lhs annotation ─────────────


def test_addition_with_distinct_labels_gives_outer_sum():
    a = tensor.from_numpy(np.array([1.0, 2.0, 3.0, 4.0, 5.0]), ["i"])
    b = tensor.from_numpy(np.array([10.0, 20.0]), ["j"])
    ev = tex.Evaluator()
    ev.bind("a", a)
    ev.bind("b", b)
    expr = tex.parse(r"c_{ij} = a_i + b_j")
    c = ev.evaluate(expr)
    expected = (
        np.array([1.0, 2.0, 3.0, 4.0, 5.0]).reshape(-1, 1)
        + np.array([10.0, 20.0]).reshape(1, -1)
    )
    np.testing.assert_allclose(c.numpy(), expected, atol=1e-12)


# ─── Unbound variable + rank-mismatch guards ─────────────────────────────


def test_unbound_variable_raises():
    ev = tex.Evaluator()
    expr = tex.parse("a_i + b_j")
    with pytest.raises(Exception):
        ev.evaluate(expr)


def test_subscript_count_mismatch_raises():
    """Bind a rank-2 tensor as `a` but parse `a_i` (subscript count = 1)."""
    a2 = tensor.from_numpy(np.zeros((3, 4)), ["i", "j"])
    ev = tex.Evaluator()
    ev.bind("a", a2)
    with pytest.raises(Exception):
        ev.evaluate(tex.parse("a_i"))


# ─── EvaluatorF32 path ────────────────────────────────────────────────────


def test_evaluator_f32_works_for_float():
    a = tensor.from_numpy(np.array([1.0, 2.0], dtype=np.float32), ["i"])
    b = tensor.from_numpy(np.array([3.0, 4.0], dtype=np.float32), ["i"])
    ev = tex.EvaluatorF32()
    ev.bind("a", a)
    ev.bind("b", b)
    result = ev.evaluate(tex.parse(r"\sum_i {a_i b_i}"))
    # 1*3 + 2*4 = 11
    assert result[0] == pytest.approx(11.0, rel=1e-6)
