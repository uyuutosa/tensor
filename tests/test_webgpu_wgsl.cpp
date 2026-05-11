// SPDX-License-Identifier: MIT
//
// Phase 3 P3.M3.1 — verifies the four WGSL element-wise binary kernel
// sources are present, well-formed at the text level (no shader
// compilation), and substitution-ready for gpu.cpp's KernelCode
// constructor. The actual GPU dispatch lives in P3.M3.2 (deferred to a
// dedicated PR with a self-hosted GPU runner).
//
// These tests do not require Dawn to be linked; the WGSL constants are
// `constexpr std::string_view` and the test runs on the always-on
// reference CI matrix.

#include <doctest/doctest.h>

#include <array>
#include <string_view>

// The header is in the webgpu adapter directory but does not depend on
// TENSOR_HAS_WEBGPU (which would require Dawn). The kernel sources are
// pure text and should be available to test even when the WebGPU
// backend is not the selected backend.
#include <tensor/core/backend/webgpu_wgsl.hpp>

namespace wgsl = tensor::core::backend::webgpu::wgsl;

namespace {

bool contains(std::string_view haystack, std::string_view needle) {
    return haystack.find(needle) != std::string_view::npos;
}

}  // namespace

namespace {
constexpr std::array kBinaryKernels = {
    wgsl::kAddF32, wgsl::kSubF32, wgsl::kMulF32, wgsl::kDivF32,
};
constexpr std::array kUnaryKernels = {
    wgsl::kExpF32, wgsl::kLogF32, wgsl::kReluF32, wgsl::kNegF32,
};
}  // namespace

TEST_CASE("webgpu::wgsl: binary kernel sources are non-empty") {
    for (auto src : kBinaryKernels) {
        CHECK(!src.empty());
    }
}

TEST_CASE("webgpu::wgsl: unary kernel sources are non-empty") {
    for (auto src : kUnaryKernels) {
        CHECK(!src.empty());
    }
}

TEST_CASE("webgpu::wgsl: binary kernels declare three storage bindings") {
    for (auto src : kBinaryKernels) {
        CHECK(contains(src, "@group(0) @binding(0)"));
        CHECK(contains(src, "@group(0) @binding(1)"));
        CHECK(contains(src, "@group(0) @binding(2)"));
        CHECK(contains(src, "var<storage, read>"));
        CHECK(contains(src, "var<storage, read_write>"));
    }
}

TEST_CASE("webgpu::wgsl: unary kernels declare exactly two storage bindings") {
    for (auto src : kUnaryKernels) {
        CHECK(contains(src, "@group(0) @binding(0)"));
        CHECK(contains(src, "@group(0) @binding(1)"));
        // Unary kernels must not declare a third binding — the binding-2
        // line that appears in binary kernels has no counterpart here.
        CHECK_FALSE(contains(src, "@group(0) @binding(2)"));
        CHECK(contains(src, "var<storage, read>"));
        CHECK(contains(src, "var<storage, read_write>"));
    }
}

TEST_CASE("webgpu::wgsl: all kernels are templated on workgroupSize + precision") {
    for (auto src : kBinaryKernels) {
        CHECK(contains(src, "{{workgroupSize}}"));
        CHECK(contains(src, "{{precision}}"));
    }
    for (auto src : kUnaryKernels) {
        CHECK(contains(src, "{{workgroupSize}}"));
        CHECK(contains(src, "{{precision}}"));
    }
}

TEST_CASE("webgpu::wgsl: all kernels declare a 1-D compute entry point") {
    for (auto src : kBinaryKernels) {
        CHECK(contains(src, "@compute @workgroup_size({{workgroupSize}})"));
        CHECK(contains(src, "fn main(@builtin(global_invocation_id) gid: vec3<u32>)"));
        CHECK(contains(src, "arrayLength(&out)"));
    }
    for (auto src : kUnaryKernels) {
        CHECK(contains(src, "@compute @workgroup_size({{workgroupSize}})"));
        CHECK(contains(src, "fn main(@builtin(global_invocation_id) gid: vec3<u32>)"));
        CHECK(contains(src, "arrayLength(&out)"));
    }
}

TEST_CASE("webgpu::wgsl: each binary kernel encodes its own operator") {
    CHECK(contains(wgsl::kAddF32, "out[i] = a[i] + b[i]"));
    CHECK(contains(wgsl::kSubF32, "out[i] = a[i] - b[i]"));
    CHECK(contains(wgsl::kMulF32, "out[i] = a[i] * b[i]"));
    CHECK(contains(wgsl::kDivF32, "out[i] = a[i] / b[i]"));
}

TEST_CASE("webgpu::wgsl: each unary kernel encodes its own activation") {
    CHECK(contains(wgsl::kExpF32, "out[i] = exp(a[i])"));
    CHECK(contains(wgsl::kLogF32, "out[i] = log(a[i])"));
    CHECK(contains(wgsl::kReluF32, "out[i] = max(a[i], 0.0)"));
    CHECK(contains(wgsl::kNegF32, "out[i] = -a[i]"));
}

TEST_CASE("webgpu::wgsl: default workgroup size matches gpu.cpp canonical 256") {
    CHECK(wgsl::kDefaultWorkgroupSize == 256u);
}

// ─── GEMM kernel (P3.M4) ──────────────────────────────────────────────

TEST_CASE("webgpu::wgsl: GEMM tile constants align with the kernel source") {
    CHECK(wgsl::kGemmTileM == 16u);
    CHECK(wgsl::kGemmTileN == 16u);
    CHECK(wgsl::kGemmTileK == 16u);
    // The kernel source hard-codes the same tile sizes; if either side
    // changes, both must.
    CHECK(contains(wgsl::kGemmF32, "TILE_M : u32 = 16u"));
    CHECK(contains(wgsl::kGemmF32, "TILE_N : u32 = 16u"));
    CHECK(contains(wgsl::kGemmF32, "TILE_K : u32 = 16u"));
}

TEST_CASE("webgpu::wgsl: GEMM kernel declares four bindings (a, b, out, params)") {
    CHECK(contains(wgsl::kGemmF32, "@group(0) @binding(0)"));
    CHECK(contains(wgsl::kGemmF32, "@group(0) @binding(1)"));
    CHECK(contains(wgsl::kGemmF32, "@group(0) @binding(2)"));
    CHECK(contains(wgsl::kGemmF32, "@group(0) @binding(3)"));
    CHECK(contains(wgsl::kGemmF32, "struct Params"));
    CHECK(contains(wgsl::kGemmF32, "var<uniform>"));
}

TEST_CASE("webgpu::wgsl: GEMM kernel is templated on precision and uses 2D workgroup") {
    CHECK(contains(wgsl::kGemmF32, "{{precision}}"));
    CHECK(contains(wgsl::kGemmF32, "@compute @workgroup_size(TILE_N, TILE_M, 1)"));
}

TEST_CASE("webgpu::wgsl: GEMM kernel uses workgroup shared memory + barriers") {
    CHECK(contains(wgsl::kGemmF32, "var<workgroup> shA"));
    CHECK(contains(wgsl::kGemmF32, "var<workgroup> shB"));
    // Both barriers are needed: one after loading the tile, one after
    // consuming it (so the next iteration's load doesn't race with the
    // current iteration's accumulation).
    auto first = wgsl::kGemmF32.find("workgroupBarrier()");
    REQUIRE(first != std::string_view::npos);
    auto second = wgsl::kGemmF32.find("workgroupBarrier()", first + 1);
    CHECK(second != std::string_view::npos);
}

TEST_CASE("webgpu::wgsl: GEMM kernel performs the inner product and respects bounds") {
    CHECK(contains(wgsl::kGemmF32, "acc = acc + shA[lrow][k] * shB[k][lcol]"));
    CHECK(contains(wgsl::kGemmF32, "if (row < p.M && col < p.N)"));
    CHECK(contains(wgsl::kGemmF32, "out[row * p.N + col] = acc"));
}
