// SPDX-License-Identifier: MIT
//
// tensor::core::contract — Einstein-style contraction (pure forward).

#include <doctest/doctest.h>

#include <tensor/core/axis.hpp>
#include <tensor/core/contract.hpp>
#include <tensor/core/dynamic_shape.hpp>
#include <tensor/core/dynamic_tensor.hpp>

using tensor::core::Axis;
using tensor::core::contract;
using tensor::core::DynamicShape;
using tensor::core::DynamicTensor;

TEST_CASE("Matrix-vector contract: y_i = Σ_j W_{ij} x_j") {
    DynamicTensor<double> W(DynamicShape{Axis{"i", 2}, Axis{"j", 3}}, {1, 2, 3, 4, 5, 6});
    DynamicTensor<double> x(DynamicShape{Axis{"j", 3}}, {10, 20, 30});
    auto y = contract(W, x);
    REQUIRE(y.shape().rank() == 1);
    REQUIRE(y.size() == 2);
    CHECK(y[0] == doctest::Approx(140.0));  // 1·10 + 2·20 + 3·30
    CHECK(y[1] == doctest::Approx(320.0));  // 4·10 + 5·20 + 6·30
}

TEST_CASE("Matrix-matrix contract: C_{ik} = Σ_j A_{ij} B_{jk}") {
    DynamicTensor<double> A(DynamicShape{Axis{"i", 2}, Axis{"j", 3}}, {1, 2, 3, 4, 5, 6});
    DynamicTensor<double> B(DynamicShape{Axis{"j", 3}, Axis{"k", 2}}, {1, 0, 0, 1, 1, 1});
    auto C = contract(A, B);
    REQUIRE(C.shape().rank() == 2);
    REQUIRE(C.size() == 4);
    CHECK(C[0] == doctest::Approx(4.0));   // 1·1+2·0+3·1
    CHECK(C[1] == doctest::Approx(5.0));   // 1·0+2·1+3·1
    CHECK(C[2] == doctest::Approx(10.0));  // 4·1+5·0+6·1
    CHECK(C[3] == doctest::Approx(11.0));  // 4·0+5·1+6·1
}

TEST_CASE("No-shared-axis contract is an outer product") {
    DynamicTensor<double> a(DynamicShape{Axis{"i", 2}}, {1, 2});
    DynamicTensor<double> b(DynamicShape{Axis{"j", 3}}, {10, 20, 30});
    auto o = contract(a, b);
    REQUIRE(o.shape().rank() == 2);
    CHECK(o[0] == doctest::Approx(10));   // 1·10
    CHECK(o[1] == doctest::Approx(20));
    CHECK(o[5] == doctest::Approx(60));   // 2·30
}

TEST_CASE("Inner product (all axes shared) contracts to a scalar") {
    DynamicTensor<double> a(DynamicShape{Axis{"i", 3}}, {1, 2, 3});
    DynamicTensor<double> b(DynamicShape{Axis{"i", 3}}, {4, 5, 6});
    auto s = contract(a, b);
    REQUIRE(s.shape().rank() == 0);
    REQUIRE(s.size() == 1);
    CHECK(s[0] == doctest::Approx(32.0));  // 1·4 + 2·5 + 3·6
}

TEST_CASE("Extent mismatch on shared label throws") {
    DynamicTensor<double> a(DynamicShape{Axis{"i", 3}}, {1, 2, 3});
    DynamicTensor<double> b(DynamicShape{Axis{"i", 4}}, {1, 2, 3, 4});
    CHECK_THROWS_AS(contract(a, b), std::invalid_argument);
}
