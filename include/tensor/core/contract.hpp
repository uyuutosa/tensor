// SPDX-License-Identifier: MIT
//
// tensor::core::contract — Einstein-style tensor contraction.
//
// `contract(a, b)` sums over the axis labels that appear in both `a` and
// `b`. Axis labels that appear in only one input survive into the result.
//
// Example:
//   a has axes (i, k); b has axes (k, j); shared = {k}.
//   result has axes (i, j) and result[i, j] = Σ_k a[i, k] * b[k, j].
//
// Matched-label extent disagreement → std::invalid_argument.

#pragma once

#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

#include "tensor/core/dynamic_shape.hpp"
#include "tensor/core/dynamic_tensor.hpp"
#include "tensor/core/broadcast.hpp"

namespace tensor::core {

struct ContractPlan {
    DynamicShape result;          // axes (A \ shared) then (B \ shared) in order of appearance
    DynamicShape shared;          // the shared axes in A's declaration order
    // For each result-axis in `result`, the source index in A (or npos)
    // and B (or npos). Exactly one of the two is non-npos.
    std::vector<std::size_t> a_kept_source;
    std::vector<std::size_t> b_kept_source;
    // For each shared axis, the corresponding axis index in A and B.
    std::vector<std::size_t> a_shared_source;
    std::vector<std::size_t> b_shared_source;
};

[[nodiscard]] inline ContractPlan contract_plan(DynamicShape const& a, DynamicShape const& b) {
    ContractPlan plan;
    std::vector<bool> b_used(b.rank(), false);

    // Walk A: classify each A-axis as kept (no match in B) or shared.
    for (std::size_t i = 0; i < a.rank(); ++i) {
        std::size_t bj = b.index_of(a[i].label);
        if (bj == b.rank()) {
            // Kept: appears only on A side.
            plan.result.push_back(a[i]);
            plan.a_kept_source.push_back(i);
            plan.b_kept_source.push_back(broadcast_npos);
        } else {
            if (b[bj].extent != a[i].extent) {
                throw std::invalid_argument(
                    "contract: extent mismatch on shared axis label");
            }
            plan.shared.push_back(a[i]);
            plan.a_shared_source.push_back(i);
            plan.b_shared_source.push_back(bj);
            b_used[bj] = true;
        }
    }
    // Walk B: append unmatched B-axes to the result.
    for (std::size_t j = 0; j < b.rank(); ++j) {
        if (b_used[j]) continue;
        plan.result.push_back(b[j]);
        plan.a_kept_source.push_back(broadcast_npos);
        plan.b_kept_source.push_back(j);
    }
    return plan;
}

namespace detail {

// Project a result-multi-index into A and B kept-axis indices.
[[nodiscard]] inline std::vector<std::size_t> project_kept(
    std::vector<std::size_t> const& result_idx,
    std::vector<std::size_t> const& kept_source,
    std::size_t side_rank) {
    std::vector<std::size_t> out(side_rank, 0);
    for (std::size_t r = 0; r < result_idx.size(); ++r) {
        std::size_t s = kept_source[r];
        if (s != broadcast_npos) {
            out[s] = result_idx[r];
        }
    }
    return out;
}

// Place shared-axis indices into a side's full index vector.
inline void merge_shared(std::vector<std::size_t>& side_idx,
                          std::vector<std::size_t> const& shared_idx,
                          std::vector<std::size_t> const& shared_source) {
    for (std::size_t s = 0; s < shared_idx.size(); ++s) {
        side_idx[shared_source[s]] = shared_idx[s];
    }
}

}  // namespace detail

// Execute a contraction given a pre-computed ContractPlan. Useful for
// backends that want to reuse the plan across multiple invocations and
// for the KernelBackend port (ADR-0011) which receives the plan from the
// Domain rather than computing it itself.
template <class T>
[[nodiscard]] DynamicTensor<T> contract_with_plan(DynamicTensor<T> const& a,
                                                  DynamicTensor<T> const& b,
                                                  ContractPlan const& plan) {
    DynamicTensor<T> out(plan.result);
    if (out.size() == 0) return out;

    std::vector<std::size_t> result_idx(plan.result.rank(), 0);
    std::size_t flat = 0;
    do {
        auto a_full = detail::project_kept(result_idx, plan.a_kept_source, a.shape().rank());
        auto b_full = detail::project_kept(result_idx, plan.b_kept_source, b.shape().rank());

        T sum{};
        if (plan.shared.rank() == 0) {
            // No shared axes: this is an outer product. Single term.
            sum = a.at_index(a_full) * b.at_index(b_full);
        } else {
            std::vector<std::size_t> shared_idx(plan.shared.rank(), 0);
            do {
                detail::merge_shared(a_full, shared_idx, plan.a_shared_source);
                detail::merge_shared(b_full, shared_idx, plan.b_shared_source);
                sum = sum + a.at_index(a_full) * b.at_index(b_full);
            } while (increment_index(shared_idx, plan.shared));
        }
        out[flat++] = sum;
    } while (increment_index(result_idx, plan.result));
    return out;
}

// Convenience overload: compute the plan internally then dispatch to
// contract_with_plan. Existing callers (autograd::dot, test_contract,
// tutorial 00) use this form.
template <class T>
[[nodiscard]] DynamicTensor<T> contract(DynamicTensor<T> const& a,
                                        DynamicTensor<T> const& b) {
    return contract_with_plan(a, b, contract_plan(a.shape(), b.shape()));
}

}  // namespace tensor::core
