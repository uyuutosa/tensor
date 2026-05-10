// SPDX-License-Identifier: MIT
//
// tensor::core::ReferenceTensor — a tensor whose elements form a recurrence
// along its single axis: each element references the previous one through
// the operator that fires the recurrence.
//
// In the 2016 README, `r * 3` with `r = ReferenceTensor(3, ...)` produces
// `(9, 27, 81, 243, 729)`: starting from the initial value 3, multiply
// by 3 to get 9; then 9*3 to get 27; then 27*3 to get 81; etc. Six entries
// are kept (the very first 3 is the initial; element 0 is initial * 3 = 9).
//
// Modernized: no `new` / owning references. The recurrence is materialised
// into a plain Tensor<T, 1> at operator time.

#pragma once

#include <cstddef>
#include <utility>
#include <vector>

#include "tensor/core/axis.hpp"
#include "tensor/core/shape.hpp"
#include "tensor/core/tensor.hpp"

namespace tensor::core {

template <class T>
class ReferenceTensor {
public:
    ReferenceTensor(T initial, Axis axis)
        : initial_(std::move(initial)), axis_(std::move(axis)) {}

    [[nodiscard]] Axis const& axis() const noexcept { return axis_; }
    [[nodiscard]] T const& initial() const noexcept { return initial_; }

    // Multiplicative recurrence: element[0] = initial * scalar,
    // element[i+1] = element[i] * scalar.
    template <class Scalar>
    [[nodiscard]] auto multiply_by(Scalar scalar) const {
        using U = std::decay_t<decltype(initial_ * scalar)>;
        std::vector<U> out;
        out.reserve(axis_.extent);
        U current = static_cast<U>(initial_);
        for (std::size_t i = 0; i < axis_.extent; ++i) {
            current = current * scalar;
            out.push_back(current);
        }
        return Tensor<U, 1>(Shape<1>{axis_}, std::move(out));
    }

private:
    T initial_;
    Axis axis_;
};

// Operator: ReferenceTensor<T> * scalar materialises the recurrence.
// Symmetric for scalar * ReferenceTensor.
template <class T, class Scalar>
[[nodiscard]] auto operator*(ReferenceTensor<T> const& r, Scalar scalar) {
    return r.multiply_by(scalar);
}

template <class T, class Scalar>
[[nodiscard]] auto operator*(Scalar scalar, ReferenceTensor<T> const& r) {
    return r.multiply_by(scalar);
}

}  // namespace tensor::core
