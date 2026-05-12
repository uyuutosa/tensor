// SPDX-License-Identifier: MIT
//
// Stage 2 GPU compute smoke: dispatches the project's kAddF32 WGSL source
// (PR #43, include/tensor/core/backend/webgpu_wgsl.hpp) on the local
// RTX 3090 via Dawn, and verifies the result matches reference within
// 1e-5 (the ADR-0012 f32 tolerance).
//
// This bypasses gpu.cpp entirely — gpu.cpp@0.2.0 has callback-info-struct
// ABI drift vs Dawn 2026-04 (see docs/reports/2026-05-12_gpu-cpp-dawn-
// abi-drift.md). We talk to the Dawn C ABI directly.

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <future>
#include <string>
#include <thread>
#include <vector>

#include <webgpu/webgpu.h>

#include "tensor/core/backend/webgpu_wgsl.hpp"

namespace wgsl = tensor::core::backend::webgpu::wgsl;

namespace {

constexpr std::size_t kN = 1024;
constexpr std::size_t kBytes = kN * sizeof(float);

// Substitute the {{precision}} / {{workgroupSize}} placeholders in a
// WGSL source. The project's WGSL constants are written in the form
// gpu.cpp's KernelCode expects; we do the substitution here ourselves
// since we are not using gpu.cpp.
std::string substituteWgsl(std::string_view tmpl,
                           std::string_view precision,
                           std::string_view workgroupSize) {
    std::string out{tmpl};
    const auto replaceAll = [&](std::string_view from, std::string_view to) {
        std::string::size_type pos = 0;
        while ((pos = out.find(from.data(), pos, from.size())) != std::string::npos) {
            out.replace(pos, from.size(), to.data(), to.size());
            pos += to.size();
        }
    };
    replaceAll("{{precision}}", precision);
    replaceAll("{{workgroupSize}}", workgroupSize);
    return out;
}

WGPUStringView makeStringView(std::string_view sv) {
    return WGPUStringView{sv.data(), sv.size()};
}

template <class Fn>
void pumpUntilReady(WGPUInstance instance, std::future<void>& f, Fn fail) {
    using namespace std::chrono_literals;
    auto deadline = std::chrono::steady_clock::now() + 10s;
    while (f.wait_for(0ms) != std::future_status::ready) {
        if (std::chrono::steady_clock::now() > deadline) {
            fail("pumpUntilReady: timed out after 10s");
            return;
        }
        wgpuInstanceProcessEvents(instance);
        std::this_thread::sleep_for(1ms);
    }
}

struct AdapterResult {
    WGPURequestAdapterStatus status{};
    WGPUAdapter adapter{};
    std::string message;
    std::promise<void> done;
};

struct DeviceResult {
    WGPURequestDeviceStatus status{};
    WGPUDevice device{};
    std::string message;
    std::promise<void> done;
};

struct WorkDoneResult {
    WGPUQueueWorkDoneStatus status{};
    std::promise<void> done;
};

struct MapResult {
    WGPUMapAsyncStatus status{};
    std::string message;
    std::promise<void> done;
};

}  // namespace

int main() {
    int rc = 0;
    auto fail = [&rc](std::string_view msg) {
        std::fprintf(stderr, "FAIL: %.*s\n", static_cast<int>(msg.size()), msg.data());
        rc = 1;
    };

    // ── 1. Instance ──────────────────────────────────────────────────
    WGPUInstanceDescriptor instanceDesc = WGPU_INSTANCE_DESCRIPTOR_INIT;
    WGPUInstance instance = wgpuCreateInstance(&instanceDesc);
    if (!instance) {
        fail("wgpuCreateInstance returned null");
        return rc;
    }

    // ── 2. Adapter ───────────────────────────────────────────────────
    AdapterResult ar;
    auto ar_f = ar.done.get_future();
    {
        WGPURequestAdapterOptions options = WGPU_REQUEST_ADAPTER_OPTIONS_INIT;
        WGPURequestAdapterCallbackInfo cb = WGPU_REQUEST_ADAPTER_CALLBACK_INFO_INIT;
        cb.mode = WGPUCallbackMode_AllowProcessEvents;
        cb.callback = [](WGPURequestAdapterStatus status, WGPUAdapter a,
                         WGPUStringView msg, void* u1, void*) {
            auto* r = static_cast<AdapterResult*>(u1);
            r->status = status;
            r->adapter = a;
            if (msg.data && msg.length > 0) r->message.assign(msg.data, msg.length);
            r->done.set_value();
        };
        cb.userdata1 = &ar;
        (void)wgpuInstanceRequestAdapter(instance, &options, cb);
    }
    pumpUntilReady(instance, ar_f, fail);
    if (rc != 0 || ar.status != WGPURequestAdapterStatus_Success) {
        fail("RequestAdapter failed: " + ar.message);
        wgpuInstanceRelease(instance);
        return rc != 0 ? rc : 1;
    }

    // ── 3. Device ────────────────────────────────────────────────────
    DeviceResult dr;
    auto dr_f = dr.done.get_future();
    {
        WGPUDeviceDescriptor devDesc = WGPU_DEVICE_DESCRIPTOR_INIT;
        WGPURequestDeviceCallbackInfo cb = WGPU_REQUEST_DEVICE_CALLBACK_INFO_INIT;
        cb.mode = WGPUCallbackMode_AllowProcessEvents;
        cb.callback = [](WGPURequestDeviceStatus status, WGPUDevice d,
                         WGPUStringView msg, void* u1, void*) {
            auto* r = static_cast<DeviceResult*>(u1);
            r->status = status;
            r->device = d;
            if (msg.data && msg.length > 0) r->message.assign(msg.data, msg.length);
            r->done.set_value();
        };
        cb.userdata1 = &dr;
        (void)wgpuAdapterRequestDevice(ar.adapter, &devDesc, cb);
    }
    pumpUntilReady(instance, dr_f, fail);
    if (rc != 0 || dr.status != WGPURequestDeviceStatus_Success) {
        fail("RequestDevice failed: " + dr.message);
        wgpuAdapterRelease(ar.adapter);
        wgpuInstanceRelease(instance);
        return rc != 0 ? rc : 1;
    }
    WGPUDevice device = dr.device;
    WGPUQueue queue = wgpuDeviceGetQueue(device);

    // ── 4. Host buffers ──────────────────────────────────────────────
    std::vector<float> a(kN), b(kN), out(kN, std::nan(""));
    for (std::size_t i = 0; i < kN; ++i) {
        a[i] = static_cast<float>(i);
        b[i] = 2.0f * static_cast<float>(i);
    }

    // ── 5. GPU buffers ───────────────────────────────────────────────
    auto makeBuf = [&](WGPUBufferUsage usage, const char* label) {
        WGPUBufferDescriptor d = WGPU_BUFFER_DESCRIPTOR_INIT;
        d.size = kBytes;
        d.usage = usage;
        d.label = makeStringView(label);
        return wgpuDeviceCreateBuffer(device, &d);
    };
    WGPUBuffer gA   = makeBuf(WGPUBufferUsage_Storage | WGPUBufferUsage_CopyDst, "a");
    WGPUBuffer gB   = makeBuf(WGPUBufferUsage_Storage | WGPUBufferUsage_CopyDst, "b");
    WGPUBuffer gOut = makeBuf(WGPUBufferUsage_Storage | WGPUBufferUsage_CopySrc, "out");
    WGPUBuffer gStaging = makeBuf(WGPUBufferUsage_MapRead | WGPUBufferUsage_CopyDst, "staging");

    wgpuQueueWriteBuffer(queue, gA, 0, a.data(), kBytes);
    wgpuQueueWriteBuffer(queue, gB, 0, b.data(), kBytes);

    // ── 6. Shader module from the project's WGSL source ──────────────
    std::string wgslSrc = substituteWgsl(
        wgsl::kAddF32, "f32", std::to_string(wgsl::kDefaultWorkgroupSize));

    WGPUShaderSourceWGSL wgslDesc = WGPU_SHADER_SOURCE_WGSL_INIT;
    wgslDesc.code = makeStringView(wgslSrc);

    WGPUShaderModuleDescriptor smDesc = WGPU_SHADER_MODULE_DESCRIPTOR_INIT;
    smDesc.nextInChain = reinterpret_cast<WGPUChainedStruct*>(&wgslDesc);
    smDesc.label = makeStringView("kAddF32");
    WGPUShaderModule shaderModule = wgpuDeviceCreateShaderModule(device, &smDesc);

    // ── 7. BindGroupLayout + PipelineLayout ──────────────────────────
    WGPUBindGroupLayoutEntry bgleA = WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT;
    bgleA.binding = 0;
    bgleA.visibility = WGPUShaderStage_Compute;
    bgleA.buffer.type = WGPUBufferBindingType_ReadOnlyStorage;

    WGPUBindGroupLayoutEntry bgleB = WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT;
    bgleB.binding = 1;
    bgleB.visibility = WGPUShaderStage_Compute;
    bgleB.buffer.type = WGPUBufferBindingType_ReadOnlyStorage;

    WGPUBindGroupLayoutEntry bgleOut = WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT;
    bgleOut.binding = 2;
    bgleOut.visibility = WGPUShaderStage_Compute;
    bgleOut.buffer.type = WGPUBufferBindingType_Storage;

    WGPUBindGroupLayoutEntry bgles[3] = {bgleA, bgleB, bgleOut};
    WGPUBindGroupLayoutDescriptor bglDesc = WGPU_BIND_GROUP_LAYOUT_DESCRIPTOR_INIT;
    bglDesc.entryCount = 3;
    bglDesc.entries = bgles;
    WGPUBindGroupLayout bgl = wgpuDeviceCreateBindGroupLayout(device, &bglDesc);

    WGPUPipelineLayoutDescriptor plDesc = WGPU_PIPELINE_LAYOUT_DESCRIPTOR_INIT;
    plDesc.bindGroupLayoutCount = 1;
    plDesc.bindGroupLayouts = &bgl;
    WGPUPipelineLayout pl = wgpuDeviceCreatePipelineLayout(device, &plDesc);

    // ── 8. ComputePipeline ───────────────────────────────────────────
    WGPUComputePipelineDescriptor cpDesc = WGPU_COMPUTE_PIPELINE_DESCRIPTOR_INIT;
    cpDesc.layout = pl;
    cpDesc.compute.module = shaderModule;
    cpDesc.compute.entryPoint = makeStringView("main");
    WGPUComputePipeline pipeline = wgpuDeviceCreateComputePipeline(device, &cpDesc);

    // ── 9. BindGroup ─────────────────────────────────────────────────
    WGPUBindGroupEntry bge[3];
    for (auto& e : bge) e = WGPU_BIND_GROUP_ENTRY_INIT;
    bge[0].binding = 0; bge[0].buffer = gA;   bge[0].offset = 0; bge[0].size = kBytes;
    bge[1].binding = 1; bge[1].buffer = gB;   bge[1].offset = 0; bge[1].size = kBytes;
    bge[2].binding = 2; bge[2].buffer = gOut; bge[2].offset = 0; bge[2].size = kBytes;

    WGPUBindGroupDescriptor bgDesc = WGPU_BIND_GROUP_DESCRIPTOR_INIT;
    bgDesc.layout = bgl;
    bgDesc.entryCount = 3;
    bgDesc.entries = bge;
    WGPUBindGroup bg = wgpuDeviceCreateBindGroup(device, &bgDesc);

    // ── 10. Encode + dispatch + copy out → staging ───────────────────
    WGPUCommandEncoderDescriptor ceDesc = WGPU_COMMAND_ENCODER_DESCRIPTOR_INIT;
    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device, &ceDesc);

    WGPUComputePassDescriptor cpassDesc = WGPU_COMPUTE_PASS_DESCRIPTOR_INIT;
    WGPUComputePassEncoder pass = wgpuCommandEncoderBeginComputePass(encoder, &cpassDesc);
    wgpuComputePassEncoderSetPipeline(pass, pipeline);
    wgpuComputePassEncoderSetBindGroup(pass, 0, bg, 0, nullptr);
    const std::uint32_t totalGroups =
        static_cast<std::uint32_t>((kN + wgsl::kDefaultWorkgroupSize - 1) /
                                   wgsl::kDefaultWorkgroupSize);
    wgpuComputePassEncoderDispatchWorkgroups(pass, totalGroups, 1, 1);
    wgpuComputePassEncoderEnd(pass);
    wgpuComputePassEncoderRelease(pass);

    wgpuCommandEncoderCopyBufferToBuffer(encoder, gOut, 0, gStaging, 0, kBytes);

    WGPUCommandBufferDescriptor cbDesc = WGPU_COMMAND_BUFFER_DESCRIPTOR_INIT;
    WGPUCommandBuffer cmd = wgpuCommandEncoderFinish(encoder, &cbDesc);
    wgpuCommandEncoderRelease(encoder);
    wgpuQueueSubmit(queue, 1, &cmd);
    wgpuCommandBufferRelease(cmd);

    // ── 11. Wait for queue ───────────────────────────────────────────
    WorkDoneResult wd;
    auto wd_f = wd.done.get_future();
    {
        WGPUQueueWorkDoneCallbackInfo cb = WGPU_QUEUE_WORK_DONE_CALLBACK_INFO_INIT;
        cb.mode = WGPUCallbackMode_AllowProcessEvents;
        cb.callback = [](WGPUQueueWorkDoneStatus status, WGPUStringView,
                         void* u1, void*) {
            auto* r = static_cast<WorkDoneResult*>(u1);
            r->status = status;
            r->done.set_value();
        };
        cb.userdata1 = &wd;
        (void)wgpuQueueOnSubmittedWorkDone(queue, cb);
    }
    pumpUntilReady(instance, wd_f, fail);
    if (rc != 0) goto cleanup;
    if (wd.status != WGPUQueueWorkDoneStatus_Success) {
        fail("Queue work-done failed");
        goto cleanup;
    }

    // ── 12. MapAsync staging buffer ──────────────────────────────────
    {
        MapResult mr;
        auto mr_f = mr.done.get_future();
        WGPUBufferMapCallbackInfo cb = WGPU_BUFFER_MAP_CALLBACK_INFO_INIT;
        cb.mode = WGPUCallbackMode_AllowProcessEvents;
        cb.callback = [](WGPUMapAsyncStatus status, WGPUStringView msg,
                         void* u1, void*) {
            auto* r = static_cast<MapResult*>(u1);
            r->status = status;
            if (msg.data && msg.length > 0) r->message.assign(msg.data, msg.length);
            r->done.set_value();
        };
        cb.userdata1 = &mr;
        (void)wgpuBufferMapAsync(gStaging, WGPUMapMode_Read, 0, kBytes, cb);
        pumpUntilReady(instance, mr_f, fail);
        if (rc != 0) goto cleanup;
        if (mr.status != WGPUMapAsyncStatus_Success) {
            fail("BufferMapAsync failed: " + mr.message);
            goto cleanup;
        }

        const float* mapped = static_cast<const float*>(
            wgpuBufferGetConstMappedRange(gStaging, 0, kBytes));
        if (!mapped) {
            fail("wgpuBufferGetConstMappedRange returned null");
            goto cleanup;
        }
        std::memcpy(out.data(), mapped, kBytes);
        wgpuBufferUnmap(gStaging);
    }

    // ── 13. Verify ───────────────────────────────────────────────────
    {
        std::size_t mismatches = 0;
        float max_err = 0.0f;
        for (std::size_t i = 0; i < kN; ++i) {
            const float expected = a[i] + b[i];
            const float err = std::fabs(out[i] - expected);
            if (err > max_err) max_err = err;
            if (err > 1e-5f) {
                if (mismatches < 5) {
                    std::fprintf(stderr, "  mismatch i=%zu expected=%g got=%g err=%g\n",
                                 i, expected, out[i], err);
                }
                ++mismatches;
            }
        }
        std::printf("kAddF32 dispatched on RTX 3090 via Dawn:\n");
        std::printf("  N=%zu  workgroup=%zu  totalGroups=%u\n",
                    kN, wgsl::kDefaultWorkgroupSize, totalGroups);
        std::printf("  mismatches: %zu / %zu\n", mismatches, kN);
        std::printf("  max |err|:  %g\n", static_cast<double>(max_err));
        std::printf("  result:     %s\n", mismatches == 0 ? "PASS" : "FAIL");
        if (mismatches != 0) rc = 1;
    }

cleanup:
    wgpuBindGroupRelease(bg);
    wgpuComputePipelineRelease(pipeline);
    wgpuPipelineLayoutRelease(pl);
    wgpuBindGroupLayoutRelease(bgl);
    wgpuShaderModuleRelease(shaderModule);
    wgpuBufferRelease(gStaging);
    wgpuBufferRelease(gOut);
    wgpuBufferRelease(gB);
    wgpuBufferRelease(gA);
    wgpuQueueRelease(queue);
    wgpuDeviceRelease(device);
    wgpuAdapterRelease(ar.adapter);
    wgpuInstanceRelease(instance);
    return rc;
}
