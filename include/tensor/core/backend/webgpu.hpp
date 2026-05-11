// SPDX-License-Identifier: MIT
//
// tensor::core::backend::webgpu — third KernelBackend adapter slot (stub).
//
// Phase 3 P3.M2. Provides the concept-satisfying Backend class so the
// architecture's third backend slot exists in code; method bodies
// currently delegate to `reference::Backend`. ADR-0012 fixed the
// implementation design (gpu.cpp on Dawn, textual WGSL emission, opaque
// GPU buffers per call, blocking sync, f32 MVP, compile-only-on-CI +
// self-hosted-GPU-for-numerical-agreement).
//
// Subsequent Phase 3 PRs replace each delegated method body with a
// real WGSL kernel + Dawn dispatch:
//
//   P3.M3 — element-wise unary/binary kernels.
//   P3.M4 — GEMM (matvec, matmul) kernels.
//   P3.M5 — broadcast / reduction / unbroadcast kernels.
//
// Until then, selecting `-DTENSOR_KERNEL_BACKEND=webgpu` gives a build
// that exercises the routing through the third slot while running on
// CPU underneath. Useful for testing the dispatch path without a GPU
// available on the developer's machine.

#pragma once

#if !defined(TENSOR_HAS_WEBGPU)
#    error "tensor/core/backend/webgpu.hpp included without TENSOR_HAS_WEBGPU; \
            configure with cmake -DTENSOR_KERNEL_BACKEND=webgpu."
#endif

#include <cstddef>
#include <utility>
#include <vector>

#include "tensor/core/backend/reference.hpp"
#include "tensor/core/concepts.hpp"
#include "tensor/core/dynamic_shape.hpp"
#include "tensor/core/dynamic_tensor.hpp"

namespace tensor::core::backend::webgpu {

// WebGPU Backend. Currently a stub: every method delegates to a private
// reference::Backend instance. Phase 3 P3.M3+ progressively replaces
// each method body with a WGSL kernel dispatched through gpu.cpp + Dawn.
class Backend {
public:
    struct backend_tag {};

    template <class T>
    [[nodiscard]] DynamicTensor<T> add(DynamicTensor<T> const& a,
                                       DynamicTensor<T> const& b) const {
        // TODO (P3.M3): emit a WGSL kernel for element-wise add and
        // dispatch through Dawn. Until then, delegate.
        return ref_.add(a, b);
    }
    template <class T>
    [[nodiscard]] DynamicTensor<T> sub(DynamicTensor<T> const& a,
                                       DynamicTensor<T> const& b) const {
        return ref_.sub(a, b);
    }
    template <class T>
    [[nodiscard]] DynamicTensor<T> mul(DynamicTensor<T> const& a,
                                       DynamicTensor<T> const& b) const {
        return ref_.mul(a, b);
    }
    template <class T>
    [[nodiscard]] DynamicTensor<T> div(DynamicTensor<T> const& a,
                                       DynamicTensor<T> const& b) const {
        return ref_.div(a, b);
    }

    template <class T>
    [[nodiscard]] DynamicTensor<T> exp(DynamicTensor<T> const& a) const {
        return ref_.exp(a);
    }
    template <class T>
    [[nodiscard]] DynamicTensor<T> log(DynamicTensor<T> const& a) const {
        return ref_.log(a);
    }
    template <class T>
    [[nodiscard]] DynamicTensor<T> relu(DynamicTensor<T> const& a) const {
        return ref_.relu(a);
    }
    template <class T>
    [[nodiscard]] DynamicTensor<T> neg(DynamicTensor<T> const& a) const {
        return ref_.neg(a);
    }

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

    template <class T>
    [[nodiscard]] DynamicTensor<T> contract(DynamicTensor<T> const& a,
                                            DynamicTensor<T> const& b,
                                            ContractPlan const& plan) const {
        // TODO (P3.M4): emit a tiled GEMM kernel for matvec / matmul
        // when ranks line up; delegate the rest. Until then, all paths
        // delegate.
        return ref_.contract(a, b, plan);
    }

    template <class T>
    [[nodiscard]] T reduce_sum(DynamicTensor<T> const& a) const {
        return ref_.reduce_sum(a);
    }

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
              "webgpu::Backend must satisfy KernelBackend");

}  // namespace tensor::core::backend::webgpu
