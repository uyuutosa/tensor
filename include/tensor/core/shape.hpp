// SPDX-License-Identifier: MIT
#pragma once

#include <array>
#include <cstddef>
#include <initializer_list>
#include <stdexcept>
#include <string_view>

#include "tensor/core/axis.hpp"

namespace tensor::core {

// Shape<N> — ordered list of N Axis entries with a compile-time rank.
//
// Rank-0 is permitted (scalar) but currently uninteresting; rank ≥ 1 is the
// expected case. Construction from `{Axis{...}, Axis{...}}` is the canonical
// shape-literal entry point.
template <std::size_t N>
class Shape {
public:
    static constexpr std::size_t rank = N;
    using axis_type = Axis;

    // Default — all axes have empty label and zero extent. Useful as a
    // placeholder; not directly meaningful as a runtime shape.
    constexpr Shape() = default;

    // Construct from N axes provided in declaration order. The static_cast
    // is to silence narrowing warnings on platforms where size_t > unsigned.
    constexpr Shape(std::array<Axis, N> axes) : axes_(std::move(axes)) {}

    // Convenience: list-initialise.  Throws std::invalid_argument if the
    // initializer_list size disagrees with N.
    Shape(std::initializer_list<Axis> axes) {
        if (axes.size() != N) {
            throw std::invalid_argument("Shape<N>: axis count mismatch");
        }
        std::size_t i = 0;
        for (auto const& ax : axes) {
            axes_[i++] = ax;
        }
    }

    // Element access.
    [[nodiscard]] constexpr Axis const& operator[](std::size_t i) const noexcept {
        return axes_[i];
    }
    [[nodiscard]] constexpr Axis& operator[](std::size_t i) noexcept { return axes_[i]; }

    // ShapeLike requires .size() — return rank as a runtime value.
    [[nodiscard]] static constexpr std::size_t size() noexcept { return N; }

    // Total element count = product of extents.
    [[nodiscard]] constexpr std::size_t total() const noexcept {
        if constexpr (N == 0) {
            return 1;  // empty product
        } else {
            std::size_t out = 1;
            for (std::size_t i = 0; i < N; ++i) {
                out *= axes_[i].extent;
            }
            return out;
        }
    }

    // Find the position of the axis with the given label, or N if not found.
    [[nodiscard]] constexpr std::size_t index_of(std::string_view label) const noexcept {
        for (std::size_t i = 0; i < N; ++i) {
            if (axes_[i].label == label) {
                return i;
            }
        }
        return N;
    }

    [[nodiscard]] constexpr bool has(std::string_view label) const noexcept {
        return index_of(label) != N;
    }

    // STL-ish iteration over the underlying axis array.
    [[nodiscard]] constexpr auto begin() const noexcept { return axes_.begin(); }
    [[nodiscard]] constexpr auto end() const noexcept { return axes_.end(); }

    friend bool operator==(Shape const&, Shape const&) = default;

private:
    std::array<Axis, N> axes_{};
};

// Deduction guide so `Shape{Axis{"i", 5}, Axis{"j", 3}}` infers Shape<2>.
template <class... Axes>
Shape(Axes...) -> Shape<sizeof...(Axes)>;

}  // namespace tensor::core
