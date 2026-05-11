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

#include <array>
#include <cstddef>
#include <type_traits>
#include <utility>
#include <vector>

#include "tensor/core/backend/reference.hpp"
#include "tensor/core/backend/webgpu_detail/context.hpp"
#include "tensor/core/backend/webgpu_detail/dispatch.hpp"
#include "tensor/core/backend/webgpu_wgsl.hpp"
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

    // ── Binary element-wise (P3.M3.2 — real Dawn dispatch) ────────────
    //
    // For T = float we dispatch through Dawn (ADR-0016: webgpu_cpp.h
    // directly). Other types delegate to reference per ADR-0012 § f32-only
    // MVP. The WGSL kernel sources are PR #43's binary set.

    template <class T>
    [[nodiscard]] DynamicTensor<T> add(DynamicTensor<T> const& a,
                                       DynamicTensor<T> const& b) const {
        return binary_op<T>(a, b, wgsl::kAddF32, &reference::Backend::add<T>);
    }
    template <class T>
    [[nodiscard]] DynamicTensor<T> sub(DynamicTensor<T> const& a,
                                       DynamicTensor<T> const& b) const {
        return binary_op<T>(a, b, wgsl::kSubF32, &reference::Backend::sub<T>);
    }
    template <class T>
    [[nodiscard]] DynamicTensor<T> mul(DynamicTensor<T> const& a,
                                       DynamicTensor<T> const& b) const {
        return binary_op<T>(a, b, wgsl::kMulF32, &reference::Backend::mul<T>);
    }
    template <class T>
    [[nodiscard]] DynamicTensor<T> div(DynamicTensor<T> const& a,
                                       DynamicTensor<T> const& b) const {
        return binary_op<T>(a, b, wgsl::kDivF32, &reference::Backend::div<T>);
    }

    // ── Unary element-wise (P3.M3.2 — real Dawn dispatch for f32) ─────
    //
    // The WGSL kernel sources are PR #44's unary set.

    template <class T>
    [[nodiscard]] DynamicTensor<T> exp(DynamicTensor<T> const& a) const {
        return unary_op<T>(a, wgsl::kExpF32, &reference::Backend::exp<T>);
    }
    template <class T>
    [[nodiscard]] DynamicTensor<T> log(DynamicTensor<T> const& a) const {
        return unary_op<T>(a, wgsl::kLogF32, &reference::Backend::log<T>);
    }
    template <class T>
    [[nodiscard]] DynamicTensor<T> relu(DynamicTensor<T> const& a) const {
        return unary_op<T>(a, wgsl::kReluF32, &reference::Backend::relu<T>);
    }
    template <class T>
    [[nodiscard]] DynamicTensor<T> neg(DynamicTensor<T> const& a) const {
        return unary_op<T>(a, wgsl::kNegF32, &reference::Backend::neg<T>);
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
        // P3.M4.1 shipped the tiled GEMM WGSL source at
        // `tensor::core::backend::webgpu::wgsl::kGemmF32`. P3.M4.2
        // replaces this delegation with the gpu.cpp dispatch sequence
        // specified in docs/detailed-design/webgpu-gemm-kernel.md §3 —
        // gates on the call being a single-shared-axis matvec or matmul
        // and delegates anything else to reference per ADR-0012.
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

    // Shared dispatch logic for binary element-wise ops on T = float.
    // Other types fall back to the reference adapter. `ref_method` is
    // the pointer-to-member-function on reference::Backend for the
    // fallback (e.g. &reference::Backend::add<T>).
    template <class T, class RefFn>
    [[nodiscard]] DynamicTensor<T> binary_op(DynamicTensor<T> const& a,
                                             DynamicTensor<T> const& b,
                                             std::string_view wgsl_template,
                                             RefFn ref_method) const {
        if constexpr (!std::is_same_v<T, float>) {
            return (ref_.*ref_method)(a, b);
        } else {
            const std::size_t n = a.size();
            const std::size_t bytes = n * sizeof(float);
            auto& ctx = detail::WebGPUContext::current();

            auto gA = detail::make_input_buffer(ctx.device(), bytes);
            auto gB = detail::make_input_buffer(ctx.device(), bytes);
            auto gOut = detail::make_output_buffer(ctx.device(), bytes);

            ctx.queue().WriteBuffer(gA, 0, a.data(), bytes);
            ctx.queue().WriteBuffer(gB, 0, b.data(), bytes);

            detail::dispatch_element_wise<3>(
                ctx, wgsl_template, std::array{gA, gB, gOut}, n,
                wgsl::kDefaultWorkgroupSize);

            DynamicTensor<T> out{a.shape()};
            detail::copy_buffer_to_host(ctx, gOut, out.data(), bytes);
            return out;
        }
    }

    // Shared dispatch logic for unary element-wise ops on T = float.
    template <class T, class RefFn>
    [[nodiscard]] DynamicTensor<T> unary_op(DynamicTensor<T> const& a,
                                            std::string_view wgsl_template,
                                            RefFn ref_method) const {
        if constexpr (!std::is_same_v<T, float>) {
            return (ref_.*ref_method)(a);
        } else {
            const std::size_t n = a.size();
            const std::size_t bytes = n * sizeof(float);
            auto& ctx = detail::WebGPUContext::current();

            auto gA = detail::make_input_buffer(ctx.device(), bytes);
            auto gOut = detail::make_output_buffer(ctx.device(), bytes);

            ctx.queue().WriteBuffer(gA, 0, a.data(), bytes);

            detail::dispatch_element_wise<2>(
                ctx, wgsl_template, std::array{gA, gOut}, n,
                wgsl::kDefaultWorkgroupSize);

            DynamicTensor<T> out{a.shape()};
            detail::copy_buffer_to_host(ctx, gOut, out.data(), bytes);
            return out;
        }
    }
};

static_assert(KernelBackend<Backend>,
              "webgpu::Backend must satisfy KernelBackend");

}  // namespace tensor::core::backend::webgpu
