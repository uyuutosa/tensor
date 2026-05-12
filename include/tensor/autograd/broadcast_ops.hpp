// SPDX-License-Identifier: MIT
//
// tensor::autograd::broadcast_ops — Einstein-style broadcast operators
// that record reverse-mode tape entries.
//
// Forward uses tensor::core::broadcast_shapes to determine the result
// shape and the per-axis source maps. Backward uses the same plan to
// reduce dL/dout over axes that exist in the result but not in a given
// input — that is, the "unbroadcast" step that reverses the forward
// broadcast direction.

#pragma once

#include <cstddef>
#include <functional>
#include <utility>
#include <vector>

#include "tensor/autograd/dynamic_variable.hpp"
#include "tensor/autograd/tape.hpp"
#include "tensor/autograd/variable.hpp"
#include "tensor/core/broadcast.hpp"
#include "tensor/core/dynamic_shape.hpp"
#include "tensor/core/dynamic_tensor.hpp"

namespace tensor::autograd {

namespace detail {

// (unbroadcast moved to tensor::core::unbroadcast in P2.5.M2 so the
// KernelBackend port can call it; see tensor/core/broadcast.hpp.)

// Forward element-wise apply with broadcast. Walks the result-shape
// multi-index and pulls values from a and b via the BroadcastPlan.
template <class T, class Op>
[[nodiscard]] tensor::core::DynamicTensor<T> forward_apply(
    tensor::core::DynamicTensor<T> const& a,
    tensor::core::DynamicTensor<T> const& b,
    tensor::core::BroadcastPlan const& plan,
    Op op) {
    using namespace tensor::core;
    DynamicTensor<T> out(plan.result);
    std::vector<std::size_t> idx(plan.result.rank(), 0);
    std::size_t flat = 0;
    do {
        auto ai = project_index(idx, plan.a_source, a.shape().rank());
        auto bi = project_index(idx, plan.b_source, b.shape().rank());
        out[flat++] = op(a.at_index(ai), b.at_index(bi));
    } while (increment_index(idx, plan.result));
    return out;
}

}  // namespace detail

// ─── DynamicVariable × DynamicVariable broadcast ops ─────────────────────────
//
// + - * /. Each constructs the BroadcastPlan once for the forward pass,
// then captures it (plus the input tensors when the backward needs them)
// in the tape closure.

template <class T>
[[nodiscard]] DynamicVariable<T> operator+(DynamicVariable<T> const& a,
                                           DynamicVariable<T> const& b) {
    using namespace tensor::core;
    auto plan = broadcast_shapes(a.value().shape(), b.value().shape());
    auto out_v = detail::forward_apply(a.value(), b.value(), plan,
                                       [](T x, T y) { return x + y; });
    bool req = a.requires_grad() || b.requires_grad();
    DynamicVariable<T> out(std::move(out_v), req);
    if (req) {
        auto a_acc = a.accum();
        auto b_acc = b.accum();
        auto out_acc = out.accum();
        auto a_shape = a.value().shape();
        auto b_shape = b.value().shape();
        auto a_source = plan.a_source;
        auto b_source = plan.b_source;
        Tape::current().record([a_acc, b_acc, out_acc, a_shape, b_shape, a_source, b_source]() {
            if (a_acc) {
                a_acc->contribute(tensor::core::unbroadcast(out_acc->grad, a_source, a_shape));
            }
            if (b_acc) {
                b_acc->contribute(tensor::core::unbroadcast(out_acc->grad, b_source, b_shape));
            }
        });
    }
    return out;
}

template <class T>
[[nodiscard]] DynamicVariable<T> operator-(DynamicVariable<T> const& a,
                                           DynamicVariable<T> const& b) {
    using namespace tensor::core;
    auto plan = broadcast_shapes(a.value().shape(), b.value().shape());
    auto out_v = detail::forward_apply(a.value(), b.value(), plan,
                                       [](T x, T y) { return x - y; });
    bool req = a.requires_grad() || b.requires_grad();
    DynamicVariable<T> out(std::move(out_v), req);
    if (req) {
        auto a_acc = a.accum();
        auto b_acc = b.accum();
        auto out_acc = out.accum();
        auto a_shape = a.value().shape();
        auto b_shape = b.value().shape();
        auto a_source = plan.a_source;
        auto b_source = plan.b_source;
        Tape::current().record([a_acc, b_acc, out_acc, a_shape, b_shape, a_source, b_source]() {
            if (a_acc) {
                a_acc->contribute(tensor::core::unbroadcast(out_acc->grad, a_source, a_shape));
            }
            if (b_acc) {
                // dL/db = -unbroadcast(dL/dout)
                auto db = tensor::core::unbroadcast(out_acc->grad, b_source, b_shape);
                for (std::size_t i = 0; i < db.size(); ++i) {
                    db[i] = -db[i];
                }
                b_acc->contribute(db);
            }
        });
    }
    return out;
}

template <class T>
[[nodiscard]] DynamicVariable<T> operator*(DynamicVariable<T> const& a,
                                           DynamicVariable<T> const& b) {
    using namespace tensor::core;
    auto plan = broadcast_shapes(a.value().shape(), b.value().shape());
    auto out_v = detail::forward_apply(a.value(), b.value(), plan,
                                       [](T x, T y) { return x * y; });
    bool req = a.requires_grad() || b.requires_grad();
    DynamicVariable<T> out(std::move(out_v), req);
    if (req) {
        auto a_acc = a.accum();
        auto b_acc = b.accum();
        auto out_acc = out.accum();
        auto a_shape = a.value().shape();
        auto b_shape = b.value().shape();
        auto a_source = plan.a_source;
        auto b_source = plan.b_source;
        auto a_val = a.value();
        auto b_val = b.value();
        Tape::current().record(
            [a_acc, b_acc, out_acc, a_shape, b_shape, a_source, b_source, a_val, b_val]() {
                // dL/da = unbroadcast(dL/dout * b_broadcast, a_source, a_shape)
                // We compute dL/dout * b_broadcast in result shape then unbroadcast.
                DynamicTensor<T> da_full(out_acc->grad.shape());
                DynamicTensor<T> db_full(out_acc->grad.shape());
                std::vector<std::size_t> idx(out_acc->grad.shape().rank(), 0);
                std::size_t flat = 0;
                do {
                    auto ai = project_index(idx, a_source, a_val.shape().rank());
                    auto bi = project_index(idx, b_source, b_val.shape().rank());
                    T const dout = out_acc->grad[flat];
                    da_full[flat] = dout * b_val.at_index(bi);
                    db_full[flat] = dout * a_val.at_index(ai);
                    ++flat;
                } while (increment_index(idx, out_acc->grad.shape()));
                if (a_acc) {
                    a_acc->contribute(tensor::core::unbroadcast(da_full, a_source, a_shape));
                }
                if (b_acc) {
                    b_acc->contribute(tensor::core::unbroadcast(db_full, b_source, b_shape));
                }
            });
    }
    return out;
}

// ─── sum_all on DynamicVariable ──────────────────────────────────────────────
template <class T>
[[nodiscard]] Variable<T, 0> sum_all(DynamicVariable<T> const& x) {
    T total{};
    for (std::size_t i = 0; i < x.value().size(); ++i) {
        total = total + x.value()[i];
    }
    using namespace tensor::core;
    Tensor<T, 0> out_v(Shape<0>{});
    out_v[0] = total;

    bool req = x.requires_grad();
    Variable<T, 0> out(std::move(out_v), req);
    if (req) {
        auto x_acc = x.accum();
        auto out_acc = out.accum();
        auto x_shape = x.value().shape();
        Tape::current().record([x_acc, out_acc, x_shape]() {
            DynamicTensor<T> dx(x_shape);
            T const s = out_acc->grad[0];
            for (std::size_t i = 0; i < dx.size(); ++i) {
                dx[i] = s;
            }
            if (x_acc) x_acc->contribute(dx);
        });
    }
    return out;
}

// ─── Convenience: lift Variable<T, N> to DynamicVariable<T> in operator chain ─
//
// When mixing static and dynamic in expressions like Variable<T, 1> +
// DynamicVariable<T>, an explicit conversion improves type clarity. We
// rely on DynamicVariable<T>'s converting constructor from Variable<T, N>
// (see dynamic_variable.hpp) — this means `Variable<T, 1>(a) +
// DynamicVariable<T>(b)` works after explicit wrapping.

}  // namespace tensor::autograd
