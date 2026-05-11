// SPDX-License-Identifier: MIT
//
// Stage 2 minimal Dawn smoke: verifies the vcpkg-installed Dawn library
// links against the local RTX 3090's Vulkan stack and that an instance +
// adapter can be requested. Does *not* dispatch a kernel — that's
// blocked on the gpu.cpp@0.2.0 ABI drift (see
// docs/reports/2026-05-12_gpu-cpp-dawn-abi-drift.md).
//
// Build: g++ -std=c++20 against libwebgpu_dawn.a + system libvulkan.so.1.
// Success criteria: program exits 0 with an adapter description printed.

#include <cstdio>
#include <cstring>
#include <future>
#include <string>

#include <webgpu/webgpu.h>

namespace {

struct AdapterRequest {
    WGPURequestAdapterStatus status{};
    WGPUAdapter adapter{};
    std::string message;
    std::promise<void> done;
};

}  // namespace

int main() {
    // 1. Create instance (Dawn's default toggles, all backends).
    WGPUInstanceDescriptor instanceDesc = WGPU_INSTANCE_DESCRIPTOR_INIT;
    WGPUInstance instance = wgpuCreateInstance(&instanceDesc);
    if (!instance) {
        std::fprintf(stderr, "wgpuCreateInstance returned null\n");
        return 1;
    }

    // 2. Request adapter. Dawn 2026-04 uses the callback-info struct
    //    pattern; this is the exact API drift that broke gpu.cpp@0.2.0.
    AdapterRequest req;
    auto future_in = req.done.get_future();

    WGPURequestAdapterOptions options = WGPU_REQUEST_ADAPTER_OPTIONS_INIT;
    WGPURequestAdapterCallbackInfo cbInfo = WGPU_REQUEST_ADAPTER_CALLBACK_INFO_INIT;
    cbInfo.mode = WGPUCallbackMode_AllowProcessEvents;
    cbInfo.callback = [](WGPURequestAdapterStatus status,
                         WGPUAdapter adapter,
                         WGPUStringView message,
                         void* userdata1,
                         void* /*userdata2*/) {
        auto* r = static_cast<AdapterRequest*>(userdata1);
        r->status = status;
        r->adapter = adapter;
        if (message.data && message.length > 0) {
            r->message.assign(message.data, message.length);
        }
        r->done.set_value();
    };
    cbInfo.userdata1 = &req;
    (void)wgpuInstanceRequestAdapter(instance, &options, cbInfo);

    // 3. Pump events until the future is set.
    while (future_in.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
        wgpuInstanceProcessEvents(instance);
    }

    if (req.status != WGPURequestAdapterStatus_Success) {
        std::fprintf(stderr, "RequestAdapter failed: status=%d msg=%s\n",
                     static_cast<int>(req.status), req.message.c_str());
        wgpuInstanceRelease(instance);
        return 1;
    }

    // 4. Inspect the adapter to confirm it is the RTX 3090.
    WGPUAdapterInfo info = WGPU_ADAPTER_INFO_INIT;
    wgpuAdapterGetInfo(req.adapter, &info);

    std::printf("Dawn instance + adapter OK on this machine.\n");
    std::printf("  vendor:       %.*s\n",
                static_cast<int>(info.vendor.length),
                info.vendor.data ? info.vendor.data : "");
    std::printf("  architecture: %.*s\n",
                static_cast<int>(info.architecture.length),
                info.architecture.data ? info.architecture.data : "");
    std::printf("  device:       %.*s\n",
                static_cast<int>(info.device.length),
                info.device.data ? info.device.data : "");
    std::printf("  description:  %.*s\n",
                static_cast<int>(info.description.length),
                info.description.data ? info.description.data : "");
    std::printf("  backendType:  %d\n", static_cast<int>(info.backendType));
    std::printf("  adapterType:  %d\n", static_cast<int>(info.adapterType));

    wgpuAdapterInfoFreeMembers(info);
    wgpuAdapterRelease(req.adapter);
    wgpuInstanceRelease(instance);
    return 0;
}
