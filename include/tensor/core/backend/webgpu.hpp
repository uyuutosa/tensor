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

    // ── Broadcast element-wise (P3.M5 — real Dawn dispatch for f32) ────
    //
    // Generalised element-wise with Einstein-style broadcasting; the
    // shipped WGSL source (`kBroadcastAddF32` etc.) consumes the
    // BroadcastPlan via a uniform buffer (BroadcastParams). Max rank
    // supported by the kernel: kBroadcastMaxRank (8); the project's
    // tests / tutorials use rank ≤ 3 in practice.

    template <class T>
    [[nodiscard]] DynamicTensor<T> broadcast_add(DynamicTensor<T> const& a,
                                                 DynamicTensor<T> const& b,
                                                 BroadcastPlan const& plan) const {
        return broadcast_op<T>(a, b, plan, wgsl::kBroadcastAddF32, "+",
                               &reference::Backend::broadcast_add<T>);
    }
    template <class T>
    [[nodiscard]] DynamicTensor<T> broadcast_sub(DynamicTensor<T> const& a,
                                                 DynamicTensor<T> const& b,
                                                 BroadcastPlan const& plan) const {
        return broadcast_op<T>(a, b, plan, wgsl::kBroadcastSubF32, "-",
                               &reference::Backend::broadcast_sub<T>);
    }
    template <class T>
    [[nodiscard]] DynamicTensor<T> broadcast_mul(DynamicTensor<T> const& a,
                                                 DynamicTensor<T> const& b,
                                                 BroadcastPlan const& plan) const {
        return broadcast_op<T>(a, b, plan, wgsl::kBroadcastMulF32, "*",
                               &reference::Backend::broadcast_mul<T>);
    }

    template <class T>
    [[nodiscard]] DynamicTensor<T> contract(DynamicTensor<T> const& a,
                                            DynamicTensor<T> const& b,
                                            ContractPlan const& plan) const {
        // P3.M4.2 — Real Dawn dispatch of the tiled GEMM kernel
        // (`kGemmF32`, PR #46) for the canonical simple-GEMM case:
        //   - exactly one shared axis,
        //   - a has rank 2 with the shared axis as its LAST axis,
        //   - b has rank 1 or 2 with the shared axis as its FIRST axis,
        //   - T == float (ADR-0012 § f32-only MVP).
        // Other shapes (multi-shared-axis, higher-rank, transposed-B)
        // delegate to reference, matching the Eigen adapter's scope.
        if constexpr (!std::is_same_v<T, float>) {
            return ref_.contract(a, b, plan);
        } else {
            const bool simple_gemm =
                plan.shared.rank() == 1 &&
                a.shape().rank() == 2 &&
                (b.shape().rank() == 1 || b.shape().rank() == 2) &&
                plan.a_shared_source.size() == 1 &&
                plan.b_shared_source.size() == 1 &&
                plan.a_shared_source[0] == a.shape().rank() - 1 &&
                plan.b_shared_source[0] == 0;
            if (!simple_gemm) {
                return ref_.contract(a, b, plan);
            }

            const std::size_t M = a.shape()[0].extent;
            const std::size_t K = a.shape()[a.shape().rank() - 1].extent;
            const std::size_t N = (b.shape().rank() == 2)
                                      ? b.shape()[1].extent
                                      : std::size_t{1};

            auto& ctx = detail::WebGPUContext::current();
            const std::size_t a_bytes = M * K * sizeof(float);
            const std::size_t b_bytes = K * N * sizeof(float);
            const std::size_t out_bytes = M * N * sizeof(float);

            auto gA = detail::make_input_buffer(ctx.device(), a_bytes);
            auto gB = detail::make_input_buffer(ctx.device(), b_bytes);
            auto gOut = detail::make_output_buffer(ctx.device(), out_bytes);

            ctx.queue().WriteBuffer(gA, 0, a.data(), a_bytes);
            ctx.queue().WriteBuffer(gB, 0, b.data(), b_bytes);

            detail::dispatch_gemm(ctx, wgsl::kGemmF32, gA, gB, gOut, M, N, K,
                                  wgsl::kGemmTileM, wgsl::kGemmTileN);

            DynamicTensor<T> out{plan.result};
            detail::copy_buffer_to_host(ctx, gOut, out.data(), out_bytes);
            return out;
        }
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

    // Shared dispatch logic for broadcast element-wise ops on T = float.
    // Non-float dtypes and shapes exceeding kBroadcastMaxRank delegate.
    template <class T, class RefFn>
    [[nodiscard]] DynamicTensor<T> broadcast_op(DynamicTensor<T> const& a,
                                                DynamicTensor<T> const& b,
                                                BroadcastPlan const& plan,
                                                std::string_view wgsl_template,
                                                std::string_view op_token,
                                                RefFn ref_method) const {
        if constexpr (!std::is_same_v<T, float>) {
            return (ref_.*ref_method)(a, b, plan);
        } else {
            if (plan.result.rank() > wgsl::kBroadcastMaxRank ||
                a.shape().rank() > wgsl::kBroadcastMaxRank ||
                b.shape().rank() > wgsl::kBroadcastMaxRank) {
                return (ref_.*ref_method)(a, b, plan);
            }

            // Pack the BroadcastPlan into the kernel's uniform layout.
            detail::BroadcastParams params{};
            params.result_rank = static_cast<std::uint32_t>(plan.result.rank());
            params.a_rank = static_cast<std::uint32_t>(a.shape().rank());
            params.b_rank = static_cast<std::uint32_t>(b.shape().rank());
            for (std::size_t r = 0; r < plan.result.rank(); ++r) {
                params.result_extents[r] =
                    static_cast<std::uint32_t>(plan.result[r].extent);
                const std::size_t as = plan.a_source[r];
                const std::size_t bs = plan.b_source[r];
                params.a_source[r] = (as == broadcast_npos)
                                         ? 0xFFFFFFFFu
                                         : static_cast<std::uint32_t>(as);
                params.b_source[r] = (bs == broadcast_npos)
                                         ? 0xFFFFFFFFu
                                         : static_cast<std::uint32_t>(bs);
            }
            for (std::size_t i = 0; i < a.shape().rank(); ++i) {
                params.a_extents[i] =
                    static_cast<std::uint32_t>(a.shape()[i].extent);
            }
            for (std::size_t i = 0; i < b.shape().rank(); ++i) {
                params.b_extents[i] =
                    static_cast<std::uint32_t>(b.shape()[i].extent);
            }

            const std::size_t a_total = a.size();
            const std::size_t b_total = b.size();
            // NOTE: DynamicShape::size() returns the rank (number of
            // axes), not the element count. Use total() for the
            // element-count product.
            const std::size_t result_total = plan.result.total();
            const std::size_t out_bytes = result_total * sizeof(float);

            auto& ctx = detail::WebGPUContext::current();
            auto gA = detail::make_input_buffer(ctx.device(), a_total * sizeof(float));
            auto gB = detail::make_input_buffer(ctx.device(), b_total * sizeof(float));
            auto gOut = detail::make_output_buffer(ctx.device(), out_bytes);
            ctx.queue().WriteBuffer(gA, 0, a.data(), a_total * sizeof(float));
            ctx.queue().WriteBuffer(gB, 0, b.data(), b_total * sizeof(float));

            detail::dispatch_broadcast(ctx, wgsl_template, op_token,
                                       gA, a_total * sizeof(float),
                                       gB, b_total * sizeof(float),
                                       gOut, out_bytes,
                                       params, result_total,
                                       wgsl::kDefaultWorkgroupSize);

            DynamicTensor<T> out{plan.result};
            detail::copy_buffer_to_host(ctx, gOut, out.data(), out_bytes);
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
