// SPDX-License-Identifier: MIT
//
// Reproduces the four-arithmetic-operations example block from the 2016
// README on the new API. This file is the M3 exit-criterion artefact.

#include <doctest/doctest.h>

#include <tensor/core/axis.hpp>
#include <tensor/core/dynamic_tensor.hpp>
#include <tensor/core/ops.hpp>
#include <tensor/core/shape.hpp>
#include <tensor/core/tensor.hpp>

using tensor::core::Axis;
using tensor::core::DynamicTensor;
using tensor::core::Shape;
using tensor::core::Tensor;

TEST_CASE("README a + b: distinct labels produce a 5x5 sum table") {
    Tensor<double, 1> a(Shape<1>{Axis{"i", 5}}, {1, 2, 3, 4, 5});
    Tensor<double, 1> b(Shape<1>{Axis{"j", 5}}, {1, 2, 3, 4, 5});
    auto c = a + b;
    REQUIRE(c.shape().rank() == 2);
    CHECK(c.shape()[0].label == "i");
    CHECK(c.shape()[1].label == "j");
    REQUIRE(c.size() == 25);
    // c(i, j) = a_i + b_j
    CHECK(c[0 * 5 + 0] == 2);   // 1 + 1
    CHECK(c[0 * 5 + 4] == 6);   // 1 + 5
    CHECK(c[4 * 5 + 0] == 6);   // 5 + 1
    CHECK(c[4 * 5 + 4] == 10);  // 5 + 5
}

TEST_CASE("README a - b: distinct labels produce a 5x5 difference table") {
    Tensor<double, 1> a(Shape<1>{Axis{"i", 5}}, {1, 2, 3, 4, 5});
    Tensor<double, 1> b(Shape<1>{Axis{"j", 5}}, {1, 2, 3, 4, 5});
    auto c = a - b;
    REQUIRE(c.size() == 25);
    CHECK(c[0 * 5 + 0] == 0);   // 1 - 1
    CHECK(c[4 * 5 + 0] == 4);   // 5 - 1
    CHECK(c[0 * 5 + 4] == -4);  // 1 - 5
    CHECK(c[4 * 5 + 4] == 0);   // 5 - 5
}

TEST_CASE("README a * b: distinct labels produce a 5x5 product table") {
    Tensor<double, 1> a(Shape<1>{Axis{"i", 5}}, {1, 2, 3, 4, 5});
    Tensor<double, 1> b(Shape<1>{Axis{"j", 5}}, {1, 2, 3, 4, 5});
    auto c = a * b;
    REQUIRE(c.size() == 25);
    CHECK(c[2 * 5 + 2] == 9);    // 3 * 3
    CHECK(c[4 * 5 + 4] == 25);   // 5 * 5
    CHECK(c[1 * 5 + 3] == 8);    // 2 * 4
}

TEST_CASE("README a / b: distinct labels produce a 5x5 ratio table") {
    Tensor<double, 1> a(Shape<1>{Axis{"i", 5}}, {1, 2, 3, 4, 5});
    Tensor<double, 1> b(Shape<1>{Axis{"j", 5}}, {1, 2, 3, 4, 5});
    auto c = a / b;
    REQUIRE(c.size() == 25);
    CHECK(c[0 * 5 + 0] == doctest::Approx(1.0));
    CHECK(c[2 * 5 + 0] == doctest::Approx(3.0));        // 3 / 1
    CHECK(c[0 * 5 + 4] == doctest::Approx(0.2));        // 1 / 5
    CHECK(c[4 * 5 + 4] == doctest::Approx(1.0));        // 5 / 5
    CHECK(c[1 * 5 + 2] == doctest::Approx(2.0 / 3.0));  // 2 / 3
}

TEST_CASE("Same-label axes contract pairwise (rank preserved, element-wise)") {
    Tensor<double, 1> a(Shape<1>{Axis{"i", 5}}, {1, 2, 3, 4, 5});
    Tensor<double, 1> c(Shape<1>{Axis{"i", 5}}, {10, 20, 30, 40, 50});
    auto sum = a + c;
    REQUIRE(sum.shape().rank() == 1);
    CHECK(sum.shape()[0].label == "i");
    CHECK(sum.size() == 5);
    CHECK(sum[0] == 11);
    CHECK(sum[4] == 55);
}

TEST_CASE("Mixed shared/distinct labels: rank 2 broadcast with one shared dim") {
    // a(i, j) (i: 2, j: 3), b(j, k) (j: 3, k: 2) → result (i, j, k) (2, 3, 2)
    Tensor<int, 2> a(Shape<2>{Axis{"i", 2}, Axis{"j", 3}}, {1, 2, 3, 4, 5, 6});
    Tensor<int, 2> b(Shape<2>{Axis{"j", 3}, Axis{"k", 2}}, {1, 2, 3, 4, 5, 6});
    auto r = a + b;
    REQUIRE(r.shape().rank() == 3);
    CHECK(r.shape()[0].label == "i");
    CHECK(r.shape()[1].label == "j");
    CHECK(r.shape()[2].label == "k");
    CHECK(r.size() == 12);
    // Spot-check: r(0, 0, 0) = a(0, 0) + b(0, 0) = 1 + 1 = 2
    CHECK(r[0] == 2);
    // r(1, 2, 1) = a(1, 2) + b(2, 1) = 6 + 6 = 12
    // index in result row-major: 1 * (3 * 2) + 2 * 2 + 1 = 11
    CHECK(r[11] == 12);
}

TEST_CASE("DynamicTensor + DynamicTensor uses the same kernel") {
    DynamicTensor<double> a(tensor::core::DynamicShape{Axis{"i", 3}},
                            {1.0, 2.0, 3.0});
    DynamicTensor<double> b(tensor::core::DynamicShape{Axis{"j", 2}},
                            {10.0, 20.0});
    auto c = a + b;
    REQUIRE(c.shape().rank() == 2);
    CHECK(c.size() == 6);
    // c(0, 0) = 1 + 10 = 11
    CHECK(c[0] == 11.0);
    // c(2, 1) = 3 + 20 = 23
    CHECK(c[5] == 23.0);
}
