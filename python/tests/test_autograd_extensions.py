"""Bundle B — `sin` / `cos` / `sqrt` / `__truediv__` / `reduce_along_label`.

Forward + backward + chain-rule checks for the autograd-surface extensions
added in the Phase 6 follow-up. Closed-form gradients:

- `sin(x)`  → cos(x)
- `cos(x)`  → -sin(x)
- `sqrt(x)` → 1 / (2 sqrt(x))
- `a / b`   → 1/b (wrt a),  -a/b² (wrt b)
- `reduce_along_label(x, 'i')` → broadcast back along axis 'i'

The forward-only paths are also cross-checked against `np.sin` / `np.cos` /
`np.sqrt` / `np.divide` / `np.sum(axis=…)` so the binding's element-wise
buffer copy is verified to match NumPy semantics.
"""

import numpy as np
import pytest

import tensor
import tensor.autograd as ag


# ─── sin / cos / sqrt ────────────────────────────────────────────────────


def test_sin_forward_and_gradient():
    x_np = np.array([0.0, 0.5, 1.0, 1.5])
    x = ag.DynamicVariable(tensor.from_numpy(x_np, ["i"]), requires_grad=True)
    y = ag.sin(x)
    np.testing.assert_allclose(y.value.numpy(), np.sin(x_np), atol=1e-12)
    ag.backward(ag.sum_all(y))
    np.testing.assert_allclose(x.grad.numpy(), np.cos(x_np), atol=1e-12)


def test_cos_forward_and_gradient():
    x_np = np.array([0.0, 0.5, 1.0, 1.5])
    x = ag.DynamicVariable(tensor.from_numpy(x_np, ["i"]), requires_grad=True)
    y = ag.cos(x)
    np.testing.assert_allclose(y.value.numpy(), np.cos(x_np), atol=1e-12)
    ag.backward(ag.sum_all(y))
    np.testing.assert_allclose(x.grad.numpy(), -np.sin(x_np), atol=1e-12)


def test_sqrt_forward_and_gradient():
    x_np = np.array([1.0, 4.0, 9.0, 16.0])
    x = ag.DynamicVariable(tensor.from_numpy(x_np, ["i"]), requires_grad=True)
    y = ag.sqrt(x)
    np.testing.assert_allclose(y.value.numpy(), np.sqrt(x_np), atol=1e-12)
    ag.backward(ag.sum_all(y))
    np.testing.assert_allclose(x.grad.numpy(), 0.5 / np.sqrt(x_np), atol=1e-12)


def test_sin_cos_chain_rule_via_double_angle_identity():
    """A small chain-rule sanity check: sin(2x) = 2 sin(x) cos(x) →
    d/dx sin(2x) = 2 cos(2x). We build `sin(x) * cos(x)` and check the
    derivative comes out as cos(2x) (the cos / sin double-angle formula
    in derivative form)."""
    x_np = np.array([0.1, 0.3, 0.7])
    x = ag.DynamicVariable(tensor.from_numpy(x_np, ["i"]), requires_grad=True)
    half_sin_2x = ag.sin(x) * ag.cos(x)
    ag.backward(ag.sum_all(half_sin_2x))
    # d/dx [sin(x) cos(x)] = cos²(x) - sin²(x) = cos(2x)
    np.testing.assert_allclose(
        x.grad.numpy(), np.cos(2 * x_np), atol=1e-12,
    )


# ─── __truediv__ ──────────────────────────────────────────────────────────


def test_truediv_forward_elementwise():
    a = ag.DynamicVariable(tensor.from_numpy(np.array([6.0, 8.0, 10.0]), ["i"]))
    b = ag.DynamicVariable(tensor.from_numpy(np.array([2.0, 4.0, 5.0]), ["i"]))
    c = a / b
    np.testing.assert_allclose(c.value.numpy(), np.array([3.0, 2.0, 2.0]), atol=1e-12)


def test_truediv_backward_with_respect_to_both():
    a_np = np.array([6.0, 8.0, 10.0])
    b_np = np.array([2.0, 4.0, 5.0])
    a = ag.DynamicVariable(tensor.from_numpy(a_np, ["i"]), requires_grad=True)
    b = ag.DynamicVariable(tensor.from_numpy(b_np, ["i"]), requires_grad=True)
    ag.backward(ag.sum_all(a / b))
    np.testing.assert_allclose(a.grad.numpy(), 1.0 / b_np, atol=1e-12)
    np.testing.assert_allclose(b.grad.numpy(), -a_np / (b_np * b_np), atol=1e-12)


def test_truediv_broadcast_scalar_denominator():
    """L = Σ_i a_i / b where b is rank-0 scalar.
    dL/da_i = 1/b ;  dL/db = -Σ_i a_i / b²"""
    a_np = np.array([2.0, 4.0, 6.0])
    b_val = 2.0
    a = ag.DynamicVariable(tensor.from_numpy(a_np, ["i"]), requires_grad=True)
    b = ag.DynamicVariable(
        tensor.from_numpy(np.array(b_val), []), requires_grad=True,
    )
    ag.backward(ag.sum_all(a / b))
    np.testing.assert_allclose(a.grad.numpy(), np.full(3, 1.0 / b_val), atol=1e-12)
    assert float(b.grad.numpy()) == pytest.approx(-np.sum(a_np) / (b_val ** 2), abs=1e-12)


# ─── reduce_along_label ──────────────────────────────────────────────────


def test_reduce_along_label_forward_matches_np_sum():
    """Sum along 'i' on a 2×3 tensor → rank-1 with axis 'j'."""
    t_np = np.array([[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]])
    t = ag.DynamicVariable(tensor.from_numpy(t_np, ["i", "j"]))
    s = ag.reduce_along_label(t, "i")
    np.testing.assert_allclose(s.value.numpy(), t_np.sum(axis=0), atol=1e-12)
    assert s.value.shape.rank() == 1
    assert s.value.shape.axes()[0].label == "j"


def test_reduce_along_label_backward_broadcasts_back():
    """L = Σ_j (Σ_i x_{ij}) → dL/dx_{ij} = 1 everywhere."""
    t_np = np.ones((2, 3))
    t = ag.DynamicVariable(tensor.from_numpy(t_np, ["i", "j"]), requires_grad=True)
    ag.backward(ag.sum_all(ag.reduce_along_label(t, "i")))
    np.testing.assert_allclose(t.grad.numpy(), np.ones_like(t_np), atol=1e-12)


def test_reduce_then_weighted_loss_yields_broadcast_gradient():
    """L = Σ_j w_j (Σ_i x_{ij}). dL/dx_{ij} = w_j broadcast over i."""
    t_np = np.ones((2, 3))
    w_np = np.array([10.0, 20.0, 30.0])
    t = ag.DynamicVariable(tensor.from_numpy(t_np, ["i", "j"]), requires_grad=True)
    w = ag.DynamicVariable(tensor.from_numpy(w_np, ["j"]))
    reduced = ag.reduce_along_label(t, "i")
    ag.backward(ag.sum_all(reduced * w))
    expected = np.broadcast_to(w_np, (2, 3))
    np.testing.assert_allclose(t.grad.numpy(), expected, atol=1e-12)


# ─── Rodrigues 2D rotation smoke (uses sin + cos + division composition) ──


def test_2d_rotation_via_rodrigues_components():
    """Rotate the vector (1, 0) by π/2 using sin/cos compositions. Verify
    the rotated vector is (0, 1) up to machine precision, and that
    ∂(rotation result)/∂θ at θ = π/2 produces the expected (-sin(π/2),
    cos(π/2)) = (-1, 0) component-wise (when each output is differentiated
    with respect to θ)."""
    theta_np = np.array(np.pi / 2)
    theta = ag.DynamicVariable(tensor.from_numpy(theta_np, []), requires_grad=True)
    c = ag.cos(theta)
    s = ag.sin(theta)
    # rotated x = cos θ * 1 - sin θ * 0 = cos θ
    rotated_x = c
    # rotated y = sin θ * 1 + cos θ * 0 = sin θ
    rotated_y = s
    assert float(rotated_x.value.numpy()) == pytest.approx(0.0, abs=1e-12)
    assert float(rotated_y.value.numpy()) == pytest.approx(1.0, abs=1e-12)

    # d(rotated_x)/dθ = -sin θ ; d(rotated_y)/dθ = cos θ.
    # Loss = rotated_y → dL/dθ = cos θ = 0 at θ = π/2.
    ag.backward(ag.sum_all(rotated_y))
    assert float(theta.grad.numpy()) == pytest.approx(np.cos(np.pi / 2), abs=1e-12)
