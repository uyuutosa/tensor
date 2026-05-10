// SPDX-License-Identifier: MIT
//
// tensor::autograd::activations — element-wise differentiable primitives.
//
// These are the building blocks a learner needs to compose loss functions
// and small NN layers. Each function takes a `Variable<T, N>` and returns
// a `Variable<T, N>` of the same shape; the backward closure captures the
// input value(s) it needs and contributes to the input's grad accumulator
// during `backward()`.

#pragma once

#include <cmath>
#include <cstddef>

#include "tensor/autograd/tape.hpp"
#include "tensor/autograd/variable.hpp"
#include "tensor/core/tensor.hpp"

namespace tensor::autograd {

// ─── exp ──────────────────────────────────────────────────────────────────────
// y = exp(x); dy/dx = exp(x) = y.
template <class T, std::size_t N>
[[nodiscard]] Variable<T, N> exp(Variable<T, N> const& x) {
    using core_tensor = tensor::core::Tensor<T, N>;
    core_tensor out_v(x.value().shape());
    for (std::size_t i = 0; i < out_v.size(); ++i) {
        out_v[i] = std::exp(x.value()[i]);
    }
    bool req = x.requires_grad();
    Variable<T, N> out(std::move(out_v), req);
    if (req) {
        auto x_acc = x.accum();
        auto out_acc = out.accum();
        // Capture output value (= dy/dx for exp).
        auto out_val_copy = out.value();
        Tape::current().record([x_acc, out_acc, out_val_copy]() {
            core_tensor dx(out_val_copy.shape());
            for (std::size_t i = 0; i < dx.size(); ++i) {
                dx[i] = out_acc->grad[i] * out_val_copy[i];
            }
            if (x_acc) x_acc->contribute(dx);
        });
    }
    return out;
}

// ─── log ──────────────────────────────────────────────────────────────────────
// y = log(x); dy/dx = 1 / x. Precondition: x > 0 element-wise (not enforced).
template <class T, std::size_t N>
[[nodiscard]] Variable<T, N> log(Variable<T, N> const& x) {
    using core_tensor = tensor::core::Tensor<T, N>;
    core_tensor out_v(x.value().shape());
    for (std::size_t i = 0; i < out_v.size(); ++i) {
        out_v[i] = std::log(x.value()[i]);
    }
    bool req = x.requires_grad();
    Variable<T, N> out(std::move(out_v), req);
    if (req) {
        auto x_acc = x.accum();
        auto out_acc = out.accum();
        auto x_val_copy = x.value();
        Tape::current().record([x_acc, out_acc, x_val_copy]() {
            core_tensor dx(x_val_copy.shape());
            for (std::size_t i = 0; i < dx.size(); ++i) {
                dx[i] = out_acc->grad[i] / x_val_copy[i];
            }
            if (x_acc) x_acc->contribute(dx);
        });
    }
    return out;
}

// ─── relu ─────────────────────────────────────────────────────────────────────
// y = max(0, x); dy/dx = I(x > 0).
//
// At x = 0 the subderivative is taken to be 0 (the conventional choice;
// matches PyTorch / TensorFlow / tinygrad). The educational notebook in
// P2.M5 will discuss the choice explicitly.
template <class T, std::size_t N>
[[nodiscard]] Variable<T, N> relu(Variable<T, N> const& x) {
    using core_tensor = tensor::core::Tensor<T, N>;
    core_tensor out_v(x.value().shape());
    for (std::size_t i = 0; i < out_v.size(); ++i) {
        out_v[i] = x.value()[i] > T{0} ? x.value()[i] : T{0};
    }
    bool req = x.requires_grad();
    Variable<T, N> out(std::move(out_v), req);
    if (req) {
        auto x_acc = x.accum();
        auto out_acc = out.accum();
        auto x_val_copy = x.value();
        Tape::current().record([x_acc, out_acc, x_val_copy]() {
            core_tensor dx(x_val_copy.shape());
            for (std::size_t i = 0; i < dx.size(); ++i) {
                dx[i] = x_val_copy[i] > T{0} ? out_acc->grad[i] : T{0};
            }
            if (x_acc) x_acc->contribute(dx);
        });
    }
    return out;
}

// ─── neg / unary operator- ────────────────────────────────────────────────────
// y = -x; dy/dx = -1 (element-wise).
template <class T, std::size_t N>
[[nodiscard]] Variable<T, N> neg(Variable<T, N> const& x) {
    using core_tensor = tensor::core::Tensor<T, N>;
    core_tensor out_v(x.value().shape());
    for (std::size_t i = 0; i < out_v.size(); ++i) {
        out_v[i] = -x.value()[i];
    }
    bool req = x.requires_grad();
    Variable<T, N> out(std::move(out_v), req);
    if (req) {
        auto x_acc = x.accum();
        auto out_acc = out.accum();
        Tape::current().record([x_acc, out_acc]() {
            core_tensor dx(out_acc->grad.shape());
            for (std::size_t i = 0; i < dx.size(); ++i) {
                dx[i] = -out_acc->grad[i];
            }
            if (x_acc) x_acc->contribute(dx);
        });
    }
    return out;
}

template <class T, std::size_t N>
[[nodiscard]] Variable<T, N> operator-(Variable<T, N> const& x) {
    return neg(x);
}

}  // namespace tensor::autograd
