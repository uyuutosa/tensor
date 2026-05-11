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
