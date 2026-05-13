// SPDX-License-Identifier: MIT
//
// tensor::autograd::reduce_along_label — autograd-aware single-axis sum.
//
// Forward: drop one named axis from a DynamicVariable by summing along it.
// Backward: broadcast dL/dout along the dropped axis to recover dL/dx.
//
// Companion to broadcast_ops.hpp's `sum_all` (which is the rank-0 reduce
// over every axis). This op closes the gap between sum_all (everything)
// and no-reduction-at-all that the Phase 6 follow-up note in
// docs/impl-plans/2026-05-12_phase-6-python-sdk.md called out.

#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "tensor/autograd/dynamic_variable.hpp"
#include "tensor/autograd/tape.hpp"
#include "tensor/core/broadcast.hpp"
#include "tensor/core/dynamic_shape.hpp"
#include "tensor/core/dynamic_tensor.hpp"
#include "tensor/core/reduce.hpp"

namespace tensor::autograd {

template <class T>
[[nodiscard]] DynamicVariable<T> reduce_along_label(
    DynamicVariable<T> const& x, std::string label)
{
    using namespace tensor::core;
    auto out_v = tensor::core::reduce_along_label(x.value(), label);
    bool req = x.requires_grad();
    DynamicVariable<T> out(std::move(out_v), req);
    if (req) {
        auto x_acc = x.accum();
        auto out_acc = out.accum();
        auto x_shape = x.value().shape();
        std::string label_captured = std::move(label);
        Tape::current().record([x_acc, out_acc, x_shape, label_captured]() {
            // Broadcast dL/dout along the reduced axis to recover dL/dx.
            std::size_t const axis_pos = x_shape.index_of(label_captured);
            DynamicTensor<T> dx(x_shape);
            std::vector<std::size_t> idx(x_shape.rank(), 0);
            std::size_t flat = 0;
            do {
                std::vector<std::size_t> out_idx;
                out_idx.reserve(x_shape.rank() == 0 ? 0 : x_shape.rank() - 1);
                for (std::size_t k = 0; k < idx.size(); ++k) {
                    if (k != axis_pos) out_idx.push_back(idx[k]);
                }
                dx[flat++] = out_acc->grad.at_index(out_idx);
            } while (increment_index(idx, x_shape));
            if (x_acc) x_acc->contribute(dx);
        });
    }
    return out;
}

}  // namespace tensor::autograd
