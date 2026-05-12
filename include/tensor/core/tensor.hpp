// SPDX-License-Identifier: MIT
#pragma once

#include <cstddef>
#include <initializer_list>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

#include "tensor/core/concepts.hpp"
#include "tensor/core/shape.hpp"

namespace tensor::core {

// Tensor<T, N> — owning, contiguous, header-only.
//
// Storage is std::vector<T> in row-major order: the rightmost axis varies
// fastest. The shape is value-stored alongside; for compile-time named-axis
// shapes (M3+, NTTP path) a separate type-parametric Tensor will share the
// same kernel implementation via the hybrid bridge.
template <class T, std::size_t N>
class Tensor {
public:
    using value_type = T;
    using shape_type = Shape<N>;
    [[nodiscard]] static constexpr std::size_t rank() noexcept { return N; }

    // ─── Construction ─────────────────────────────────────────────────────
    Tensor() = default;

    explicit Tensor(Shape<N> shape) : shape_(std::move(shape)), buffer_(shape_.total()) {}

    Tensor(Shape<N> shape, std::initializer_list<T> values) : shape_(std::move(shape)) {
        if (values.size() != shape_.total()) {
            throw std::invalid_argument("Tensor: value count != product of extents");
        }
        buffer_.assign(values.begin(), values.end());
    }

    Tensor(Shape<N> shape, std::vector<T> values) : shape_(std::move(shape)), buffer_(std::move(values)) {
        if (buffer_.size() != shape_.total()) {
            throw std::invalid_argument("Tensor: buffer size != product of extents");
        }
    }

    // ─── Shape access ─────────────────────────────────────────────────────
    [[nodiscard]] Shape<N> const& shape() const noexcept { return shape_; }
    [[nodiscard]] std::size_t size() const noexcept { return buffer_.size(); }

    // ─── Buffer access ────────────────────────────────────────────────────
    [[nodiscard]] T* data() noexcept { return buffer_.data(); }
    [[nodiscard]] T const* data() const noexcept { return buffer_.data(); }

    [[nodiscard]] std::span<T> as_span() noexcept { return {buffer_.data(), buffer_.size()}; }
    [[nodiscard]] std::span<T const> as_span() const noexcept {
        return {buffer_.data(), buffer_.size()};
    }

    // ─── Linear (flat) element access ─────────────────────────────────────
    [[nodiscard]] T& operator[](std::size_t flat_index) noexcept { return buffer_[flat_index]; }
    [[nodiscard]] T const& operator[](std::size_t flat_index) const noexcept {
        return buffer_[flat_index];
    }

    // ─── Multi-dimensional element access (row-major) ─────────────────────
    // Variadic to keep the call site math-flavoured: `t(i, j, k)` not `t[{i,j,k}]`.
    template <class... Idx>
        requires(sizeof...(Idx) == N) && (std::convertible_to<Idx, std::size_t> && ...)
    [[nodiscard]] T& operator()(Idx... idx) noexcept {
        return buffer_[flatten(std::array<std::size_t, N>{static_cast<std::size_t>(idx)...})];
    }

    template <class... Idx>
        requires(sizeof...(Idx) == N) && (std::convertible_to<Idx, std::size_t> && ...)
    [[nodiscard]] T const& operator()(Idx... idx) const noexcept {
        return buffer_[flatten(std::array<std::size_t, N>{static_cast<std::size_t>(idx)...})];
    }

    // ─── Equality ─────────────────────────────────────────────────────────
    friend bool operator==(Tensor const&, Tensor const&) = default;

private:
    [[nodiscard]] std::size_t flatten(std::array<std::size_t, N> idx) const noexcept {
        // Row-major: rightmost axis varies fastest.
        std::size_t out = 0;
        std::size_t stride = 1;
        for (std::size_t k = 0; k < N; ++k) {
            std::size_t i = N - 1 - k;
            out += idx[i] * stride;
            stride *= shape_[i].extent;
        }
        return out;
    }

    Shape<N> shape_{};
    std::vector<T> buffer_{};
};

// Sanity: confirm that Tensor satisfies the TensorLike port at compile time.
// (One concrete instantiation is enough to keep concept regressions out.)
static_assert(TensorLike<Tensor<double, 1>>);
static_assert(TensorLike<Tensor<float, 3>>);

}  // namespace tensor::core
