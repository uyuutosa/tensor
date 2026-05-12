// SPDX-License-Identifier: MIT
//
// Phase 2.5 P2.5.M2 — reference Backend satisfies the KernelBackend port
// (ADR-0011) and produces the same outputs as the existing core utilities
// it delegates to.

#include <doctest/doctest.h>

#include <cmath>

#include <tensor/core/axis.hpp>
#include <tensor/core/backend/reference.hpp>
#include <tensor/core/broadcast.hpp>
#include <tensor/core/concepts.hpp>
#include <tensor/core/contract.hpp>
#include <tensor/core/dynamic_shape.hpp>
#include <tensor/core/dynamic_tensor.hpp>

using tensor::core::Axis;
using tensor::core::DynamicShape;
using tensor::core::DynamicTensor;
using tensor::core::backend::reference::Backend;

TEST_CASE("reference::Backend satisfies KernelBackend") {
    static_assert(tensor::core::KernelBackend<Backend>);
}

TEST_CASE("reference::Backend element-wise add/sub/mul/div") {
    Backend b;
    DynamicTensor<double> a(DynamicShape{Axis{"i", 4}}, {1, 2, 4, 8});
    DynamicTensor<double> c(DynamicShape{Axis{"i", 4}}, {2, 2, 2, 2});
    CHECK(b.add(a, c)[0] == doctest::Approx(3));
    CHECK(b.sub(a, c)[3] == doctest::Approx(6));
    CHECK(b.mul(a, c)[2] == doctest::Approx(8));
    CHECK(b.div(a, c)[0] == doctest::Approx(0.5));
}

TEST_CASE("reference::Backend element-wise unary exp/log/relu/neg") {
    Backend b;
    DynamicTensor<double> a(DynamicShape{Axis{"i", 4}}, {-1.0, 0.0, 0.5, 2.0});
    auto e = b.exp(a);
    CHECK(e[1] == doctest::Approx(1.0));
    CHECK(e[3] == doctest::Approx(std::exp(2.0)));

    DynamicTensor<double> p(DynamicShape{Axis{"i", 3}}, {1, 2, 4});
    auto l = b.log(p);
    CHECK(l[0] == doctest::Approx(0.0));
    CHECK(l[1] == doctest::Approx(std::log(2.0)));

    auto r = b.relu(a);
    CHECK(r[0] == doctest::Approx(0));
    CHECK(r[3] == doctest::Approx(2.0));

    auto n = b.neg(a);
    CHECK(n[0] == doctest::Approx(1.0));
    CHECK(n[3] == doctest::Approx(-2.0));
}

TEST_CASE("reference::Backend broadcast_add/sub/mul") {
    Backend b;
    DynamicTensor<double> a(DynamicShape{Axis{"i", 2}}, {1, 2});
    DynamicTensor<double> bb(DynamicShape{Axis{"j", 3}}, {10, 20, 30});
    auto plan = tensor::core::broadcast_shapes(a.shape(), bb.shape());

    auto sum = b.broadcast_add(a, bb, plan);
    REQUIRE(sum.size() == 6);
    CHECK(sum[0] == doctest::Approx(11));
    CHECK(sum[5] == doctest::Approx(32));

    auto diff = b.broadcast_sub(a, bb, plan);
    CHECK(diff[0] == doctest::Approx(-9));
    CHECK(diff[5] == doctest::Approx(-28));

    auto prod = b.broadcast_mul(a, bb, plan);
    CHECK(prod[0] == doctest::Approx(10));
    CHECK(prod[5] == doctest::Approx(60));
}

TEST_CASE("reference::Backend contract — matvec and matmul") {
    Backend b;
    DynamicTensor<double> W(DynamicShape{Axis{"i", 2}, Axis{"j", 3}}, {1, 2, 3, 4, 5, 6});
    DynamicTensor<double> x(DynamicShape{Axis{"j", 3}}, {10, 20, 30});
    auto plan = tensor::core::contract_plan(W.shape(), x.shape());
    auto y = b.contract(W, x, plan);
    REQUIRE(y.size() == 2);
    CHECK(y[0] == doctest::Approx(140));
    CHECK(y[1] == doctest::Approx(320));
}

TEST_CASE("reference::Backend reduce_sum") {
    Backend b;
    DynamicTensor<double> a(DynamicShape{Axis{"i", 5}}, {1, 2, 3, 4, 5});
    CHECK(b.reduce_sum(a) == doctest::Approx(15.0));
}

TEST_CASE("reference::Backend unbroadcast — reduce over collapsed axes") {
    Backend b;
    DynamicShape ab_shape{Axis{"i", 2}, Axis{"j", 3}};
    auto plan = tensor::core::broadcast_shapes(
        DynamicShape{Axis{"i", 2}}, DynamicShape{Axis{"j", 3}});
    DynamicTensor<double> ones(plan.result, std::vector<double>(6, 1.0));

    auto ub_a = b.unbroadcast(ones, plan.a_source, DynamicShape{Axis{"i", 2}});
    REQUIRE(ub_a.size() == 2);
    // Σ_j 1 = 3 per i
    CHECK(ub_a[0] == doctest::Approx(3.0));
    CHECK(ub_a[1] == doctest::Approx(3.0));

    auto ub_b = b.unbroadcast(ones, plan.b_source, DynamicShape{Axis{"j", 3}});
    REQUIRE(ub_b.size() == 3);
    // Σ_i 1 = 2 per j
    for (std::size_t j = 0; j < 3; ++j) CHECK(ub_b[j] == doctest::Approx(2.0));
}
