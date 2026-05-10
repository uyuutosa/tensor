// SPDX-License-Identifier: MIT
//
// tensor::autograd::Variable — autograd-aware tensor wrapper.
//
// Variable<T, N> wraps tensor::core::Tensor<T, N> with a shared_ptr to
// a GradAccum<T, N>. Operators construct new Variables and record a
// closure into the thread-local Tape that propagates gradients.
//
// MVP restrictions (this milestone):
//   - Element-wise binary ops (+ - *) require same shape on both inputs.
//     Label-broadcast autograd is a follow-up; the broadcast backward
//     reduces over collapsed axes.
//   - sum_all() reduces a tensor to a rank-0 scalar; the canonical
//     starting point for backward().
//   - All ops capture by value (Tensor<T, N> is copyable). The cost is
//     acceptable for an educational artifact; a TensorView non-owning
//     wrapper is on the roadmap.

#pragma once

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <utility>

#include "tensor/autograd/tape.hpp"
#include "tensor/core/axis.hpp"
#include "tensor/core/shape.hpp"
#include "tensor/core/tensor.hpp"

namespace tensor::autograd {

template <class T, std::size_t N>
class Variable {
public:
    using value_type = T;
    using tensor_type = tensor::core::Tensor<T, N>;

    Variable() = default;

    explicit Variable(tensor_type value, bool requires_grad = false)
        : value_(std::move(value)),
          accum_(requires_grad ? std::make_shared<GradAccum<T, N>>() : nullptr) {
        if (accum_) {
            // Pre-size the grad tensor to match the value's shape so
            // contributors can index by flat offset directly.
            accum_->grad = tensor_type(value_.shape());
        }
    }

    [[nodiscard]] tensor_type const& value() const noexcept { return value_; }
    [[nodiscard]] tensor_type const& grad() const {
        if (!accum_) {
            throw std::runtime_error("Variable::grad: this variable does not require_grad");
        }
        return accum_->grad;
    }
    [[nodiscard]] bool requires_grad() const noexcept { return accum_ != nullptr; }

    // Internal: tape closures need access to the shared accumulator.
    [[nodiscard]] std::shared_ptr<GradAccum<T, N>> const& accum() const noexcept {
        return accum_;
    }

    // Seed the gradient (used by backward(): for a scalar loss, set d_loss/d_loss = 1).
    void seed_grad(tensor_type const& seed) {
        if (!accum_) {
            throw std::runtime_error("Variable::seed_grad: this variable does not require_grad");
        }
        accum_->grad = seed;
        accum_->initialized = true;
    }

private:
    tensor_type value_{};
    std::shared_ptr<GradAccum<T, N>> accum_{};
};

namespace detail {

// Build a tensor of all-ones with the same shape as `s` (used as the
// scalar seed for backward()).
template <class T, std::size_t N>
[[nodiscard]] tensor::core::Tensor<T, N> ones_like(tensor::core::Shape<N> const& s) {
    tensor::core::Tensor<T, N> out(s);
    for (std::size_t i = 0; i < out.size(); ++i) {
        out[i] = T{1};
    }
    return out;
}

template <class T, std::size_t N>
[[nodiscard]] tensor::core::Tensor<T, N> zeros_like(tensor::core::Shape<N> const& s) {
    return tensor::core::Tensor<T, N>(s);  // default-init is zero for arithmetic types
}

template <class T, std::size_t N>
void check_same_shape(Variable<T, N> const& a, Variable<T, N> const& b, char const* op) {
    if (!(a.value().shape() == b.value().shape())) {
        throw std::invalid_argument(
            std::string("Variable autograd ") + op +
            ": shape mismatch (broadcast backward is a Phase 2 follow-up)");
    }
}

}  // namespace detail

// ─── Element-wise + - * (same shape) ──────────────────────────────────────────
template <class T, std::size_t N>
[[nodiscard]] Variable<T, N> operator+(Variable<T, N> const& a, Variable<T, N> const& b) {
    detail::check_same_shape(a, b, "+");
    using tensor_type = tensor::core::Tensor<T, N>;
    tensor_type out_v(a.value().shape());
    for (std::size_t i = 0; i < out_v.size(); ++i) {
        out_v[i] = a.value()[i] + b.value()[i];
    }
    bool req = a.requires_grad() || b.requires_grad();
    Variable<T, N> out(std::move(out_v), req);
    if (req) {
        auto a_acc = a.accum();
        auto b_acc = b.accum();
        auto out_acc = out.accum();
        Tape::current().record([a_acc, b_acc, out_acc]() {
            // dL/da = dL/dc * 1; dL/db = dL/dc * 1
            if (a_acc) a_acc->contribute(out_acc->grad);
            if (b_acc) b_acc->contribute(out_acc->grad);
        });
    }
    return out;
}

template <class T, std::size_t N>
[[nodiscard]] Variable<T, N> operator-(Variable<T, N> const& a, Variable<T, N> const& b) {
    detail::check_same_shape(a, b, "-");
    using tensor_type = tensor::core::Tensor<T, N>;
    tensor_type out_v(a.value().shape());
    for (std::size_t i = 0; i < out_v.size(); ++i) {
        out_v[i] = a.value()[i] - b.value()[i];
    }
    bool req = a.requires_grad() || b.requires_grad();
    Variable<T, N> out(std::move(out_v), req);
    if (req) {
        auto a_acc = a.accum();
        auto b_acc = b.accum();
        auto out_acc = out.accum();
        Tape::current().record([a_acc, b_acc, out_acc]() {
            // dL/da = dL/dc; dL/db = -dL/dc
            if (a_acc) a_acc->contribute(out_acc->grad);
            if (b_acc) {
                tensor_type neg(out_acc->grad.shape());
                for (std::size_t i = 0; i < neg.size(); ++i) {
                    neg[i] = -out_acc->grad[i];
                }
                b_acc->contribute(neg);
            }
        });
    }
    return out;
}

template <class T, std::size_t N>
[[nodiscard]] Variable<T, N> operator*(Variable<T, N> const& a, Variable<T, N> const& b) {
    detail::check_same_shape(a, b, "*");
    using tensor_type = tensor::core::Tensor<T, N>;
    tensor_type out_v(a.value().shape());
    for (std::size_t i = 0; i < out_v.size(); ++i) {
        out_v[i] = a.value()[i] * b.value()[i];
    }
    bool req = a.requires_grad() || b.requires_grad();
    Variable<T, N> out(std::move(out_v), req);
    if (req) {
        auto a_acc = a.accum();
        auto b_acc = b.accum();
        auto out_acc = out.accum();
        // Capture inputs by value (Tensor is copyable). Required so the
        // closure has the multiplicands available at backward time.
        auto a_val = a.value();
        auto b_val = b.value();
        Tape::current().record([a_acc, b_acc, out_acc, a_val, b_val]() {
            // dL/da = dL/dc * b; dL/db = dL/dc * a (element-wise).
            if (a_acc) {
                tensor_type da(a_val.shape());
                for (std::size_t i = 0; i < da.size(); ++i) {
                    da[i] = out_acc->grad[i] * b_val[i];
                }
                a_acc->contribute(da);
            }
            if (b_acc) {
                tensor_type db(b_val.shape());
                for (std::size_t i = 0; i < db.size(); ++i) {
                    db[i] = out_acc->grad[i] * a_val[i];
                }
                b_acc->contribute(db);
            }
        });
    }
    return out;
}

// ─── sum_all: reduce a tensor to a rank-0 scalar Variable ─────────────────────
//
// The backward broadcasts the scalar gradient back to all elements of the
// input. This is the canonical entry point for `backward()` on a loss.
template <class T, std::size_t N>
[[nodiscard]] Variable<T, 0> sum_all(Variable<T, N> const& x) {
    T total{};
    for (std::size_t i = 0; i < x.value().size(); ++i) {
        total = total + x.value()[i];
    }
    using core_tensor0 = tensor::core::Tensor<T, 0>;
    using core_shape0 = tensor::core::Shape<0>;
    core_tensor0 out_v(core_shape0{});
    out_v[0] = total;

    bool req = x.requires_grad();
    Variable<T, 0> out(std::move(out_v), req);
    if (req) {
        auto x_acc = x.accum();
        auto out_acc = out.accum();
        auto x_shape = x.value().shape();
        Tape::current().record([x_acc, out_acc, x_shape]() {
            // dL/dx_i = dL/dscalar (broadcast).
            using tensor_type = tensor::core::Tensor<T, N>;
            tensor_type dx(x_shape);
            T s = out_acc->grad[0];
            for (std::size_t i = 0; i < dx.size(); ++i) {
                dx[i] = s;
            }
            if (x_acc) x_acc->contribute(dx);
        });
    }
    return out;
}

// ─── backward: kick off reverse-mode propagation from a scalar Variable ──────
//
// Sets the seed gradient of `loss` to 1 and walks the tape in reverse.
// `loss` must be rank-0 and require_grad.
template <class T>
void backward(Variable<T, 0>& loss) {
    if (!loss.requires_grad()) {
        throw std::runtime_error("backward: loss does not require_grad");
    }
    using tensor0 = tensor::core::Tensor<T, 0>;
    tensor0 seed(loss.value().shape());
    seed[0] = T{1};
    loss.seed_grad(seed);
    Tape::current().backward();
}

}  // namespace tensor::autograd
