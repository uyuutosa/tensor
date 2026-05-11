// SPDX-License-Identifier: MIT
//
// Stage 2 GPU smoke: dispatch the project's kAddF32 WGSL source through
// gpu.cpp + Dawn (vcpkg-installed) on the local RTX 3090. Verifies the
// shipped WGSL kernel (PR #43) produces the right output before we wire
// it into the project's webgpu::Backend (Stage 3 / P3.M3.2 PR).
//
// This file is intentionally outside CMakeLists.txt — built ad hoc via a
// one-shot g++ invocation that links libwebgpu_dawn.so from
// ${VCPKG_INSTALLED}/x64-linux/lib/. Once Stage 3 lands the project's
// CMake takes over.

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <future>
#include <iostream>
#include <string>
#include <vector>

#include "gpu.hpp"
#include "tensor/core/backend/webgpu_wgsl.hpp"

namespace wgsl = tensor::core::backend::webgpu::wgsl;

namespace {

bool approx_equal(float a, float b, float tol = 1e-5f) {
    return std::fabs(a - b) <= tol;
}

}  // namespace

int main() {
    constexpr std::size_t kN = 1024;

    // 1. Create gpu.cpp Context. On Linux + NVIDIA this resolves to Dawn
    //    over Vulkan via the NVIDIA driver's ICD.
    auto ctx = gpu::createContext();

    // 2. Allocate device-side tensors (f32, flat shape {kN}).
    auto gA   = gpu::createTensor(ctx, {kN}, gpu::kf32);
    auto gB   = gpu::createTensor(ctx, {kN}, gpu::kf32);
    auto gOut = gpu::createTensor(ctx, {kN}, gpu::kf32);

    // 3. Host data: a[i] = i, b[i] = 2i.
    std::vector<float> a(kN), b(kN), out(kN, -1.0f);
    for (std::size_t i = 0; i < kN; ++i) {
        a[i] = static_cast<float>(i);
        b[i] = static_cast<float>(i) * 2.0f;
    }

    // 4. Upload inputs.
    gpu::toGPU(ctx, a.data(), gA);
    gpu::toGPU(ctx, b.data(), gB);

    // 5. Build the kernel from the project's WGSL source. The
    //    KernelCode constructor substitutes {{precision}} and
    //    {{workgroupSize}} per gpu.hpp:291-389.
    gpu::KernelCode code{
        std::string{wgsl::kAddF32},
        wgsl::kDefaultWorkgroupSize,
        gpu::kf32,
    };

    // 6. Create + dispatch.
    gpu::Bindings bindings{gA, gB, gOut};
    gpu::Shape totalWorkgroups{
        gpu::cdiv(kN, wgsl::kDefaultWorkgroupSize),
        1,
        1,
    };
    auto kernel = gpu::createKernel(ctx, code, bindings, totalWorkgroups);

    std::promise<void> p;
    auto f = p.get_future();
    gpu::dispatchKernel(ctx, kernel, p);
    gpu::wait(ctx, f);

    // 7. Download output.
    gpu::toCPU(ctx, gOut, out.data(), kN * sizeof(float));

    // 8. Verify a[i] + b[i] == out[i] within 1e-5 (the ADR-0012 tolerance
    //    for f32).
    std::size_t mismatches = 0;
    float max_abs_err = 0.0f;
    for (std::size_t i = 0; i < kN; ++i) {
        const float expected = a[i] + b[i];  // 3 * i
        const float err = std::fabs(out[i] - expected);
        if (err > max_abs_err) max_abs_err = err;
        if (!approx_equal(out[i], expected)) {
            if (mismatches < 5) {
                std::cerr << "  mismatch at i=" << i
                          << " expected=" << expected
                          << " got=" << out[i]
                          << " err=" << err << "\n";
            }
            ++mismatches;
        }
    }

    std::cout << "kAddF32 smoke on RTX 3090 (N=" << kN << "):\n"
              << "  mismatches: " << mismatches << " / " << kN << "\n"
              << "  max |err|:  " << max_abs_err << "\n"
              << "  result:     " << (mismatches == 0 ? "PASS" : "FAIL") << "\n";
    return mismatches == 0 ? 0 : 1;
}
