// SPDX-License-Identifier: MIT
//
// tensor::core::FunctionTensor — a tensor whose elements are *callables*
// applied element-wise to another tensor at operator time.
//
// In the 2016 README, the function tensor was constructed with a raw
// function pointer and used `new`/`delete` for storage. The modernized
// API takes a templated callable (by value) and calls it per element
// with `(index, value)` arguments.

#pragma once

#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

#include "tensor/core/axis.hpp"
#include "tensor/core/shape.hpp"
#include "tensor/core/tensor.hpp"

namespace tensor::core {

// FunctionTensor<F> wraps a single named axis and a callable F.
// Application against a Tensor<T, 1> with the matching axis runs F over
// each element to produce a Tensor<U, 1> where U = decltype(F(size_t, T)).
template <class F>
class FunctionTensor {
public:
    FunctionTensor(Axis axis, F fn) : axis_(std::move(axis)), fn_(std::move(fn)) {}

    [[nodiscard]] Axis const& axis() const noexcept { return axis_; }

    // Apply this function tensor over an input tensor. The input must have
    // the same axis label; matching extents required.
    template <class T>
    [[nodiscard]] auto apply_to(Tensor<T, 1> const& input) const {
        if (input.shape()[0].label != axis_.label) {
            throw std::invalid_argument(
                "FunctionTensor::apply_to: axis label mismatch");
        }
        if (input.shape()[0].extent != axis_.extent) {
            throw std::invalid_argument(
                "FunctionTensor::apply_to: axis extent mismatch");
        }
        using U = std::decay_t<decltype(fn_(std::size_t{0}, input(0)))>;
        std::vector<U> out;
        out.reserve(axis_.extent);
        for (std::size_t i = 0; i < axis_.extent; ++i) {
            out.push_back(fn_(i, input(i)));
        }
        return Tensor<U, 1>(Shape<1>{axis_}, std::move(out));
    }

private:
    Axis axis_;
    F fn_;
};

// Operators — Tensor<T, 1> * FunctionTensor<F> applies F element-wise.
// The original 2016 API used `*` and the same is preserved for fidelity
// with the README narrative; semantically `apply_to(a)` is the same idea
// expressed at the call-site level.
template <class T, class F>
[[nodiscard]] auto operator*(Tensor<T, 1> const& a, FunctionTensor<F> const& f) {
    return f.apply_to(a);
}

template <class T, class F>
[[nodiscard]] auto operator*(FunctionTensor<F> const& f, Tensor<T, 1> const& a) {
    return f.apply_to(a);
}

}  // namespace tensor::core
