// SPDX-License-Identifier: MIT
//
// tensor::core::backend::eigen — Eigen3-based KernelBackend adapter.
//
// Second concrete adapter for the KernelBackend port (ADR-0011). Provides
// fast paths for the operations Eigen excels at — SIMD element-wise on
// dense buffers, BLAS-flavoured GEMM for matmul-shape contractions —
// and delegates the rest to the reference adapter.
//
// Activated via `cmake -DTENSOR_KERNEL_BACKEND=eigen`. When that option
// is not set the macro `TENSOR_HAS_EIGEN` is undefined and this header
// must not be included; CMakeLists.txt guards the include indirectly via
// the active-backend selector.

#pragma once

#if !defined(TENSOR_HAS_EIGEN)
#    error "tensor/core/backend/eigen.hpp included without TENSOR_HAS_EIGEN; \
            configure with cmake -DTENSOR_KERNEL_BACKEND=eigen."
#endif

#include <Eigen/Core>

#include <cstddef>
#include <type_traits>
#include <utility>
#include <vector>

#include "tensor/core/backend/reference.hpp"
#include "tensor/core/concepts.hpp"
#include "tensor/core/contract.hpp"
#include "tensor/core/dynamic_shape.hpp"
#include "tensor/core/dynamic_tensor.hpp"

namespace tensor::core::backend::eigen {

// Eigen Backend — fast-paths for double on element-wise and matmul-shape
// contractions; delegates the rest to reference::Backend so the port
// surface is complete.
class Backend {
public:
    struct backend_tag {};

    // ─── Element-wise binary (same shape, double fast-path) ──────────────
    template <class T>
    [[nodiscard]] DynamicTensor<T> add(DynamicTensor<T> const& a,
                                       DynamicTensor<T> const& b) const {
        if constexpr (std::is_same_v<T, double>) {
            DynamicTensor<T> out(a.shape());
            ::Eigen::Map<::Eigen::ArrayXd const> ea(a.data(), a.size());
            ::Eigen::Map<::Eigen::ArrayXd const> eb(b.data(), b.size());
            ::Eigen::Map<::Eigen::ArrayXd> eo(out.data(), out.size());
            eo = ea + eb;
            return out;
        } else {
            return ref_.add(a, b);
        }
    }

    template <class T>
    [[nodiscard]] DynamicTensor<T> sub(DynamicTensor<T> const& a,
                                       DynamicTensor<T> const& b) const {
        if constexpr (std::is_same_v<T, double>) {
            DynamicTensor<T> out(a.shape());
            ::Eigen::Map<::Eigen::ArrayXd const> ea(a.data(), a.size());
            ::Eigen::Map<::Eigen::ArrayXd const> eb(b.data(), b.size());
            ::Eigen::Map<::Eigen::ArrayXd>(out.data(), out.size()) = ea - eb;
            return out;
        } else {
            return ref_.sub(a, b);
        }
    }

    template <class T>
    [[nodiscard]] DynamicTensor<T> mul(DynamicTensor<T> const& a,
                                       DynamicTensor<T> const& b) const {
        if constexpr (std::is_same_v<T, double>) {
            DynamicTensor<T> out(a.shape());
            ::Eigen::Map<::Eigen::ArrayXd const> ea(a.data(), a.size());
            ::Eigen::Map<::Eigen::ArrayXd const> eb(b.data(), b.size());
            ::Eigen::Map<::Eigen::ArrayXd>(out.data(), out.size()) = ea * eb;
            return out;
        } else {
            return ref_.mul(a, b);
        }
    }

    template <class T>
    [[nodiscard]] DynamicTensor<T> div(DynamicTensor<T> const& a,
                                       DynamicTensor<T> const& b) const {
        if constexpr (std::is_same_v<T, double>) {
            DynamicTensor<T> out(a.shape());
            ::Eigen::Map<::Eigen::ArrayXd const> ea(a.data(), a.size());
            ::Eigen::Map<::Eigen::ArrayXd const> eb(b.data(), b.size());
            ::Eigen::Map<::Eigen::ArrayXd>(out.data(), out.size()) = ea / eb;
            return out;
        } else {
            return ref_.div(a, b);
        }
    }

    // ─── Element-wise unary (double fast-path) ───────────────────────────
    template <class T>
    [[nodiscard]] DynamicTensor<T> exp(DynamicTensor<T> const& a) const {
        if constexpr (std::is_same_v<T, double>) {
            DynamicTensor<T> out(a.shape());
            ::Eigen::Map<::Eigen::ArrayXd const> ea(a.data(), a.size());
            ::Eigen::Map<::Eigen::ArrayXd>(out.data(), out.size()) = ea.exp();
            return out;
        } else {
            return ref_.exp(a);
        }
    }

    template <class T>
    [[nodiscard]] DynamicTensor<T> log(DynamicTensor<T> const& a) const {
        if constexpr (std::is_same_v<T, double>) {
            DynamicTensor<T> out(a.shape());
            ::Eigen::Map<::Eigen::ArrayXd const> ea(a.data(), a.size());
            ::Eigen::Map<::Eigen::ArrayXd>(out.data(), out.size()) = ea.log();
            return out;
        } else {
            return ref_.log(a);
        }
    }

    template <class T>
    [[nodiscard]] DynamicTensor<T> relu(DynamicTensor<T> const& a) const {
        if constexpr (std::is_same_v<T, double>) {
            DynamicTensor<T> out(a.shape());
            ::Eigen::Map<::Eigen::ArrayXd const> ea(a.data(), a.size());
            ::Eigen::Map<::Eigen::ArrayXd>(out.data(), out.size()) = ea.cwiseMax(0.0);
            return out;
        } else {
            return ref_.relu(a);
        }
    }

    template <class T>
    [[nodiscard]] DynamicTensor<T> neg(DynamicTensor<T> const& a) const {
        if constexpr (std::is_same_v<T, double>) {
            DynamicTensor<T> out(a.shape());
            ::Eigen::Map<::Eigen::ArrayXd const> ea(a.data(), a.size());
            ::Eigen::Map<::Eigen::ArrayXd>(out.data(), out.size()) = -ea;
            return out;
        } else {
            return ref_.neg(a);
        }
    }

    // ─── Broadcast element-wise — delegate ────────────────────────────────
    //
    // Eigen's broadcasting works on fixed-shape arrays; mapping the
    // named-axis BroadcastPlan onto it cleanly is a future fast-path.
    // For MVP we delegate; the per-op walker in reference::Backend already
    // does the right thing.
    template <class T>
    [[nodiscard]] DynamicTensor<T> broadcast_add(DynamicTensor<T> const& a,
                                                 DynamicTensor<T> const& b,
                                                 BroadcastPlan const& plan) const {
        return ref_.broadcast_add(a, b, plan);
    }

    template <class T>
    [[nodiscard]] DynamicTensor<T> broadcast_sub(DynamicTensor<T> const& a,
                                                 DynamicTensor<T> const& b,
                                                 BroadcastPlan const& plan) const {
        return ref_.broadcast_sub(a, b, plan);
    }

    template <class T>
    [[nodiscard]] DynamicTensor<T> broadcast_mul(DynamicTensor<T> const& a,
                                                 DynamicTensor<T> const& b,
                                                 BroadcastPlan const& plan) const {
        return ref_.broadcast_mul(a, b, plan);
    }

    // ─── Contraction (Eigen GEMM fast-path for matvec / matmul) ──────────
    //
    // Fast path covers:
    //   - a rank-2, b rank-1, single shared axis → matvec via Eigen GEMV.
    //   - a rank-2, b rank-2, single shared axis → matmul via Eigen GEMM.
    // Everything else (higher-rank contractions, no shared axis, multiple
    // shared axes) delegates to reference.
    template <class T>
    [[nodiscard]] DynamicTensor<T> contract(DynamicTensor<T> const& a,
                                            DynamicTensor<T> const& b,
                                            ContractPlan const& plan) const {
        if constexpr (std::is_same_v<T, double>) {
            std::size_t const ar = a.shape().rank();
            std::size_t const br = b.shape().rank();
            std::size_t const sr = plan.shared.rank();
            if (sr == 1 && ar == 2 && br == 1) {
                // matvec; shared axis position in A and B given by the plan.
                std::size_t const a_share = plan.a_shared_source[0];
                std::size_t const a_kept  = a_share == 0 ? 1u : 0u;
                std::size_t const rows = a.shape()[a_kept].extent;
                std::size_t const cols = a.shape()[a_share].extent;
                DynamicTensor<T> out(plan.result);
                // Eigen Matrix Map is column-major by default; our buffer is
                // row-major. Use RowMajor layout via stride or transpose.
                ::Eigen::Map<::Eigen::Matrix<double, ::Eigen::Dynamic, ::Eigen::Dynamic,
                                             ::Eigen::RowMajor> const>
                    eA(a.data(), static_cast<::Eigen::Index>(rows),
                                  static_cast<::Eigen::Index>(cols));
                ::Eigen::Map<::Eigen::VectorXd const> eB(
                    b.data(), static_cast<::Eigen::Index>(cols));
                ::Eigen::Map<::Eigen::VectorXd> eO(
                    out.data(), static_cast<::Eigen::Index>(rows));
                // If a_kept is the first axis (a_share == 1), no transpose needed.
                // If a_kept is the second axis (a_share == 0), we must
                // contract over the first dimension; do so via transpose.
                if (a_share == 1) {
                    eO.noalias() = eA * eB;
                } else {
                    eO.noalias() = eA.transpose() * eB;
                }
                return out;
            }
            if (sr == 1 && ar == 2 && br == 2) {
                // matmul; rebuild RowMajor maps. Determine the shared axis
                // positions and the kept axes in A and B.
                std::size_t const a_share = plan.a_shared_source[0];
                std::size_t const b_share = plan.b_shared_source[0];
                std::size_t const a_kept  = a_share == 0 ? 1u : 0u;
                std::size_t const b_kept  = b_share == 0 ? 1u : 0u;
                std::size_t const m = a.shape()[a_kept].extent;
                std::size_t const k = a.shape()[a_share].extent;
                std::size_t const n = b.shape()[b_kept].extent;
                DynamicTensor<T> out(plan.result);
                ::Eigen::Map<::Eigen::Matrix<double, ::Eigen::Dynamic, ::Eigen::Dynamic,
                                             ::Eigen::RowMajor> const>
                    eA(a.data(),
                       static_cast<::Eigen::Index>(a.shape()[0].extent),
                       static_cast<::Eigen::Index>(a.shape()[1].extent));
                ::Eigen::Map<::Eigen::Matrix<double, ::Eigen::Dynamic, ::Eigen::Dynamic,
                                             ::Eigen::RowMajor> const>
                    eB(b.data(),
                       static_cast<::Eigen::Index>(b.shape()[0].extent),
                       static_cast<::Eigen::Index>(b.shape()[1].extent));
                ::Eigen::Map<::Eigen::Matrix<double, ::Eigen::Dynamic, ::Eigen::Dynamic,
                                             ::Eigen::RowMajor>>
                    eO(out.data(),
                       static_cast<::Eigen::Index>(m),
                       static_cast<::Eigen::Index>(n));
                // We need C[i, k] = Σ_j A[i, j] * B[j, k]. Eigen will produce
                // that when we feed it the canonical (i, j) × (j, k) layout.
                // Use transposes to match.
                auto eA_can = (a_share == 1) ? eA : eA.transpose().eval();
                auto eB_can = (b_share == 0) ? eB : eB.transpose().eval();
                eO.noalias() = eA_can * eB_can;
                static_cast<void>(k);  // suppress unused on some compilers
                return out;
            }
            // Other rank / shared combinations: reference fallback.
            return ref_.contract(a, b, plan);
        } else {
            return ref_.contract(a, b, plan);
        }
    }

    // ─── Reduction (Eigen sum for double) ────────────────────────────────
    template <class T>
    [[nodiscard]] T reduce_sum(DynamicTensor<T> const& a) const {
        if constexpr (std::is_same_v<T, double>) {
            ::Eigen::Map<::Eigen::ArrayXd const> ea(a.data(), a.size());
            return ea.sum();
        } else {
            return ref_.reduce_sum(a);
        }
    }

    // ─── Unbroadcast — delegate ──────────────────────────────────────────
    template <class T>
    [[nodiscard]] DynamicTensor<T> unbroadcast(
        DynamicTensor<T> const& a,
        std::vector<std::size_t> const& source_map,
        DynamicShape const& source_shape) const {
        return ref_.unbroadcast(a, source_map, source_shape);
    }

private:
    tensor::core::backend::reference::Backend ref_;
};

static_assert(KernelBackend<Backend>,
              "eigen::Backend must satisfy KernelBackend");

}  // namespace tensor::core::backend::eigen
