// SPDX-License-Identifier: MIT
//
// tensor::core::broadcast — Einstein-style label-broadcast.
//
// Given two shapes A and B (each a sequence of named axes), the broadcast
// rule produces a result shape R according to:
//
//   - Every axis in A appears in R, in A's declaration order.
//   - Every axis in B whose label is not already present in R is appended
//     (in B's declaration order).
//   - When a label appears in both A and B, the result has a single axis
//     with that label whose extent is the matched extent. Mismatched
//     extents on a shared label raise std::invalid_argument.
//
// `broadcast_shapes` returns R alongside a `BroadcastPlan` capturing,
// for each result axis, the corresponding source axis index in A and B
// (or `npos` when the axis is unique to one side). Operators iterate the
// result indices and use the plan to pick the right element from each
// input.

#pragma once

#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

#include "tensor/core/dynamic_shape.hpp"

namespace tensor::core {

inline constexpr std::size_t broadcast_npos = static_cast<std::size_t>(-1);

struct BroadcastPlan {
    // For each result-shape axis (in result order), the source axis index
    // in A and B; broadcast_npos if absent on that side.
    std::vector<std::size_t> a_source;
    std::vector<std::size_t> b_source;
    DynamicShape result;
};

[[nodiscard]] inline BroadcastPlan broadcast_shapes(DynamicShape const& a,
                                                    DynamicShape const& b) {
    BroadcastPlan plan;
    // Track which axes of B have already been merged.
    std::vector<bool> b_used(b.rank(), false);

    // First pass: walk A in order, inheriting each axis. If B has the same
    // label, merge — verifying extents.
    for (std::size_t i = 0; i < a.rank(); ++i) {
        plan.result.push_back(a[i]);
        plan.a_source.push_back(i);
        std::size_t bj = b.index_of(a[i].label);
        if (bj != b.rank()) {
            if (b[bj].extent != a[i].extent) {
                throw std::invalid_argument(
                    "broadcast_shapes: extent mismatch on shared label");
            }
            plan.b_source.push_back(bj);
            b_used[bj] = true;
        } else {
            plan.b_source.push_back(broadcast_npos);
        }
    }

    // Second pass: append B-only axes (in B's declaration order).
    for (std::size_t j = 0; j < b.rank(); ++j) {
        if (b_used[j]) {
            continue;
        }
        plan.result.push_back(b[j]);
        plan.a_source.push_back(broadcast_npos);
        plan.b_source.push_back(j);
    }

    return plan;
}

// Helper: increment a row-major multi-index by one, given the shape's extents.
// Returns false when the iteration is exhausted.
[[nodiscard]] inline bool increment_index(std::vector<std::size_t>& idx,
                                          DynamicShape const& shape) {
    if (shape.rank() == 0) {
        return false;  // single scalar; one iteration only
    }
    for (std::size_t k = 0; k < shape.rank(); ++k) {
        std::size_t pos = shape.rank() - 1 - k;  // rightmost varies fastest
        ++idx[pos];
        if (idx[pos] < shape[pos].extent) {
            return true;
        }
        idx[pos] = 0;
    }
    return false;
}

// Project a result-space multi-index into a source-space multi-index using
// a column of the BroadcastPlan (a_source or b_source). Result axes that
// are absent on the source side (npos) are dropped.
[[nodiscard]] inline std::vector<std::size_t> project_index(
    std::vector<std::size_t> const& result_idx,
    std::vector<std::size_t> const& source_map,
    std::size_t source_rank) {
    std::vector<std::size_t> out(source_rank, 0);
    for (std::size_t r = 0; r < result_idx.size(); ++r) {
        std::size_t s = source_map[r];
        if (s != broadcast_npos) {
            out[s] = result_idx[r];
        }
    }
    return out;
}

}  // namespace tensor::core
