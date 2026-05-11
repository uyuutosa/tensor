// SPDX-License-Identifier: MIT
//
// tensor::core::backend::webgpu::detail — element-wise kernel dispatch
// helpers. Per ADR-0016 the helpers use Dawn's RAII C++ wrapper
// directly (no gpu.cpp).
//
// `dispatch_element_wise<N>(wgsl, bufs, n_elements, workgroup_size)`
// compiles the WGSL source, sets up the bind group layout, executes
// the compute pass on the device, and waits for queue completion.
// `N` is the number of storage bindings (3 for binary, 2 for unary).
//
// For the MVP we compile per call. The next perf-investigation slice
// (ADR-0014 / discussion-points Axis C) may add a pipeline cache keyed
// on (WGSL source, workgroup size).

#pragma once

#if !defined(TENSOR_HAS_WEBGPU)
#    error "tensor/core/backend/webgpu_detail/dispatch.hpp included without TENSOR_HAS_WEBGPU"
#endif

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <string_view>

#include <webgpu/webgpu_cpp.h>

#include "tensor/core/backend/webgpu_detail/context.hpp"

namespace tensor::core::backend::webgpu::detail {

// Substitute {{precision}} and {{workgroupSize}} placeholders in a
// project WGSL kernel template. gpu.cpp used to do this internally;
// per ADR-0016 we do it ourselves.
inline std::string substitute_wgsl(std::string_view tmpl,
                                   std::string_view precision,
                                   std::string_view workgroup_size) {
    std::string out{tmpl};
    auto replace_all = [&](std::string_view from, std::string_view to) {
        std::string::size_type pos = 0;
        while ((pos = out.find(from.data(), pos, from.size())) !=
               std::string::npos) {
            out.replace(pos, from.size(), to.data(), to.size());
            pos += to.size();
        }
    };
    replace_all("{{precision}}", precision);
    replace_all("{{workgroupSize}}", workgroup_size);
    return out;
}

inline wgpu::StringView make_string_view(std::string_view sv) {
    return wgpu::StringView{sv.data(), sv.size()};
}

// Allocate a storage buffer sized `bytes` with Storage + CopyDst usage.
inline wgpu::Buffer make_input_buffer(wgpu::Device const& device, std::size_t bytes) {
    wgpu::BufferDescriptor desc{};
    desc.size = bytes;
    desc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
    return const_cast<wgpu::Device&>(device).CreateBuffer(&desc);
}

// Allocate a storage buffer sized `bytes` with Storage + CopySrc usage
// (kernel output that we copy out via a staging buffer).
inline wgpu::Buffer make_output_buffer(wgpu::Device const& device, std::size_t bytes) {
    wgpu::BufferDescriptor desc{};
    desc.size = bytes;
    desc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
    return const_cast<wgpu::Device&>(device).CreateBuffer(&desc);
}

// Allocate a CPU-mappable staging buffer for reading the output back.
inline wgpu::Buffer make_staging_buffer(wgpu::Device const& device, std::size_t bytes) {
    wgpu::BufferDescriptor desc{};
    desc.size = bytes;
    desc.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;
    return const_cast<wgpu::Device&>(device).CreateBuffer(&desc);
}

// Dispatch an element-wise kernel with `N` storage bindings. The first
// N-1 bindings are read-only inputs; the last is the read-write output.
// `bufs[0..N-2]` are inputs (`a`, `b`, …); `bufs[N-1]` is the output.
template <std::size_t N>
void dispatch_element_wise(WebGPUContext& ctx,
                           std::string_view wgsl_template,
                           std::array<wgpu::Buffer, N> const& bufs,
                           std::size_t n_elements,
                           std::size_t workgroup_size) {
    static_assert(N >= 2,
                  "dispatch_element_wise needs at least one input + one output");

    auto& device = ctx.device();
    auto& queue = ctx.queue();
    auto& instance = ctx.instance();
    const std::size_t buf_bytes = n_elements * sizeof(float);

    // Substitute WGSL placeholders.
    std::string wgsl = substitute_wgsl(
        wgsl_template, std::string_view{"f32"},
        std::to_string(workgroup_size));

    // ShaderModule.
    wgpu::ShaderSourceWGSL wgslSrc{};
    wgslSrc.code = make_string_view(wgsl);
    wgpu::ShaderModuleDescriptor smDesc{};
    smDesc.nextInChain = &wgslSrc;
    auto shader = device.CreateShaderModule(&smDesc);

    // BindGroupLayout — bindings 0..N-2 = ReadOnlyStorage, N-1 = Storage.
    std::array<wgpu::BindGroupLayoutEntry, N> bgle{};
    for (std::size_t i = 0; i < N; ++i) {
        bgle[i].binding = static_cast<std::uint32_t>(i);
        bgle[i].visibility = wgpu::ShaderStage::Compute;
        bgle[i].buffer.type = (i + 1 == N)
                                  ? wgpu::BufferBindingType::Storage
                                  : wgpu::BufferBindingType::ReadOnlyStorage;
    }
    wgpu::BindGroupLayoutDescriptor bglDesc{};
    bglDesc.entryCount = N;
    bglDesc.entries = bgle.data();
    auto bgl = device.CreateBindGroupLayout(&bglDesc);

    // PipelineLayout.
    wgpu::BindGroupLayout bgls[1] = {bgl};
    wgpu::PipelineLayoutDescriptor plDesc{};
    plDesc.bindGroupLayoutCount = 1;
    plDesc.bindGroupLayouts = bgls;
    auto pl = device.CreatePipelineLayout(&plDesc);

    // ComputePipeline.
    wgpu::ComputePipelineDescriptor cpDesc{};
    cpDesc.layout = pl;
    cpDesc.compute.module = shader;
    cpDesc.compute.entryPoint = make_string_view("main");
    auto pipeline = device.CreateComputePipeline(&cpDesc);

    // BindGroup.
    std::array<wgpu::BindGroupEntry, N> bge{};
    for (std::size_t i = 0; i < N; ++i) {
        bge[i].binding = static_cast<std::uint32_t>(i);
        bge[i].buffer = bufs[i];
        bge[i].offset = 0;
        bge[i].size = buf_bytes;
    }
    wgpu::BindGroupDescriptor bgDesc{};
    bgDesc.layout = bgl;
    bgDesc.entryCount = N;
    bgDesc.entries = bge.data();
    auto bg = device.CreateBindGroup(&bgDesc);

    // Encode + dispatch.
    auto encoder = device.CreateCommandEncoder();
    {
        auto pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bg);
        const std::uint32_t total_groups = static_cast<std::uint32_t>(
            (n_elements + workgroup_size - 1) / workgroup_size);
        pass.DispatchWorkgroups(total_groups, 1, 1);
        pass.End();
    }
    auto cmd = encoder.Finish();
    queue.Submit(1, &cmd);

    // Wait for queue.
    bool done = false;
    queue.OnSubmittedWorkDone(
        wgpu::CallbackMode::AllowProcessEvents,
        [&done](wgpu::QueueWorkDoneStatus, wgpu::StringView) { done = true; });
    pump_until(instance, done);
}

// Uniform-buffer Params for the broadcast kernels (mirrors
// BroadcastParams declared inside webgpu_wgsl.hpp::kBroadcastBodyF32).
// The std140-like layout matches WGSL uniform-block rules: scalar u32
// is 4 bytes; array<u32, 8> is 32 bytes contiguous because each element
// is u32-sized — WGSL does NOT round each element up to 16 bytes (that
// rule applies to array<T> where T is < 16 bytes only for certain
// element types; u32 arrays pack tightly).
struct BroadcastParams {
    std::uint32_t result_rank;
    std::uint32_t a_rank;
    std::uint32_t b_rank;
    std::uint32_t padding;
    std::uint32_t result_extents[8];
    std::uint32_t a_extents[8];
    std::uint32_t b_extents[8];
    std::uint32_t a_source[8];
    std::uint32_t b_source[8];
};

// Dispatch a broadcast element-wise kernel (`kBroadcastAddF32` etc.).
// `op_token` is the WGSL infix operator string (`"+"`, `"-"`, `"*"`)
// that substitutes the `{{op}}` placeholder in the kernel template.
// Caller has already uploaded `gA` (size = a_total_elements * sizeof(float))
// and `gB`, and provided a `gOut` sized for `result_total * sizeof(float)`.
inline void dispatch_broadcast(WebGPUContext& ctx,
                               std::string_view wgsl_template,
                               std::string_view op_token,
                               wgpu::Buffer const& gA, std::size_t a_bytes,
                               wgpu::Buffer const& gB, std::size_t b_bytes,
                               wgpu::Buffer const& gOut, std::size_t out_bytes,
                               BroadcastParams const& params,
                               std::size_t result_total,
                               std::size_t workgroup_size) {
    auto& device = ctx.device();
    auto& queue = ctx.queue();
    auto& instance = ctx.instance();

    // Substitute placeholders. `{{op}}` is broadcast-kernel-specific;
    // {{precision}} and {{workgroupSize}} are the standard ones from
    // dispatch_element_wise.
    std::string wgsl{wgsl_template};
    {
        auto replace_all = [&](std::string_view from, std::string_view to) {
            std::string::size_type pos = 0;
            while ((pos = wgsl.find(from.data(), pos, from.size())) !=
                   std::string::npos) {
                wgsl.replace(pos, from.size(), to.data(), to.size());
                pos += to.size();
            }
        };
        replace_all("{{precision}}", std::string_view{"f32"});
        replace_all("{{workgroupSize}}", std::to_string(workgroup_size));
        replace_all("{{op}}", op_token);
    }

    wgpu::ShaderSourceWGSL wgslSrc{};
    wgslSrc.code = make_string_view(wgsl);
    wgpu::ShaderModuleDescriptor smDesc{};
    smDesc.nextInChain = &wgslSrc;
    auto shader = device.CreateShaderModule(&smDesc);

    // Params buffer — Storage (read-only) so the u32 arrays pack tightly
    // per WGSL's std430 rules. A Uniform buffer would force the arrays
    // to std140-like 16-byte-per-element stride which our C++ struct
    // does not honor.
    wgpu::BufferDescriptor pDesc{};
    pDesc.size = sizeof(BroadcastParams);
    pDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
    auto gParams = device.CreateBuffer(&pDesc);
    queue.WriteBuffer(gParams, 0, &params, sizeof(BroadcastParams));

    // 4-binding layout: all storage (3 input/output + 1 params).
    std::array<wgpu::BindGroupLayoutEntry, 4> bgle{};
    bgle[0].binding = 0;
    bgle[0].visibility = wgpu::ShaderStage::Compute;
    bgle[0].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
    bgle[1].binding = 1;
    bgle[1].visibility = wgpu::ShaderStage::Compute;
    bgle[1].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
    bgle[2].binding = 2;
    bgle[2].visibility = wgpu::ShaderStage::Compute;
    bgle[2].buffer.type = wgpu::BufferBindingType::Storage;
    bgle[3].binding = 3;
    bgle[3].visibility = wgpu::ShaderStage::Compute;
    bgle[3].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
    wgpu::BindGroupLayoutDescriptor bglDesc{};
    bglDesc.entryCount = 4;
    bglDesc.entries = bgle.data();
    auto bgl = device.CreateBindGroupLayout(&bglDesc);

    wgpu::BindGroupLayout bgls[1] = {bgl};
    wgpu::PipelineLayoutDescriptor plDesc{};
    plDesc.bindGroupLayoutCount = 1;
    plDesc.bindGroupLayouts = bgls;
    auto pl = device.CreatePipelineLayout(&plDesc);

    wgpu::ComputePipelineDescriptor cpDesc{};
    cpDesc.layout = pl;
    cpDesc.compute.module = shader;
    cpDesc.compute.entryPoint = make_string_view("main");
    auto pipeline = device.CreateComputePipeline(&cpDesc);

    std::array<wgpu::BindGroupEntry, 4> bge{};
    bge[0].binding = 0; bge[0].buffer = gA;      bge[0].offset = 0; bge[0].size = a_bytes;
    bge[1].binding = 1; bge[1].buffer = gB;      bge[1].offset = 0; bge[1].size = b_bytes;
    bge[2].binding = 2; bge[2].buffer = gOut;    bge[2].offset = 0; bge[2].size = out_bytes;
    bge[3].binding = 3; bge[3].buffer = gParams; bge[3].offset = 0; bge[3].size = sizeof(BroadcastParams);
    wgpu::BindGroupDescriptor bgDesc{};
    bgDesc.layout = bgl;
    bgDesc.entryCount = 4;
    bgDesc.entries = bge.data();
    auto bg = device.CreateBindGroup(&bgDesc);

    auto encoder = device.CreateCommandEncoder();
    {
        auto pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bg);
        const std::uint32_t groups = static_cast<std::uint32_t>(
            (result_total + workgroup_size - 1) / workgroup_size);
        pass.DispatchWorkgroups(groups, 1, 1);
        pass.End();
    }
    auto cmd = encoder.Finish();
    queue.Submit(1, &cmd);

    bool done = false;
    queue.OnSubmittedWorkDone(
        wgpu::CallbackMode::AllowProcessEvents,
        [&done](wgpu::QueueWorkDoneStatus, wgpu::StringView) { done = true; });
    pump_until(instance, done);
}

// Uniform-buffer Params for the tiled GEMM kernel (mirrors the
// `struct Params { M, N, K }` declared inside webgpu_wgsl.hpp::kGemmF32).
// 16-byte alignment is implicit because the struct is 12 bytes and
// WGSL uniform buffers are always padded to 16-byte minimum.
struct GemmParams {
    std::uint32_t M;
    std::uint32_t N;
    std::uint32_t K;
};

// Dispatch the tiled GEMM kernel (`kGemmF32`) for inputs gA (M×K),
// gB (K×N), output gOut (M×N). 2-D dispatch with workgroup size
// (TILE_N, TILE_M, 1) = (16, 16, 1) and totalWorkgroups
// (ceil(N/16), ceil(M/16), 1).
//
// Bindings:
//   0 = gA       (storage, read-only)
//   1 = gB       (storage, read-only)
//   2 = gOut     (storage, read-write)
//   3 = uniform  (Params{M, N, K})
inline void dispatch_gemm(WebGPUContext& ctx,
                          std::string_view wgsl_template,
                          wgpu::Buffer const& gA,
                          wgpu::Buffer const& gB,
                          wgpu::Buffer const& gOut,
                          std::size_t M, std::size_t N, std::size_t K,
                          std::size_t tile_m, std::size_t tile_n) {
    auto& device = ctx.device();
    auto& queue = ctx.queue();
    auto& instance = ctx.instance();

    // Substitute WGSL placeholders. kGemmF32 already inlines the tile
    // constants as `const TILE_*: u32 = 16u;`, so only precision is
    // template-substituted here. workgroupSize is implicit in the
    // kernel's `@compute @workgroup_size(TILE_N, TILE_M, 1)`.
    std::string wgsl = substitute_wgsl(
        wgsl_template, std::string_view{"f32"}, std::string_view{"256"});

    // ShaderModule.
    wgpu::ShaderSourceWGSL wgslSrc{};
    wgslSrc.code = make_string_view(wgsl);
    wgpu::ShaderModuleDescriptor smDesc{};
    smDesc.nextInChain = &wgslSrc;
    auto shader = device.CreateShaderModule(&smDesc);

    // Uniform buffer with Params.
    GemmParams params{static_cast<std::uint32_t>(M),
                      static_cast<std::uint32_t>(N),
                      static_cast<std::uint32_t>(K)};
    wgpu::BufferDescriptor pDesc{};
    pDesc.size = sizeof(GemmParams);
    pDesc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
    auto gParams = device.CreateBuffer(&pDesc);
    queue.WriteBuffer(gParams, 0, &params, sizeof(GemmParams));

    // BindGroupLayout — 4 bindings (3 storage + 1 uniform).
    std::array<wgpu::BindGroupLayoutEntry, 4> bgle{};
    bgle[0].binding = 0;
    bgle[0].visibility = wgpu::ShaderStage::Compute;
    bgle[0].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
    bgle[1].binding = 1;
    bgle[1].visibility = wgpu::ShaderStage::Compute;
    bgle[1].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
    bgle[2].binding = 2;
    bgle[2].visibility = wgpu::ShaderStage::Compute;
    bgle[2].buffer.type = wgpu::BufferBindingType::Storage;
    bgle[3].binding = 3;
    bgle[3].visibility = wgpu::ShaderStage::Compute;
    bgle[3].buffer.type = wgpu::BufferBindingType::Uniform;
    wgpu::BindGroupLayoutDescriptor bglDesc{};
    bglDesc.entryCount = 4;
    bglDesc.entries = bgle.data();
    auto bgl = device.CreateBindGroupLayout(&bglDesc);

    // PipelineLayout.
    wgpu::BindGroupLayout bgls[1] = {bgl};
    wgpu::PipelineLayoutDescriptor plDesc{};
    plDesc.bindGroupLayoutCount = 1;
    plDesc.bindGroupLayouts = bgls;
    auto pl = device.CreatePipelineLayout(&plDesc);

    // ComputePipeline.
    wgpu::ComputePipelineDescriptor cpDesc{};
    cpDesc.layout = pl;
    cpDesc.compute.module = shader;
    cpDesc.compute.entryPoint = make_string_view("main");
    auto pipeline = device.CreateComputePipeline(&cpDesc);

    // BindGroup.
    std::array<wgpu::BindGroupEntry, 4> bge{};
    bge[0].binding = 0; bge[0].buffer = gA;      bge[0].offset = 0; bge[0].size = M * K * sizeof(float);
    bge[1].binding = 1; bge[1].buffer = gB;      bge[1].offset = 0; bge[1].size = K * N * sizeof(float);
    bge[2].binding = 2; bge[2].buffer = gOut;    bge[2].offset = 0; bge[2].size = M * N * sizeof(float);
    bge[3].binding = 3; bge[3].buffer = gParams; bge[3].offset = 0; bge[3].size = sizeof(GemmParams);
    wgpu::BindGroupDescriptor bgDesc{};
    bgDesc.layout = bgl;
    bgDesc.entryCount = 4;
    bgDesc.entries = bge.data();
    auto bg = device.CreateBindGroup(&bgDesc);

    // Encode + dispatch (2-D).
    auto encoder = device.CreateCommandEncoder();
    {
        auto pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bg);
        const std::uint32_t groups_x = static_cast<std::uint32_t>((N + tile_n - 1) / tile_n);
        const std::uint32_t groups_y = static_cast<std::uint32_t>((M + tile_m - 1) / tile_m);
        pass.DispatchWorkgroups(groups_x, groups_y, 1);
        pass.End();
    }
    auto cmd = encoder.Finish();
    queue.Submit(1, &cmd);

    bool done = false;
    queue.OnSubmittedWorkDone(
        wgpu::CallbackMode::AllowProcessEvents,
        [&done](wgpu::QueueWorkDoneStatus, wgpu::StringView) { done = true; });
    pump_until(instance, done);
}

// Copy a GPU buffer to a host-mappable staging buffer, then map it and
// copy the bytes into `out`. Caller owns the host buffer.
inline void copy_buffer_to_host(WebGPUContext& ctx, wgpu::Buffer const& src,
                                void* out, std::size_t bytes) {
    auto& device = ctx.device();
    auto& queue = ctx.queue();
    auto& instance = ctx.instance();

    auto staging = make_staging_buffer(device, bytes);
    auto encoder = device.CreateCommandEncoder();
    encoder.CopyBufferToBuffer(src, 0, staging, 0, bytes);
    auto cmd = encoder.Finish();
    queue.Submit(1, &cmd);

    // Wait for submission, then map.
    bool wd_done = false;
    queue.OnSubmittedWorkDone(
        wgpu::CallbackMode::AllowProcessEvents,
        [&wd_done](wgpu::QueueWorkDoneStatus, wgpu::StringView) { wd_done = true; });
    pump_until(instance, wd_done);

    bool map_done = false;
    std::string map_msg;
    bool map_ok = false;
    staging.MapAsync(wgpu::MapMode::Read, 0, bytes,
                     wgpu::CallbackMode::AllowProcessEvents,
                     [&](wgpu::MapAsyncStatus status, wgpu::StringView msg) {
                         map_ok = (status == wgpu::MapAsyncStatus::Success);
                         if (!map_ok && msg.data && msg.length > 0) {
                             map_msg.assign(msg.data, msg.length);
                         }
                         map_done = true;
                     });
    pump_until(instance, map_done);
    if (!map_ok) {
        throw std::runtime_error{"WebGPU staging-buffer MapAsync failed: " + map_msg};
    }

    void const* mapped = staging.GetConstMappedRange(0, bytes);
    if (!mapped) {
        throw std::runtime_error{"wgpu::Buffer::GetConstMappedRange returned null"};
    }
    std::memcpy(out, mapped, bytes);
    staging.Unmap();
}

}  // namespace tensor::core::backend::webgpu::detail
