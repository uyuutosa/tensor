// SPDX-License-Identifier: MIT
//
// tensor::autograd::sgd — minimal stochastic gradient descent helpers.
//
// Tiny training-loop ergonomics. Each helper is one obvious line in
// math; the helpers exist so that tutorial code reads like the math
// rather than spelling out the index loop every iteration.
//
// Two helpers, mirroring the two Variable variants:
//   sgd_update(v, lr)  — returns a new Tensor representing v.value() - lr * v.grad().
//                         The caller then rebuilds the Variable for the
//                         next iteration. This is the recommended pattern
//                         when the optimizer needs to reset the autograd
//                         graph between iterations (the most common case).
//   apply_sgd(v, lr)   — same effect but mutates v in place by overwriting
//                         the stored value and zero_grad(). Requires that
//                         no captured tape closure still references v's
//                         old GradAccum; safe to call after backward() and
//                         before the next forward.

#pragma once

#include <cstddef>

#include "tensor/autograd/dynamic_variable.hpp"
#include "tensor/autograd/variable.hpp"
#include "tensor/core/dynamic_tensor.hpp"
#include "tensor/core/tensor.hpp"

namespace tensor::autograd {

// ─── Variable<T, N> ──────────────────────────────────────────────────────────
template <class T, std::size_t N>
[[nodiscard]] tensor::core::Tensor<T, N> sgd_update(Variable<T, N> const& v, T lr) {
    tensor::core::Tensor<T, N> out = v.value();
    for (std::size_t i = 0; i < out.size(); ++i) {
        out[i] = out[i] - lr * v.grad()[i];
    }
    return out;
}

// ─── DynamicVariable<T> ─────────────────────────────────────────────────────
template <class T>
[[nodiscard]] tensor::core::DynamicTensor<T> sgd_update(DynamicVariable<T> const& v, T lr) {
    tensor::core::DynamicTensor<T> out = v.value();
    for (std::size_t i = 0; i < out.size(); ++i) {
        out[i] = out[i] - lr * v.grad()[i];
    }
    return out;
}

}  // namespace tensor::autograd
