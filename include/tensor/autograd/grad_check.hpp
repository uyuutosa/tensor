// SPDX-License-Identifier: MIT
//
// tensor::autograd::grad_check — finite-difference gradient verification.
//
// `gradient_check(f, x, eps, tol)` perturbs each element of `x` by ±eps,
// evaluates the scalar function `f` (which must rebuild the autograd
// graph from `x`), and compares the resulting numerical gradient to the
// analytical gradient produced by `backward()`.
//
// This is a teaching artefact, not a production-grade checker. It runs
// in O(numel(x)) forward passes; suitable for the tutorial corpus and
// for unit tests, not for production training.

#pragma once

#include <cmath>
#include <cstddef>
#include <functional>
#include <stdexcept>

#include "tensor/autograd/tape.hpp"
#include "tensor/autograd/variable.hpp"
#include "tensor/core/tensor.hpp"

namespace tensor::autograd {

// gradient_check(f, x, eps, tol)
//   f: a callable taking Variable<T, N> const& and returning Variable<T, 0>
//      (a scalar loss). Each invocation must build a fresh autograd graph.
//   x: the input variable whose gradient is checked. Must require_grad.
//   eps: the finite-difference step size (default 1e-3).
//   tol: per-element absolute tolerance (default 1e-3).
//
// Returns true if the analytical gradient matches the numerical gradient
// within tolerance for every element; false otherwise.
//
// Side effects: invokes f O(numel(x)+1) times and clears the tape between
// invocations (each call to f records into a fresh tape).
template <class T, std::size_t N, class F>
[[nodiscard]] bool gradient_check(F&& f,
                                  Variable<T, N>& x,
                                  T eps = T{1e-3},
                                  T tol = T{1e-3}) {
    if (!x.requires_grad()) {
        throw std::runtime_error("gradient_check: x must require_grad");
    }
    using core_tensor = tensor::core::Tensor<T, N>;

    // ─── Analytical gradient via backward ─────────────────────────────────────
    Tape::current().clear();
    auto x_for_back = Variable<T, N>(x.value(), true);
    auto loss = f(x_for_back);
    backward(loss);
    core_tensor analytical = x_for_back.grad();

    // ─── Numerical gradient via central differences ──────────────────────────
    core_tensor numerical(x.value().shape());
    auto x_value = x.value();
    for (std::size_t i = 0; i < x.value().size(); ++i) {
        T const original = x_value[i];

        x_value[i] = original + eps;
        Tape::current().clear();
        auto v_plus = Variable<T, N>(x_value, false);
        auto loss_plus = f(v_plus);
        T const f_plus = loss_plus.value()[0];

        x_value[i] = original - eps;
        Tape::current().clear();
        auto v_minus = Variable<T, N>(x_value, false);
        auto loss_minus = f(v_minus);
        T const f_minus = loss_minus.value()[0];

        x_value[i] = original;  // restore
        numerical[i] = (f_plus - f_minus) / (T{2} * eps);
    }

    // ─── Compare element-wise ────────────────────────────────────────────────
    for (std::size_t i = 0; i < analytical.size(); ++i) {
        T const diff = std::abs(analytical[i] - numerical[i]);
        if (diff > tol) {
            return false;
        }
    }
    Tape::current().clear();
    return true;
}

}  // namespace tensor::autograd
