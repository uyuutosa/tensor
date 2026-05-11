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

TEST_CASE("webgpu::wgsl: kernel sources are non-empty") {
    CHECK(!wgsl::kAddF32.empty());
    CHECK(!wgsl::kSubF32.empty());
    CHECK(!wgsl::kMulF32.empty());
    CHECK(!wgsl::kDivF32.empty());
}

TEST_CASE("webgpu::wgsl: all kernels declare the three storage bindings") {
    for (auto src : {wgsl::kAddF32, wgsl::kSubF32, wgsl::kMulF32, wgsl::kDivF32}) {
        CHECK(contains(src, "@group(0) @binding(0)"));
        CHECK(contains(src, "@group(0) @binding(1)"));
        CHECK(contains(src, "@group(0) @binding(2)"));
        CHECK(contains(src, "var<storage, read>"));
        CHECK(contains(src, "var<storage, read_write>"));
    }
}

TEST_CASE("webgpu::wgsl: all kernels are templated on workgroupSize + precision") {
    for (auto src : {wgsl::kAddF32, wgsl::kSubF32, wgsl::kMulF32, wgsl::kDivF32}) {
        CHECK(contains(src, "{{workgroupSize}}"));
        CHECK(contains(src, "{{precision}}"));
    }
}

TEST_CASE("webgpu::wgsl: all kernels declare a 1-D compute entry point") {
    for (auto src : {wgsl::kAddF32, wgsl::kSubF32, wgsl::kMulF32, wgsl::kDivF32}) {
        CHECK(contains(src, "@compute @workgroup_size({{workgroupSize}})"));
        CHECK(contains(src, "fn main(@builtin(global_invocation_id) gid: vec3<u32>)"));
        CHECK(contains(src, "arrayLength(&out)"));
    }
}

TEST_CASE("webgpu::wgsl: each kernel encodes its own operator") {
    CHECK(contains(wgsl::kAddF32, "out[i] = a[i] + b[i]"));
    CHECK(contains(wgsl::kSubF32, "out[i] = a[i] - b[i]"));
    CHECK(contains(wgsl::kMulF32, "out[i] = a[i] * b[i]"));
    CHECK(contains(wgsl::kDivF32, "out[i] = a[i] / b[i]"));
}

TEST_CASE("webgpu::wgsl: default workgroup size matches gpu.cpp canonical 256") {
    CHECK(wgsl::kDefaultWorkgroupSize == 256u);
}
