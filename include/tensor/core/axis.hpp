// SPDX-License-Identifier: MIT
#pragma once

#include <compare>
#include <cstddef>
#include <string>
#include <string_view>

namespace tensor::core {

// Axis — runtime named axis identity (ADR-0004 default path).
//
// `label` is a small-string-optimized std::string. The NTTP compile-time
// fast path (LabelTag<"i">) introduced in M3 will store the label in the
// type system instead, but both paths satisfy the AxisLike concept.
struct Axis {
    std::string label;
    std::size_t extent{0};

    Axis() = default;
    Axis(std::string_view label_, std::size_t extent_)
        : label(label_), extent(extent_) {}

    // Two axes are equal iff label and extent match. Order is by label
    // (lexicographic) then extent — used for canonicalisation in M3.
    friend auto operator<=>(Axis const&, Axis const&) = default;
};

}  // namespace tensor::core
