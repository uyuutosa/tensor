"""P6.M4 — autograd cross-validation.

Exercises the `tensor.autograd` submodule end-to-end:

- DynamicVariable construction + requires_grad accounting
- arithmetic (+, -, *) with gradient flow
- activations (exp / log / relu / neg)
- dot (autograd-aware contract)
- sum_all + backward
- sgd_update

Exit criterion per docs/impl-plans/2026-05-12_phase-6-python-sdk.md
P6.M4: a one-liner linear-regression training loop converges to
W ≈ 2, b ≈ 1 within 1e-3 (mirroring tutorials/07_mlp-on-toy.ipynb).
"""

import math

import numpy as np
import pytest

import tensor
import tensor.autograd as ag


def _scalar_dv(value: float, requires_grad: bool = False):
    """Construct a rank-0 (scalar) DynamicVariable wrapping a Python float."""
    t = tensor.from_numpy(np.array(value, dtype=np.float64), [])
    return ag.DynamicVariable(t, requires_grad=requires_grad)


# ─── DynamicVariable basics ──────────────────────────────────────────────


def test_dynamic_variable_construct_no_grad():
    t = tensor.from_numpy(np.array([1.0, 2.0, 3.0]), ["i"])
    v = ag.DynamicVariable(t)
    assert v.requires_grad is False
    assert v.value.size() == 3


def test_dynamic_variable_requires_grad_starts_zero():
    t = tensor.from_numpy(np.array([1.0, 2.0, 3.0]), ["i"])
    v = ag.DynamicVariable(t, requires_grad=True)
    assert v.requires_grad is True
    grad = v.grad.numpy()
    np.testing.assert_array_equal(grad, np.zeros(3))


def test_dynamic_variable_grad_throws_without_requires_grad():
    t = tensor.from_numpy(np.array([1.0, 2.0]), ["i"])
    v = ag.DynamicVariable(t)
    with pytest.raises(Exception):
        _ = v.grad


# ─── forward + backward — single op ──────────────────────────────────────


def test_backward_on_sum_of_squares_gives_2x():
    """Loss = Σ_i x_i^2 → dL/dx_i = 2 x_i."""
    x_np = np.array([1.0, 2.0, 3.0, 4.0])
    x = ag.DynamicVariable(tensor.from_numpy(x_np, ["i"]), requires_grad=True)
    loss = ag.sum_all(x * x)
    ag.backward(loss)
    np.testing.assert_allclose(x.grad.numpy(), 2.0 * x_np, atol=1e-12)


def test_backward_on_sum_difference_gives_unit_grad():
    """Loss = Σ_i (a_i - b_i) → dL/da = 1, dL/db = -1."""
    a_np = np.array([5.0, 6.0, 7.0])
    b_np = np.array([1.0, 2.0, 3.0])
    a = ag.DynamicVariable(tensor.from_numpy(a_np, ["i"]), requires_grad=True)
    b = ag.DynamicVariable(tensor.from_numpy(b_np, ["i"]), requires_grad=True)
    loss = ag.sum_all(a - b)
    ag.backward(loss)
    np.testing.assert_allclose(a.grad.numpy(), np.ones_like(a_np), atol=1e-12)
    np.testing.assert_allclose(b.grad.numpy(), -np.ones_like(b_np), atol=1e-12)


# ─── activations ─────────────────────────────────────────────────────────


def test_relu_zero_negatives_in_grad():
    """ReLU's derivative is 1 where x > 0, else 0."""
    x_np = np.array([-2.0, -1.0, 0.5, 2.0])
    x = ag.DynamicVariable(tensor.from_numpy(x_np, ["i"]), requires_grad=True)
    loss = ag.sum_all(ag.relu(x))
    ag.backward(loss)
    expected = (x_np > 0.0).astype(np.float64)
    np.testing.assert_allclose(x.grad.numpy(), expected, atol=1e-12)


def test_exp_gradient_equals_exp_value():
    """d/dx exp(x) = exp(x)."""
    x_np = np.array([0.0, 1.0, -0.5])
    x = ag.DynamicVariable(tensor.from_numpy(x_np, ["i"]), requires_grad=True)
    loss = ag.sum_all(ag.exp(x))
    ag.backward(loss)
    np.testing.assert_allclose(x.grad.numpy(), np.exp(x_np), atol=1e-12)


def test_log_gradient_equals_reciprocal():
    """d/dx log(x) = 1/x."""
    x_np = np.array([1.0, 2.0, 4.0, 10.0])
    x = ag.DynamicVariable(tensor.from_numpy(x_np, ["i"]), requires_grad=True)
    loss = ag.sum_all(ag.log(x))
    ag.backward(loss)
    np.testing.assert_allclose(x.grad.numpy(), 1.0 / x_np, atol=1e-12)


def test_neg_gradient_equals_minus_one():
    x_np = np.array([1.0, -2.0, 3.0])
    x = ag.DynamicVariable(tensor.from_numpy(x_np, ["i"]), requires_grad=True)
    loss = ag.sum_all(ag.neg(x))
    ag.backward(loss)
    np.testing.assert_allclose(x.grad.numpy(), -np.ones_like(x_np), atol=1e-12)


# ─── dot (autograd-aware contract) ───────────────────────────────────────


def test_dot_matvec_gradient_with_respect_to_x():
    """y = W·x, loss = Σ_i y_i, dL/dx_j = Σ_i W_{ij}.

    Equivalent to one row of W summed over i — the same shape as x.
    """
    W_np = np.array([[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]])
    x_np = np.array([10.0, 20.0, 30.0])
    W = ag.DynamicVariable(tensor.from_numpy(W_np, ["i", "j"]))
    x = ag.DynamicVariable(tensor.from_numpy(x_np, ["j"]), requires_grad=True)

    y = ag.dot(W, x)
    loss = ag.sum_all(y)
    ag.backward(loss)

    # Each column-sum of W is the gradient on the corresponding x_j.
    expected = W_np.sum(axis=0)
    np.testing.assert_allclose(x.grad.numpy(), expected, atol=1e-12)


# ─── End-to-end exit criterion: linear-regression training loop ─────────


def test_linear_regression_converges_to_w2_b1():
    """Mirrors tutorials/07_mlp-on-toy.ipynb §4 — fits y = 2x + 1 from
    a noisy sample. Exit criterion: |W - 2| < 1e-3 and |b - 1| < 1e-3."""
    rng = np.random.default_rng(seed=1729)
    n = 64
    x_data = np.linspace(-1.0, 1.0, n)
    noise = rng.normal(0.0, 0.001, size=n)  # small noise; teaching artefact
    y_data = 2.0 * x_data + 1.0 + noise

    x_var = ag.DynamicVariable(tensor.from_numpy(x_data, ["n"]))
    y_var = ag.DynamicVariable(tensor.from_numpy(y_data, ["n"]))

    # Initialise W and b as rank-0 DynamicVariables. Rank-0 × rank-1
    # broadcasts via the same Einstein-style machinery the C++ side uses.
    W = ag.DynamicVariable(
        tensor.from_numpy(np.array(0.0), []),
        requires_grad=True,
    )
    b = ag.DynamicVariable(
        tensor.from_numpy(np.array(0.0), []),
        requires_grad=True,
    )

    lr = 0.1
    n_inv = 1.0 / n  # average-squared-error scaling
    inv = ag.DynamicVariable(tensor.from_numpy(np.array(n_inv), []))

    epochs = 500
    last_loss = math.inf
    for _ in range(epochs):
        W.zero_grad()
        b.zero_grad()

        pred = W * x_var + b
        diff = pred - y_var
        loss = ag.sum_all(diff * diff * inv)  # MSE
        ag.backward(loss)

        new_W = ag.sgd_update(W, lr)
        new_b = ag.sgd_update(b, lr)
        W = ag.DynamicVariable(new_W, requires_grad=True)
        b = ag.DynamicVariable(new_b, requires_grad=True)
        last_loss = float(loss.value)

    final_W = float(W.value.numpy())
    final_b = float(b.value.numpy())

    # Exit criterion from the impl-plan.
    assert abs(final_W - 2.0) < 1e-3, (
        f"W did not converge to 2 (got {final_W:.6f}; final loss {last_loss:.2e})"
    )
    assert abs(final_b - 1.0) < 1e-3, (
        f"b did not converge to 1 (got {final_b:.6f}; final loss {last_loss:.2e})"
    )
