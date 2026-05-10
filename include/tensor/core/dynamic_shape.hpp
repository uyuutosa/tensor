// SPDX-License-Identifier: MIT
#pragma once

#include <cstddef>
#include <initializer_list>
#include <string_view>
#include <utility>
#include <vector>

#include "tensor/core/axis.hpp"

namespace tensor::core {

// DynamicShape — runtime-rank shape, used as the result type of operators
// whose output rank depends on input labels (Einstein-style broadcasting).
//
// Static-rank Shape<N> remains the primary user-facing type when a learner
// types the rank into the source code; DynamicShape exists for results.
class DynamicShape {
public:
    using axis_type = Axis;

    DynamicShape() = default;
    DynamicShape(std::vector<Axis> axes) : axes_(std::move(axes)) {}
    DynamicShape(std::initializer_list<Axis> axes) : axes_(axes) {}

    [[nodiscard]] std::size_t rank() const noexcept { return axes_.size(); }
    [[nodiscard]] std::size_t size() const noexcept { return axes_.size(); }

    [[nodiscard]] Axis const& operator[](std::size_t i) const noexcept { return axes_[i]; }
    [[nodiscard]] Axis& operator[](std::size_t i) noexcept { return axes_[i]; }

    [[nodiscard]] std::size_t total() const noexcept {
        std::size_t out = 1;
        for (auto const& ax : axes_) {
            out *= ax.extent;
        }
        return out;
    }

    [[nodiscard]] std::size_t index_of(std::string_view label) const noexcept {
        for (std::size_t i = 0; i < axes_.size(); ++i) {
            if (axes_[i].label == label) {
                return i;
            }
        }
        return axes_.size();
    }

    [[nodiscard]] bool has(std::string_view label) const noexcept {
        return index_of(label) != axes_.size();
    }

    [[nodiscard]] auto begin() const noexcept { return axes_.begin(); }
    [[nodiscard]] auto end() const noexcept { return axes_.end(); }

    [[nodiscard]] std::vector<Axis> const& axes() const noexcept { return axes_; }

    void push_back(Axis a) { axes_.push_back(std::move(a)); }

    friend bool operator==(DynamicShape const&, DynamicShape const&) = default;

private:
    std::vector<Axis> axes_{};
};

}  // namespace tensor::core
