// SPDX-License-Identifier: MIT
//
// Phase 2.5 P2.5.M3 — Eigen Backend produces results that agree with
// the reference adapter on every op the port exposes.

#if defined(TENSOR_HAS_EIGEN)

#include <doctest/doctest.h>

#include <tensor/core/axis.hpp>
#include <tensor/core/backend/eigen.hpp>
#include <tensor/core/backend/reference.hpp>
#include <tensor/core/broadcast.hpp>
#include <tensor/core/contract.hpp>
#include <tensor/core/dynamic_shape.hpp>
#include <tensor/core/dynamic_tensor.hpp>

using tensor::core::Axis;
using tensor::core::DynamicShape;
using tensor::core::DynamicTensor;
using EigenBackend = tensor::core::backend::eigen::Backend;
using RefBackend = tensor::core::backend::reference::Backend;

namespace {
constexpr double kTol = 1e-9;
}

TEST_CASE("eigen::Backend satisfies KernelBackend") {
    static_assert(tensor::core::KernelBackend<EigenBackend>);
}

TEST_CASE("eigen vs reference: element-wise binary ops agree") {
    EigenBackend e;
    RefBackend r;
    DynamicTensor<double> a(DynamicShape{Axis{"i", 5}}, {1, 2, 3, 4, 5});
    DynamicTensor<double> b(DynamicShape{Axis{"i", 5}}, {10, 20, 30, 40, 50});
    for (std::size_t i = 0; i < 5; ++i) {
        CHECK(e.add(a, b)[i] == doctest::Approx(r.add(a, b)[i]).epsilon(kTol));
        CHECK(e.sub(a, b)[i] == doctest::Approx(r.sub(a, b)[i]).epsilon(kTol));
        CHECK(e.mul(a, b)[i] == doctest::Approx(r.mul(a, b)[i]).epsilon(kTol));
        CHECK(e.div(a, b)[i] == doctest::Approx(r.div(a, b)[i]).epsilon(kTol));
    }
}

TEST_CASE("eigen vs reference: element-wise unary ops agree") {
    EigenBackend e;
    RefBackend r;
    DynamicTensor<double> a(DynamicShape{Axis{"i", 5}}, {-1.0, -0.1, 0.0, 0.5, 2.0});
    DynamicTensor<double> p(DynamicShape{Axis{"i", 4}}, {0.5, 1.0, 2.0, 4.0});
    for (std::size_t i = 0; i < a.size(); ++i) {
        CHECK(e.exp(a)[i]  == doctest::Approx(r.exp(a)[i]).epsilon(kTol));
        CHECK(e.relu(a)[i] == doctest::Approx(r.relu(a)[i]).epsilon(kTol));
        CHECK(e.neg(a)[i]  == doctest::Approx(r.neg(a)[i]).epsilon(kTol));
    }
    for (std::size_t i = 0; i < p.size(); ++i) {
        CHECK(e.log(p)[i] == doctest::Approx(r.log(p)[i]).epsilon(kTol));
    }
}

TEST_CASE("eigen vs reference: matvec contract agrees") {
    EigenBackend e;
    RefBackend r;
    DynamicTensor<double> W(DynamicShape{Axis{"i", 4}, Axis{"j", 3}},
                            {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12});
    DynamicTensor<double> x(DynamicShape{Axis{"j", 3}}, {2, 3, 4});
    auto plan = tensor::core::contract_plan(W.shape(), x.shape());
    auto ye = e.contract(W, x, plan);
    auto yr = r.contract(W, x, plan);
    REQUIRE(ye.size() == yr.size());
    for (std::size_t i = 0; i < ye.size(); ++i) {
        CHECK(ye[i] == doctest::Approx(yr[i]).epsilon(kTol));
    }
}

TEST_CASE("eigen vs reference: matmul contract agrees") {
    EigenBackend e;
    RefBackend r;
    DynamicTensor<double> A(DynamicShape{Axis{"i", 3}, Axis{"j", 2}}, {1, 2, 3, 4, 5, 6});
    DynamicTensor<double> B(DynamicShape{Axis{"j", 2}, Axis{"k", 4}},
                            {1, 0, 1, 0, 0, 1, 0, 1});
    auto plan = tensor::core::contract_plan(A.shape(), B.shape());
    auto Ce = e.contract(A, B, plan);
    auto Cr = r.contract(A, B, plan);
    REQUIRE(Ce.size() == Cr.size());
    for (std::size_t i = 0; i < Ce.size(); ++i) {
        CHECK(Ce[i] == doctest::Approx(Cr[i]).epsilon(kTol));
    }
}

TEST_CASE("eigen vs reference: reduce_sum agrees") {
    EigenBackend e;
    RefBackend r;
    DynamicTensor<double> a(DynamicShape{Axis{"i", 6}}, {1, 2, 3, 4, 5, 6});
    CHECK(e.reduce_sum(a) == doctest::Approx(r.reduce_sum(a)).epsilon(kTol));
}

TEST_CASE("eigen vs reference: broadcast + unbroadcast agree (delegated)") {
    EigenBackend e;
    RefBackend r;
    DynamicTensor<double> a(DynamicShape{Axis{"i", 2}}, {1, 2});
    DynamicTensor<double> b(DynamicShape{Axis{"j", 3}}, {10, 20, 30});
    auto plan = tensor::core::broadcast_shapes(a.shape(), b.shape());
    auto e_sum = e.broadcast_add(a, b, plan);
    auto r_sum = r.broadcast_add(a, b, plan);
    for (std::size_t i = 0; i < e_sum.size(); ++i) {
        CHECK(e_sum[i] == doctest::Approx(r_sum[i]).epsilon(kTol));
    }
    DynamicTensor<double> grad(plan.result, std::vector<double>(e_sum.size(), 1.0));
    auto e_ub = e.unbroadcast(grad, plan.a_source, a.shape());
    auto r_ub = r.unbroadcast(grad, plan.a_source, a.shape());
    REQUIRE(e_ub.size() == r_ub.size());
    for (std::size_t i = 0; i < e_ub.size(); ++i) {
        CHECK(e_ub[i] == doctest::Approx(r_ub[i]).epsilon(kTol));
    }
}

#endif  // TENSOR_HAS_EIGEN
