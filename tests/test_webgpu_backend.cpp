// SPDX-License-Identifier: MIT
//
// Phase 3 P3.M2 — WebGPU Backend stub satisfies KernelBackend and
// produces results that agree with reference (trivially, since every
// method delegates to reference).
//
// As Phase 3 P3.M3+ replaces method bodies with WGSL kernels, this
// test suite is the cross-validation anchor — outputs must continue
// to match reference within a documented tolerance (1e-5 for float
// per ADR-0012).

#if defined(TENSOR_HAS_WEBGPU)

#include <doctest/doctest.h>

#include <tensor/core/axis.hpp>
#include <tensor/core/backend/reference.hpp>
#include <tensor/core/backend/webgpu.hpp>
#include <tensor/core/broadcast.hpp>
#include <tensor/core/contract.hpp>
#include <tensor/core/dynamic_shape.hpp>
#include <tensor/core/dynamic_tensor.hpp>

using tensor::core::Axis;
using tensor::core::DynamicShape;
using tensor::core::DynamicTensor;
using WebGPUBackend = tensor::core::backend::webgpu::Backend;
using RefBackend = tensor::core::backend::reference::Backend;

namespace {
constexpr double kTol = 1e-9;
}

TEST_CASE("webgpu::Backend satisfies KernelBackend") {
    static_assert(tensor::core::KernelBackend<WebGPUBackend>);
}

TEST_CASE("webgpu vs reference: element-wise binary ops agree (stub delegates)") {
    WebGPUBackend w;
    RefBackend r;
    DynamicTensor<double> a(DynamicShape{Axis{"i", 5}}, {1, 2, 3, 4, 5});
    DynamicTensor<double> b(DynamicShape{Axis{"i", 5}}, {10, 20, 30, 40, 50});
    for (std::size_t i = 0; i < 5; ++i) {
        CHECK(w.add(a, b)[i] == doctest::Approx(r.add(a, b)[i]).epsilon(kTol));
        CHECK(w.sub(a, b)[i] == doctest::Approx(r.sub(a, b)[i]).epsilon(kTol));
        CHECK(w.mul(a, b)[i] == doctest::Approx(r.mul(a, b)[i]).epsilon(kTol));
        CHECK(w.div(a, b)[i] == doctest::Approx(r.div(a, b)[i]).epsilon(kTol));
    }
}

TEST_CASE("webgpu vs reference: matvec contract agrees") {
    WebGPUBackend w;
    RefBackend r;
    DynamicTensor<double> W_(DynamicShape{Axis{"i", 4}, Axis{"j", 3}},
                             {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12});
    DynamicTensor<double> x(DynamicShape{Axis{"j", 3}}, {2, 3, 4});
    auto plan = tensor::core::contract_plan(W_.shape(), x.shape());
    auto y_w = w.contract(W_, x, plan);
    auto y_r = r.contract(W_, x, plan);
    REQUIRE(y_w.size() == y_r.size());
    for (std::size_t i = 0; i < y_w.size(); ++i) {
        CHECK(y_w[i] == doctest::Approx(y_r[i]).epsilon(kTol));
    }
}

TEST_CASE("webgpu vs reference: reduce_sum agrees") {
    WebGPUBackend w;
    RefBackend r;
    DynamicTensor<double> a(DynamicShape{Axis{"i", 6}}, {1, 2, 3, 4, 5, 6});
    CHECK(w.reduce_sum(a) == doctest::Approx(r.reduce_sum(a)).epsilon(kTol));
}

TEST_CASE("webgpu vs reference: broadcast + unbroadcast agree (stub delegates)") {
    WebGPUBackend w;
    RefBackend r;
    DynamicTensor<double> a(DynamicShape{Axis{"i", 2}}, {1, 2});
    DynamicTensor<double> b(DynamicShape{Axis{"j", 3}}, {10, 20, 30});
    auto plan = tensor::core::broadcast_shapes(a.shape(), b.shape());
    auto sum_w = w.broadcast_add(a, b, plan);
    auto sum_r = r.broadcast_add(a, b, plan);
    REQUIRE(sum_w.size() == sum_r.size());
    for (std::size_t i = 0; i < sum_w.size(); ++i) {
        CHECK(sum_w[i] == doctest::Approx(sum_r[i]).epsilon(kTol));
    }
    DynamicTensor<double> grad(plan.result, std::vector<double>(sum_w.size(), 1.0));
    auto ub_w = w.unbroadcast(grad, plan.a_source, a.shape());
    auto ub_r = r.unbroadcast(grad, plan.a_source, a.shape());
    REQUIRE(ub_w.size() == ub_r.size());
    for (std::size_t i = 0; i < ub_w.size(); ++i) {
        CHECK(ub_w[i] == doctest::Approx(ub_r[i]).epsilon(kTol));
    }
}

#endif  // TENSOR_HAS_WEBGPU
