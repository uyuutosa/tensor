// SPDX-License-Identifier: MIT
#pragma once

#include <cstddef>
#include <initializer_list>
#include <stdexcept>
#include <utility>
#include <vector>

#include "tensor/core/dynamic_shape.hpp"
#include "tensor/core/shape.hpp"
#include "tensor/core/tensor.hpp"

namespace tensor::core {

// DynamicTensor<T> — runtime-rank tensor.
//
// Used as the canonical result type for operators that may reduce or
// expand rank based on label-broadcast (Einstein-style). Layout is
// row-major / contiguous, matching Tensor<T, N>.
template <class T>
class DynamicTensor {
public:
    using value_type = T;
    using shape_type = DynamicShape;

    DynamicTensor() = default;
    explicit DynamicTensor(DynamicShape shape)
        : shape_(std::move(shape)), buffer_(shape_.total()) {}

    DynamicTensor(DynamicShape shape, std::vector<T> buffer)
        : shape_(std::move(shape)), buffer_(std::move(buffer)) {
        if (buffer_.size() != shape_.total()) {
            throw std::invalid_argument("DynamicTensor: buffer size != shape.total()");
        }
    }

    DynamicTensor(DynamicShape shape, std::initializer_list<T> values)
        : shape_(std::move(shape)) {
        if (values.size() != shape_.total()) {
            throw std::invalid_argument("DynamicTensor: value count != shape.total()");
        }
        buffer_.assign(values.begin(), values.end());
    }

    // Conversion from static-rank Tensor<T, N>.
    template <std::size_t N>
    DynamicTensor(Tensor<T, N> const& t)
        : shape_(std::vector<Axis>(t.shape().begin(), t.shape().end())),
          buffer_(t.as_span().begin(), t.as_span().end()) {}

    // Explicit conversion to static-rank Tensor<T, N>; throws if rank mismatch.
    template <std::size_t N>
    [[nodiscard]] Tensor<T, N> to_static() const {
        if (shape_.rank() != N) {
            throw std::invalid_argument("DynamicTensor::to_static: rank mismatch");
        }
        std::array<Axis, N> arr{};
        for (std::size_t i = 0; i < N; ++i) {
            arr[i] = shape_[i];
        }
        return Tensor<T, N>(Shape<N>(arr), buffer_);
    }

    // ─── Shape / buffer ───────────────────────────────────────────────────
    [[nodiscard]] DynamicShape const& shape() const noexcept { return shape_; }
    [[nodiscard]] std::size_t size() const noexcept { return buffer_.size(); }
    [[nodiscard]] T* data() noexcept { return buffer_.data(); }
    [[nodiscard]] T const* data() const noexcept { return buffer_.data(); }

    // ─── Linear access ────────────────────────────────────────────────────
    [[nodiscard]] T& operator[](std::size_t i) noexcept { return buffer_[i]; }
    [[nodiscard]] T const& operator[](std::size_t i) const noexcept { return buffer_[i]; }

    // ─── Multi-index access ───────────────────────────────────────────────
    [[nodiscard]] T& at_index(std::vector<std::size_t> const& idx) {
        return buffer_[flatten(idx)];
    }
    [[nodiscard]] T const& at_index(std::vector<std::size_t> const& idx) const {
        return buffer_[flatten(idx)];
    }

    friend bool operator==(DynamicTensor const&, DynamicTensor const&) = default;

private:
    [[nodiscard]] std::size_t flatten(std::vector<std::size_t> const& idx) const {
        // Row-major: rightmost axis varies fastest.
        std::size_t out = 0;
        std::size_t stride = 1;
        for (std::size_t k = 0; k < shape_.rank(); ++k) {
            std::size_t i = shape_.rank() - 1 - k;
            out += idx[i] * stride;
            stride *= shape_[i].extent;
        }
        return out;
    }

    DynamicShape shape_{};
    std::vector<T> buffer_{};
};

}  // namespace tensor::core
