// SPDX-License-Identifier: MIT
//
// tensor::core::backend::webgpu::detail::WebGPUContext — Dawn instance /
// adapter / device / queue lifetime manager. Thread-local singleton per
// ADR-0014's "one Backend per thread" guidance and the detailed-design
// doc's discussion of context lifetime
// (docs/detailed-design/webgpu-element-wise-kernels.md §3 "Context lifetime").
//
// Per ADR-0016, this code talks to Dawn via Dawn's own RAII C++ wrapper
// `<webgpu/webgpu_cpp.h>` — gpu.cpp is no longer load-bearing. The init
// path drives async adapter / device requests via ProcessEvents loops;
// once initialised, every backend method on this thread reuses the
// device + queue.

#pragma once

#if !defined(TENSOR_HAS_WEBGPU)
#    error "tensor/core/backend/webgpu_detail/context.hpp included without \
TENSOR_HAS_WEBGPU; configure with cmake -DTENSOR_KERNEL_BACKEND=webgpu."
#endif

#include <chrono>
#include <stdexcept>
#include <string>
#include <thread>

#include <webgpu/webgpu_cpp.h>

namespace tensor::core::backend::webgpu::detail {

// Pumps `instance.ProcessEvents()` until `done` flips to true, or
// `timeout_seconds` elapse — whichever comes first. Throws on timeout.
inline void pump_until(wgpu::Instance const& instance, bool const& done,
                       int timeout_seconds = 10) {
    auto const deadline =
        std::chrono::steady_clock::now() + std::chrono::seconds{timeout_seconds};
    while (!done) {
        if (std::chrono::steady_clock::now() > deadline) {
            throw std::runtime_error{
                "WebGPU async wait timed out (Dawn ProcessEvents loop)"};
        }
        // ProcessEvents() drives Dawn's WaitAnyOnly / AllowProcessEvents
        // callback modes. `mut` is required because the method is not
        // marked const in webgpu_cpp.h.
        const_cast<wgpu::Instance&>(instance).ProcessEvents();
        std::this_thread::sleep_for(std::chrono::microseconds{200});
    }
}

class WebGPUContext {
public:
    // Per-thread singleton. ADR-0014 §8 §3 documents the single-threaded
    // user-side assumption; multi-threaded users get one Context each.
    static WebGPUContext& current() {
        thread_local WebGPUContext ctx;
        return ctx;
    }

    // Lazily initialise on first use. Idempotent.
    void ensure_initialized() {
        if (initialized_) return;

        // Instance.
        wgpu::InstanceDescriptor instanceDesc{};
        instance_ = wgpu::CreateInstance(&instanceDesc);
        if (!instance_) {
            throw std::runtime_error{"wgpu::CreateInstance returned null"};
        }

        // Adapter — async via ProcessEvents.
        wgpu::Adapter adapter_out;
        bool adapter_done = false;
        std::string adapter_msg;
        {
            wgpu::RequestAdapterOptions opts{};
            instance_.RequestAdapter(
                &opts, wgpu::CallbackMode::AllowProcessEvents,
                [&](wgpu::RequestAdapterStatus status, wgpu::Adapter adapter,
                    wgpu::StringView msg) {
                    if (status == wgpu::RequestAdapterStatus::Success) {
                        adapter_out = std::move(adapter);
                    } else {
                        adapter_msg.assign(msg.data, msg.length);
                    }
                    adapter_done = true;
                });
        }
        pump_until(instance_, adapter_done);
        if (!adapter_out) {
            throw std::runtime_error{
                "wgpu::Instance::RequestAdapter failed: " + adapter_msg};
        }

        // Device — async via ProcessEvents.
        wgpu::Device device_out;
        bool device_done = false;
        std::string device_msg;
        {
            wgpu::DeviceDescriptor devDesc{};
            adapter_out.RequestDevice(
                &devDesc, wgpu::CallbackMode::AllowProcessEvents,
                [&](wgpu::RequestDeviceStatus status, wgpu::Device device,
                    wgpu::StringView msg) {
                    if (status == wgpu::RequestDeviceStatus::Success) {
                        device_out = std::move(device);
                    } else {
                        device_msg.assign(msg.data, msg.length);
                    }
                    device_done = true;
                });
        }
        pump_until(instance_, device_done);
        if (!device_out) {
            throw std::runtime_error{
                "wgpu::Adapter::RequestDevice failed: " + device_msg};
        }

        adapter_ = std::move(adapter_out);
        device_ = std::move(device_out);
        queue_ = device_.GetQueue();
        initialized_ = true;
    }

    [[nodiscard]] wgpu::Instance& instance() {
        ensure_initialized();
        return instance_;
    }
    [[nodiscard]] wgpu::Device& device() {
        ensure_initialized();
        return device_;
    }
    [[nodiscard]] wgpu::Queue& queue() {
        ensure_initialized();
        return queue_;
    }

private:
    bool initialized_ = false;
    wgpu::Instance instance_;
    wgpu::Adapter adapter_;
    wgpu::Device device_;
    wgpu::Queue queue_;
};

}  // namespace tensor::core::backend::webgpu::detail
