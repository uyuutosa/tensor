// SPDX-License-Identifier: MIT
//
// tensor::autograd::DynamicVariable — runtime-rank autograd wrapper.
//
// Counterpart to Variable<T, N>: wraps tensor::core::DynamicTensor<T>
// plus a shared_ptr<DynamicGradAccum<T>>. Used as the canonical result
// type of broadcast-aware operators that may collapse or expand rank
// depending on input labels (Einstein-style).

#pragma once

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <utility>

#include "tensor/autograd/tape.hpp"
#include "tensor/autograd/variable.hpp"
#include "tensor/core/dynamic_shape.hpp"
#include "tensor/core/dynamic_tensor.hpp"
#include "tensor/core/shape.hpp"
#include "tensor/core/tensor.hpp"

namespace tensor::autograd {

// Dynamic-rank gradient accumulator. Mirrors GradAccum<T, N> but uses
// DynamicTensor so closures can carry whichever shape the forward op
// chose for the result.
template <class T>
struct DynamicGradAccum {
    tensor::core::DynamicTensor<T> grad;
    bool initialized{false};

    void contribute(tensor::core::DynamicTensor<T> const& delta) {
        if (!initialized) {
            grad = delta;
            initialized = true;
        } else {
            // Element-wise accumulation; shapes must match.
            for (std::size_t i = 0; i < grad.size(); ++i) {
                grad[i] = grad[i] + delta[i];
            }
        }
    }
};

template <class T>
class DynamicVariable {
public:
    using value_type = T;
    using tensor_type = tensor::core::DynamicTensor<T>;

    DynamicVariable() = default;

    explicit DynamicVariable(tensor_type value, bool requires_grad = false)
        : value_(std::move(value)),
          accum_(requires_grad ? std::make_shared<DynamicGradAccum<T>>() : nullptr) {
        if (accum_) {
            accum_->grad = tensor_type(value_.shape());
        }
    }

    // Implicit conversion from static-rank Variable<T, N> — the resulting
    // dynamic variable shares the same gradient lifecycle implicitly by
    // routing the static accumulator's contribute through a closure-
    // captured handle when needed. (For now we copy; the proper bridge
    // is a Phase 2 follow-up.)
    template <std::size_t N>
    DynamicVariable(Variable<T, N> const& v)
        : value_(v.value()),
          accum_(v.requires_grad() ? std::make_shared<DynamicGradAccum<T>>() : nullptr) {
        if (accum_) {
            accum_->grad = tensor_type(value_.shape());
        }
    }

    [[nodiscard]] tensor_type const& value() const noexcept { return value_; }
    [[nodiscard]] tensor_type const& grad() const {
        if (!accum_) {
            throw std::runtime_error("DynamicVariable::grad: does not require_grad");
        }
        return accum_->grad;
    }
    [[nodiscard]] bool requires_grad() const noexcept { return accum_ != nullptr; }

    [[nodiscard]] std::shared_ptr<DynamicGradAccum<T>> const& accum() const noexcept {
        return accum_;
    }

    // zero_grad — reset the accumulated gradient and mark it uninitialised.
    // No-op when requires_grad is false.
    void zero_grad() {
        if (!accum_) return;
        for (std::size_t i = 0; i < accum_->grad.size(); ++i) {
            accum_->grad[i] = T{};
        }
        accum_->initialized = false;
    }

    void seed_grad(tensor_type const& seed) {
        if (!accum_) {
            throw std::runtime_error("DynamicVariable::seed_grad: does not require_grad");
        }
        accum_->grad = seed;
        accum_->initialized = true;
    }

    // Convert back to static-rank Variable<T, N>. Throws if rank mismatch
    // OR if the dynamic variable has a tracked grad — that case requires
    // a bridge closure to forward gradient contributions back. For Phase 2
    // we allow the no-grad case only.
    template <std::size_t N>
    [[nodiscard]] Variable<T, N> to_static() const {
        if (accum_) {
            throw std::runtime_error(
                "DynamicVariable::to_static: cannot convert a grad-tracked variable; "
                "consume gradients via backward() then to_static() the plain value");
        }
        return Variable<T, N>(value_.template to_static<N>(), false);
    }

private:
    tensor_type value_{};
    std::shared_ptr<DynamicGradAccum<T>> accum_{};
};

}  // namespace tensor::autograd
