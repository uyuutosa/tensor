// SPDX-License-Identifier: MIT
//
// tensor::core::backend::reference — the reference CPU adapter.
//
// First concrete implementation of the KernelBackend port (ADR-0011).
// All 15 methods delegate to the existing core utilities in
// broadcast.hpp, contract.hpp, and a few inline element-wise loops.
//
// This adapter exists primarily as:
//   - the always-available default backend (no third-party dependency);
//   - the reference against which other adapters (Eigen, Kokkos, WebGPU)
//     are tested for numerical agreement;
//   - the template that contributors copy when starting a new adapter.

#pragma once

#include <cmath>
#include <cstddef>
#include <utility>
#include <vector>

#include "tensor/core/broadcast.hpp"
#include "tensor/core/concepts.hpp"
#include "tensor/core/contract.hpp"
#include "tensor/core/dynamic_shape.hpp"
#include "tensor/core/dynamic_tensor.hpp"

namespace tensor::core::backend::reference {

class Backend {
public:
    // KernelBackend marker tag.
    struct backend_tag {};

    // ─── Element-wise binary (same shape) ─────────────────────────────────
    template <class T>
    [[nodiscard]] DynamicTensor<T> add(DynamicTensor<T> const& a,
                                       DynamicTensor<T> const& b) const {
        DynamicTensor<T> out(a.shape());
        for (std::size_t i = 0; i < a.size(); ++i) out[i] = a[i] + b[i];
        return out;
    }

    template <class T>
    [[nodiscard]] DynamicTensor<T> sub(DynamicTensor<T> const& a,
                                       DynamicTensor<T> const& b) const {
        DynamicTensor<T> out(a.shape());
        for (std::size_t i = 0; i < a.size(); ++i) out[i] = a[i] - b[i];
        return out;
    }

    template <class T>
    [[nodiscard]] DynamicTensor<T> mul(DynamicTensor<T> const& a,
                                       DynamicTensor<T> const& b) const {
        DynamicTensor<T> out(a.shape());
        for (std::size_t i = 0; i < a.size(); ++i) out[i] = a[i] * b[i];
        return out;
    }

    template <class T>
    [[nodiscard]] DynamicTensor<T> div(DynamicTensor<T> const& a,
                                       DynamicTensor<T> const& b) const {
        DynamicTensor<T> out(a.shape());
        for (std::size_t i = 0; i < a.size(); ++i) out[i] = a[i] / b[i];
        return out;
    }

    // ─── Element-wise unary ───────────────────────────────────────────────
    template <class T>
    [[nodiscard]] DynamicTensor<T> exp(DynamicTensor<T> const& a) const {
        DynamicTensor<T> out(a.shape());
        for (std::size_t i = 0; i < a.size(); ++i) out[i] = std::exp(a[i]);
        return out;
    }

    template <class T>
    [[nodiscard]] DynamicTensor<T> log(DynamicTensor<T> const& a) const {
        DynamicTensor<T> out(a.shape());
        for (std::size_t i = 0; i < a.size(); ++i) out[i] = std::log(a[i]);
        return out;
    }

    template <class T>
    [[nodiscard]] DynamicTensor<T> relu(DynamicTensor<T> const& a) const {
        DynamicTensor<T> out(a.shape());
        for (std::size_t i = 0; i < a.size(); ++i) out[i] = a[i] > T{0} ? a[i] : T{0};
        return out;
    }

    template <class T>
    [[nodiscard]] DynamicTensor<T> neg(DynamicTensor<T> const& a) const {
        DynamicTensor<T> out(a.shape());
        for (std::size_t i = 0; i < a.size(); ++i) out[i] = -a[i];
        return out;
    }

    // ─── Broadcast element-wise ───────────────────────────────────────────
    //
    // The Domain computes the BroadcastPlan once and hands it to the
    // backend; the backend walks the result-shape multi-index using
    // project_index from broadcast.hpp.
    template <class T>
    [[nodiscard]] DynamicTensor<T> broadcast_add(DynamicTensor<T> const& a,
                                                 DynamicTensor<T> const& b,
                                                 BroadcastPlan const& plan) const {
        return apply_broadcast(a, b, plan, [](T x, T y) { return x + y; });
    }

    template <class T>
    [[nodiscard]] DynamicTensor<T> broadcast_sub(DynamicTensor<T> const& a,
                                                 DynamicTensor<T> const& b,
                                                 BroadcastPlan const& plan) const {
        return apply_broadcast(a, b, plan, [](T x, T y) { return x - y; });
    }

    template <class T>
    [[nodiscard]] DynamicTensor<T> broadcast_mul(DynamicTensor<T> const& a,
                                                 DynamicTensor<T> const& b,
                                                 BroadcastPlan const& plan) const {
        return apply_broadcast(a, b, plan, [](T x, T y) { return x * y; });
    }

    // ─── Contraction ──────────────────────────────────────────────────────
    template <class T>
    [[nodiscard]] DynamicTensor<T> contract(DynamicTensor<T> const& a,
                                            DynamicTensor<T> const& b,
                                            ContractPlan const& plan) const {
        return contract_with_plan(a, b, plan);
    }

    // ─── Reduction ────────────────────────────────────────────────────────
    template <class T>
    [[nodiscard]] T reduce_sum(DynamicTensor<T> const& a) const {
        T s{};
        for (std::size_t i = 0; i < a.size(); ++i) s = s + a[i];
        return s;
    }

    // ─── Unbroadcast ──────────────────────────────────────────────────────
    template <class T>
    [[nodiscard]] DynamicTensor<T> unbroadcast(
        DynamicTensor<T> const& a,
        std::vector<std::size_t> const& source_map,
        DynamicShape const& source_shape) const {
        return tensor::core::unbroadcast(a, source_map, source_shape);
    }

private:
    template <class T, class Op>
    [[nodiscard]] static DynamicTensor<T> apply_broadcast(
        DynamicTensor<T> const& a,
        DynamicTensor<T> const& b,
        BroadcastPlan const& plan,
        Op op) {
        DynamicTensor<T> out(plan.result);
        if (out.size() == 0) return out;
        std::vector<std::size_t> idx(plan.result.rank(), 0);
        std::size_t flat = 0;
        do {
            auto ai = project_index(idx, plan.a_source, a.shape().rank());
            auto bi = project_index(idx, plan.b_source, b.shape().rank());
            out[flat++] = op(a.at_index(ai), b.at_index(bi));
        } while (increment_index(idx, plan.result));
        return out;
    }
};

// Compile-time verification that the reference adapter satisfies the
// KernelBackend port. If this fails, the port surface and the adapter
// have diverged — fix one or the other before merging.
static_assert(KernelBackend<Backend>,
              "reference::Backend must satisfy KernelBackend");

}  // namespace tensor::core::backend::reference
