// SPDX-License-Identifier: MIT
//
// tensor::autograd::dot — autograd-aware Einstein-style contraction.
//
// Forward: y = contract(a, b) — sum over shared axis labels.
// Backward:
//   dL/da = contract(dL/dy, b)
//   dL/db = contract(a, dL/dy)
//
// The same contraction kernel powers both forward and backward; this is
// the algebraic gift that pays for the named-axis abstraction.

#pragma once

#include <cstddef>
#include <utility>

#include "tensor/autograd/dynamic_variable.hpp"
#include "tensor/autograd/tape.hpp"
#include "tensor/core/contract.hpp"
#include "tensor/core/dynamic_tensor.hpp"

namespace tensor::autograd {

template <class T>
[[nodiscard]] DynamicVariable<T> dot(DynamicVariable<T> const& a,
                                     DynamicVariable<T> const& b) {
    auto out_v = tensor::core::contract(a.value(), b.value());
    bool req = a.requires_grad() || b.requires_grad();
    DynamicVariable<T> out(std::move(out_v), req);
    if (req) {
        auto a_acc = a.accum();
        auto b_acc = b.accum();
        auto out_acc = out.accum();
        auto a_val = a.value();
        auto b_val = b.value();
        Tape::current().record([a_acc, b_acc, out_acc, a_val, b_val]() {
            if (a_acc) {
                a_acc->contribute(tensor::core::contract(out_acc->grad, b_val));
            }
            if (b_acc) {
                b_acc->contribute(tensor::core::contract(a_val, out_acc->grad));
            }
        });
    }
    return out;
}

}  // namespace tensor::autograd
