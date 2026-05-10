// SPDX-License-Identifier: MIT
//
// tensor::core::ops — element-wise binary operators with Einstein-style
// label-broadcast.
//
// `Tensor<T, M>` × `Tensor<U, N>` always returns `DynamicTensor<W>` where
// `W = decltype(T{} op U{})` and the result rank is the cardinality of
// the union of input axis labels. Same-label axes contract pairwise;
// distinct-label axes form an outer product.
//
// The same kernel powers `DynamicTensor<T>` × `DynamicTensor<T>` so a
// later compile-time path (LabelTag NTTP, ADR-0004) can dispatch into
// the same loop without a behavioural divergence.

#pragma once

#include <cstddef>
#include <type_traits>
#include <vector>

#include "tensor/core/broadcast.hpp"
#include "tensor/core/dynamic_shape.hpp"
#include "tensor/core/dynamic_tensor.hpp"
#include "tensor/core/tensor.hpp"

namespace tensor::core {

namespace detail {

// Generic element-wise kernel. `Op` is any callable taking (Ta, Tb) and
// returning a value type W. The kernel iterates the result-shape multi-
// index space, projects to A and B, and applies `Op`.
template <class Ta, class Tb, class Op>
[[nodiscard]] auto apply_elementwise(DynamicTensor<Ta> const& a,
                                     DynamicTensor<Tb> const& b,
                                     Op op) {
    using W = std::decay_t<decltype(op(std::declval<Ta>(), std::declval<Tb>()))>;
    auto plan = broadcast_shapes(a.shape(), b.shape());
    DynamicTensor<W> out(plan.result);

    std::vector<std::size_t> idx(plan.result.rank(), 0);
    std::size_t flat = 0;
    do {
        auto ai = project_index(idx, plan.a_source, a.shape().rank());
        auto bi = project_index(idx, plan.b_source, b.shape().rank());
        out[flat++] = op(a.at_index(ai), b.at_index(bi));
    } while (increment_index(idx, plan.result));

    // The do/while above runs once even when rank == 0 (scalar broadcast),
    // which is correct for our purposes; the `++flat` matches.
    return out;
}

}  // namespace detail

// ─── DynamicTensor × DynamicTensor ────────────────────────────────────────────
template <class Ta, class Tb>
[[nodiscard]] auto operator+(DynamicTensor<Ta> const& a, DynamicTensor<Tb> const& b) {
    return detail::apply_elementwise(a, b, [](auto x, auto y) { return x + y; });
}
template <class Ta, class Tb>
[[nodiscard]] auto operator-(DynamicTensor<Ta> const& a, DynamicTensor<Tb> const& b) {
    return detail::apply_elementwise(a, b, [](auto x, auto y) { return x - y; });
}
template <class Ta, class Tb>
[[nodiscard]] auto operator*(DynamicTensor<Ta> const& a, DynamicTensor<Tb> const& b) {
    return detail::apply_elementwise(a, b, [](auto x, auto y) { return x * y; });
}
template <class Ta, class Tb>
[[nodiscard]] auto operator/(DynamicTensor<Ta> const& a, DynamicTensor<Tb> const& b) {
    return detail::apply_elementwise(a, b, [](auto x, auto y) { return x / y; });
}

// ─── Static-rank Tensor<T, M> × Tensor<U, N> via DynamicTensor ────────────────
// These overloads convert through DynamicTensor and return DynamicTensor;
// the result rank is generally not known at compile time (it depends on
// the union of label sets, which is runtime data).
template <class Ta, std::size_t M, class Tb, std::size_t N>
[[nodiscard]] auto operator+(Tensor<Ta, M> const& a, Tensor<Tb, N> const& b) {
    return DynamicTensor<Ta>(a) + DynamicTensor<Tb>(b);
}
template <class Ta, std::size_t M, class Tb, std::size_t N>
[[nodiscard]] auto operator-(Tensor<Ta, M> const& a, Tensor<Tb, N> const& b) {
    return DynamicTensor<Ta>(a) - DynamicTensor<Tb>(b);
}
template <class Ta, std::size_t M, class Tb, std::size_t N>
[[nodiscard]] auto operator*(Tensor<Ta, M> const& a, Tensor<Tb, N> const& b) {
    return DynamicTensor<Ta>(a) * DynamicTensor<Tb>(b);
}
template <class Ta, std::size_t M, class Tb, std::size_t N>
[[nodiscard]] auto operator/(Tensor<Ta, M> const& a, Tensor<Tb, N> const& b) {
    return DynamicTensor<Ta>(a) / DynamicTensor<Tb>(b);
}

}  // namespace tensor::core
