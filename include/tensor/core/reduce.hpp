// SPDX-License-Identifier: MIT
//
// tensor::core::reduce_along_label — sum a tensor along a single named axis.
//
// Companion to the broadcast / contract primitives. Used by the `\sum_i`
// AST node in tensor::tex::Evaluator to materialise summation over a
// named axis without going through a full contraction.

#pragma once

#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "tensor/core/axis.hpp"
#include "tensor/core/broadcast.hpp"
#include "tensor/core/dynamic_shape.hpp"
#include "tensor/core/dynamic_tensor.hpp"

namespace tensor::core {

// reduce_along_label(t, label) — produce a tensor with `label` removed
// from the shape by summing over it.
//
// `Σ_i t_{...}` where `i` is at any position in t.shape().
//
// Throws std::invalid_argument if `label` is not present in t.shape().
template <class T>
[[nodiscard]] DynamicTensor<T> reduce_along_label(DynamicTensor<T> const& t,
                                                   std::string_view label) {
    std::size_t const axis_pos = t.shape().index_of(label);
    if (axis_pos == t.shape().rank()) {
        throw std::invalid_argument(
            std::string("reduce_along_label: axis '") + std::string(label) +
            "' not present in tensor shape");
    }

    // Build the result shape by dropping the reduced axis.
    std::vector<Axis> kept;
    kept.reserve(t.shape().rank() - 1);
    for (std::size_t i = 0; i < t.shape().rank(); ++i) {
        if (i == axis_pos) continue;
        kept.push_back(t.shape()[i]);
    }
    DynamicShape result_shape(std::move(kept));
    DynamicTensor<T> out(result_shape);

    if (t.size() == 0) return out;

    // Walk t's multi-index space; project to the result-multi-index by
    // dropping the reduced position; accumulate.
    std::vector<std::size_t> idx(t.shape().rank(), 0);
    do {
        // Build the result index by skipping `axis_pos`.
        std::vector<std::size_t> result_idx;
        result_idx.reserve(result_shape.rank());
        for (std::size_t i = 0; i < idx.size(); ++i) {
            if (i == axis_pos) continue;
            result_idx.push_back(idx[i]);
        }
        // out[result_idx] += t[idx]
        out.at_index(result_idx) = out.at_index(result_idx) + t.at_index(idx);
    } while (increment_index(idx, t.shape()));

    return out;
}

// reduce_along_labels(t, labels) — repeatedly reduce over each label in
// the given list. Used by `\sum_{ij} ...` where multiple indices are
// summed.
template <class T>
[[nodiscard]] DynamicTensor<T> reduce_along_labels(
    DynamicTensor<T> t,
    std::vector<std::string> const& labels) {
    for (auto const& l : labels) {
        t = reduce_along_label(t, l);
    }
    return t;
}

}  // namespace tensor::core
