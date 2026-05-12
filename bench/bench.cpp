// SPDX-License-Identifier: MIT
//
// bench/bench.cpp — backend performance comparison framework.
//
// Realises Phase 2.5 P2.5.M4. Times a fixed set of operations across
// the available KernelBackend adapters and prints a comparison table.
// Output is text-only; the report `docs/reports/<date>_backend-
// performance-comparison.md` lifts the table into a dated Layer B doc.
//
// Build:
//   cmake --preset=default -DTENSOR_BUILD_BENCH=ON -DTENSOR_KERNEL_BACKEND=eigen
//   cmake --build --preset=default --target tensor_bench
//
// Run:
//   ./build/default/bench/tensor_bench [iterations]

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include <tensor/core/axis.hpp>
#include <tensor/core/backend/reference.hpp>
#include <tensor/core/broadcast.hpp>
#include <tensor/core/contract.hpp>
#include <tensor/core/dynamic_shape.hpp>
#include <tensor/core/dynamic_tensor.hpp>

#if defined(TENSOR_HAS_EIGEN)
#  include <tensor/core/backend/eigen.hpp>
#endif

#if defined(TENSOR_HAS_WEBGPU)
#  include <tensor/core/backend/webgpu.hpp>
#endif

using tensor::core::Axis;
using tensor::core::DynamicShape;
using tensor::core::DynamicTensor;
using clock_t_ = std::chrono::high_resolution_clock;

namespace {

constexpr int kDefaultIters = 5;

template <class F>
double time_average_ms(F&& f, int iters) {
    // Warm up once.
    (void)f();
    double total_ms = 0.0;
    for (int i = 0; i < iters; ++i) {
        auto const t0 = clock_t_::now();
        (void)f();
        auto const t1 = clock_t_::now();
        total_ms += std::chrono::duration<double, std::milli>(t1 - t0).count();
    }
    return total_ms / iters;
}

DynamicTensor<double> random_tensor(DynamicShape const& shape, std::mt19937& rng) {
    std::uniform_real_distribution<double> dist(-1.0, 1.0);
    std::vector<double> buf(shape.total());
    for (auto& v : buf) v = dist(rng);
    return DynamicTensor<double>(shape, std::move(buf));
}

DynamicTensor<float> random_tensor_f32(DynamicShape const& shape, std::mt19937& rng) {
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    std::vector<float> buf(shape.total());
    for (auto& v : buf) v = dist(rng);
    return DynamicTensor<float>(shape, std::move(buf));
}

void print_row(std::string const& bench, std::string const& backend, double ms) {
    std::cout << "  " << std::left << std::setw(28) << bench
              << std::setw(12) << backend
              << std::right << std::setw(10) << std::fixed << std::setprecision(3)
              << ms << " ms\n";
}

}  // namespace

int main(int argc, char** argv) {
    int iters = kDefaultIters;
    if (argc >= 2) iters = std::atoi(argv[1]);

    std::mt19937 rng(42);

    // ── Element-wise: 1M-element add ─────────────────────────────────────
    DynamicShape vec_shape{Axis{"i", 1'000'000}};
    auto a_big = random_tensor(vec_shape, rng);
    auto b_big = random_tensor(vec_shape, rng);

    // ── Matvec: 1024 × 1024 W × 1024 x ──────────────────────────────────
    DynamicShape W_shape{Axis{"i", 1024}, Axis{"j", 1024}};
    DynamicShape x_shape{Axis{"j", 1024}};
    auto W_t = random_tensor(W_shape, rng);
    auto x_t = random_tensor(x_shape, rng);
    auto mv_plan = tensor::core::contract_plan(W_shape, x_shape);

    // ── Matmul: 512 × 512 × 512 ─────────────────────────────────────────
    DynamicShape A_shape{Axis{"i", 512}, Axis{"j", 512}};
    DynamicShape B_shape{Axis{"j", 512}, Axis{"k", 512}};
    auto A_t = random_tensor(A_shape, rng);
    auto B_t = random_tensor(B_shape, rng);
    auto mm_plan = tensor::core::contract_plan(A_shape, B_shape);

    std::cout << "tensor benchmark — " << iters << " iterations per case\n\n";
    std::cout << "  " << std::left << std::setw(28) << "case"
              << std::setw(12) << "backend"
              << std::right << std::setw(10) << "average\n";
    std::cout << std::string(64, '-') << '\n';

    tensor::core::backend::reference::Backend ref;
    print_row("add (1M elements)", "reference",
              time_average_ms([&] { return ref.add(a_big, b_big); }, iters));
    print_row("matvec (1024 × 1024 × 1024)", "reference",
              time_average_ms([&] { return ref.contract(W_t, x_t, mv_plan); }, iters));
    print_row("matmul (512 × 512 × 512)", "reference",
              time_average_ms([&] { return ref.contract(A_t, B_t, mm_plan); }, iters));

#if defined(TENSOR_HAS_EIGEN)
    tensor::core::backend::eigen::Backend eig;
    print_row("add (1M elements)", "eigen",
              time_average_ms([&] { return eig.add(a_big, b_big); }, iters));
    print_row("matvec (1024 × 1024 × 1024)", "eigen",
              time_average_ms([&] { return eig.contract(W_t, x_t, mv_plan); }, iters));
    print_row("matmul (512 × 512 × 512)", "eigen",
              time_average_ms([&] { return eig.contract(A_t, B_t, mm_plan); }, iters));
#else
    std::cout << "\n  (Eigen adapter not built — re-configure with -DTENSOR_KERNEL_BACKEND=eigen)\n";
#endif

#if defined(TENSOR_HAS_WEBGPU)
    // ── WebGPU only dispatches f32 (ADR-0012 § f32-only MVP) — build
    //    fresh float-typed tensors for an apples-to-apples comparison
    //    against reference on the same f32 inputs.
    auto a_big_f = random_tensor_f32(vec_shape, rng);
    auto b_big_f = random_tensor_f32(vec_shape, rng);
    auto W_t_f = random_tensor_f32(W_shape, rng);
    auto x_t_f = random_tensor_f32(x_shape, rng);
    auto A_t_f = random_tensor_f32(A_shape, rng);
    auto B_t_f = random_tensor_f32(B_shape, rng);

    std::cout << '\n';
    std::cout << "  (f32 — webgpu vs reference on identical inputs; "
                 "host↔device roundtrip included)\n";

    print_row("add (1M elements, f32)", "reference",
              time_average_ms([&] { return ref.add(a_big_f, b_big_f); }, iters));
    print_row("matvec (1024 × 1024 × 1024, f32)", "reference",
              time_average_ms([&] { return ref.contract(W_t_f, x_t_f, mv_plan); }, iters));
    print_row("matmul (512 × 512 × 512, f32)", "reference",
              time_average_ms([&] { return ref.contract(A_t_f, B_t_f, mm_plan); }, iters));

    tensor::core::backend::webgpu::Backend wgpu;
    // Warm-up: the first call lazy-creates the Dawn instance / adapter /
    // device + compiles the kernel; that one-time cost dwarfs the per-op
    // time. `time_average_ms` already warms before timing, but we touch
    // the device once here so the Dawn init isn't attributed to "add".
    (void)wgpu.add(a_big_f, b_big_f);

    print_row("add (1M elements, f32)", "webgpu",
              time_average_ms([&] { return wgpu.add(a_big_f, b_big_f); }, iters));
    print_row("matvec (1024 × 1024 × 1024, f32)", "webgpu",
              time_average_ms([&] { return wgpu.contract(W_t_f, x_t_f, mv_plan); }, iters));
    print_row("matmul (512 × 512 × 512, f32)", "webgpu",
              time_average_ms([&] { return wgpu.contract(A_t_f, B_t_f, mm_plan); }, iters));
#else
    std::cout << "\n  (WebGPU adapter not built — re-configure with -DTENSOR_KERNEL_BACKEND=webgpu)\n";
#endif

    std::cout << "\nDone. Copy the table into docs/reports/<date>_backend-performance-comparison.md.\n";
    return 0;
}
