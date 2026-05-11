// SPDX-License-Identifier: MIT
//
// tensor::core::backend::webgpu::wgsl — WGSL kernel source strings.
//
// Phase 3 P3.M3.1 (per the addendum in docs/impl-plans/2026-05-11_phase-3-webgpu.md).
// This header holds the WGSL source for the WebGPU adapter's compute kernels
// as `constexpr std::string_view` constants. The kernels are written in the
// `{{workgroupSize}}` / `{{precision}}` templated form expected by
// gpu::KernelCode (see third_party/gpu_cpp/gpu.hpp:291-389) — at runtime
// gpu.cpp substitutes the placeholders before passing the source to Dawn /
// WGSL compilation.
//
// The dispatch wiring that actually feeds these strings to gpu.cpp lives in
// docs/detailed-design/webgpu-element-wise-kernels.md (the design) and lands
// in P3.M3.2 (the dedicated PR with self-hosted GPU runner). Until then the
// WebGPU Backend's add/sub/mul/div methods still delegate to reference per
// ADR-0012; the kernels here are inert citable source code.
//
// Per ADR-0013 (canonical-reference framing), these kernel sources are
// themselves a citable artifact — they are how the named-axis algebra
// projects onto WebGPU's WGSL surface, with one kernel per element-wise
// binary operator.

#pragma once

#include <string_view>

namespace tensor::core::backend::webgpu::wgsl {

// Element-wise binary kernels — one shader per operator.
//
// All four follow the same shape:
//   - inputs: read-only storage buffers `a`, `b`, sized N (the tensor's
//     flat extent).
//   - output: read-write storage buffer `out`, also sized N.
//   - workgroup is 1-D (`{{workgroupSize}}` threads on the x-axis);
//     totalWorkgroups = ceil(N / workgroupSize).
//   - `{{precision}}` is f32 in the Phase 3 MVP per ADR-0012; f16 is a
//     follow-up that gpu.cpp's `enable f16;` prelude already supports.
//
// The bodies are intentionally one-liners. Phase 4+ may add fused
// kernels (e.g. `a * b + c`); they would live alongside these.

inline constexpr std::string_view kAddF32 = R"WGSL(
@group(0) @binding(0) var<storage, read>       a   : array<{{precision}}>;
@group(0) @binding(1) var<storage, read>       b   : array<{{precision}}>;
@group(0) @binding(2) var<storage, read_write> out : array<{{precision}}>;

@compute @workgroup_size({{workgroupSize}})
fn main(@builtin(global_invocation_id) gid: vec3<u32>) {
    let i = gid.x;
    if (i >= arrayLength(&out)) { return; }
    out[i] = a[i] + b[i];
}
)WGSL";

inline constexpr std::string_view kSubF32 = R"WGSL(
@group(0) @binding(0) var<storage, read>       a   : array<{{precision}}>;
@group(0) @binding(1) var<storage, read>       b   : array<{{precision}}>;
@group(0) @binding(2) var<storage, read_write> out : array<{{precision}}>;

@compute @workgroup_size({{workgroupSize}})
fn main(@builtin(global_invocation_id) gid: vec3<u32>) {
    let i = gid.x;
    if (i >= arrayLength(&out)) { return; }
    out[i] = a[i] - b[i];
}
)WGSL";

inline constexpr std::string_view kMulF32 = R"WGSL(
@group(0) @binding(0) var<storage, read>       a   : array<{{precision}}>;
@group(0) @binding(1) var<storage, read>       b   : array<{{precision}}>;
@group(0) @binding(2) var<storage, read_write> out : array<{{precision}}>;

@compute @workgroup_size({{workgroupSize}})
fn main(@builtin(global_invocation_id) gid: vec3<u32>) {
    let i = gid.x;
    if (i >= arrayLength(&out)) { return; }
    out[i] = a[i] * b[i];
}
)WGSL";

inline constexpr std::string_view kDivF32 = R"WGSL(
@group(0) @binding(0) var<storage, read>       a   : array<{{precision}}>;
@group(0) @binding(1) var<storage, read>       b   : array<{{precision}}>;
@group(0) @binding(2) var<storage, read_write> out : array<{{precision}}>;

@compute @workgroup_size({{workgroupSize}})
fn main(@builtin(global_invocation_id) gid: vec3<u32>) {
    let i = gid.x;
    if (i >= arrayLength(&out)) { return; }
    out[i] = a[i] / b[i];
}
)WGSL";

// Element-wise unary kernels — one shader per activation / sign operation.
//
// Same shape as the binary kernels above, but with two storage bindings
// instead of three:
//   - input:  read-only storage buffer `a`, sized N.
//   - output: read-write storage buffer `out`, also sized N.
//
// Mapped to `KernelBackend`'s unary methods: `exp`, `log`, `relu`, `neg`.
// These are the activations Phase 2 autograd already supports on the
// reference + Eigen adapters; P3.M3.2 swaps them to GPU dispatch.

inline constexpr std::string_view kExpF32 = R"WGSL(
@group(0) @binding(0) var<storage, read>       a   : array<{{precision}}>;
@group(0) @binding(1) var<storage, read_write> out : array<{{precision}}>;

@compute @workgroup_size({{workgroupSize}})
fn main(@builtin(global_invocation_id) gid: vec3<u32>) {
    let i = gid.x;
    if (i >= arrayLength(&out)) { return; }
    out[i] = exp(a[i]);
}
)WGSL";

inline constexpr std::string_view kLogF32 = R"WGSL(
@group(0) @binding(0) var<storage, read>       a   : array<{{precision}}>;
@group(0) @binding(1) var<storage, read_write> out : array<{{precision}}>;

@compute @workgroup_size({{workgroupSize}})
fn main(@builtin(global_invocation_id) gid: vec3<u32>) {
    let i = gid.x;
    if (i >= arrayLength(&out)) { return; }
    out[i] = log(a[i]);
}
)WGSL";

// ReLU expressed via `max(a, 0)` rather than a branch; the WGSL `max`
// built-in maps to a single GPU instruction on every Dawn-supported
// backend (Vulkan, Metal, D3D12). The literal `0.0` is implicitly the
// active precision because the operand type drives it.
inline constexpr std::string_view kReluF32 = R"WGSL(
@group(0) @binding(0) var<storage, read>       a   : array<{{precision}}>;
@group(0) @binding(1) var<storage, read_write> out : array<{{precision}}>;

@compute @workgroup_size({{workgroupSize}})
fn main(@builtin(global_invocation_id) gid: vec3<u32>) {
    let i = gid.x;
    if (i >= arrayLength(&out)) { return; }
    out[i] = max(a[i], 0.0);
}
)WGSL";

inline constexpr std::string_view kNegF32 = R"WGSL(
@group(0) @binding(0) var<storage, read>       a   : array<{{precision}}>;
@group(0) @binding(1) var<storage, read_write> out : array<{{precision}}>;

@compute @workgroup_size({{workgroupSize}})
fn main(@builtin(global_invocation_id) gid: vec3<u32>) {
    let i = gid.x;
    if (i >= arrayLength(&out)) { return; }
    out[i] = -a[i];
}
)WGSL";

// Default workgroup size for the element-wise kernels above. 256 is the
// canonical Dawn / WebGPU choice (matches gpu.cpp's default in the
// `KernelCode(string, size_t)` overload at third_party/gpu_cpp/gpu.hpp:308).
inline constexpr std::size_t kDefaultWorkgroupSize = 256;

}  // namespace tensor::core::backend::webgpu::wgsl
