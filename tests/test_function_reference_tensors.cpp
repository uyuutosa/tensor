// SPDX-License-Identifier: MIT
//
// Reproduces the function-tensor and reference-tensor sections of the
// 2016 README on the new API.

#include <doctest/doctest.h>

#include <tensor/core/axis.hpp>
#include <tensor/core/function_tensor.hpp>
#include <tensor/core/reference_tensor.hpp>
#include <tensor/core/shape.hpp>
#include <tensor/core/tensor.hpp>

using tensor::core::Axis;
using tensor::core::FunctionTensor;
using tensor::core::ReferenceTensor;
using tensor::core::Shape;
using tensor::core::Tensor;

TEST_CASE("FunctionTensor reproduces 2016 README a * f = (1, 4, 7, 10, 13)") {
    Tensor<double, 1> a(Shape<1>{Axis{"i", 5}}, {1, 2, 3, 4, 5});
    auto f = FunctionTensor(Axis{"i", 5},
                            [](std::size_t i, double v) {
                                return v + 2.0 * static_cast<double>(i);
                            });
    auto out = a * f;
    REQUIRE(out.size() == 5);
    CHECK(out(0) == doctest::Approx(1.0));
    CHECK(out(1) == doctest::Approx(4.0));
    CHECK(out(2) == doctest::Approx(7.0));
    CHECK(out(3) == doctest::Approx(10.0));
    CHECK(out(4) == doctest::Approx(13.0));
}

TEST_CASE("FunctionTensor commutes (f * a == a * f)") {
    Tensor<int, 1> a(Shape<1>{Axis{"i", 3}}, {10, 20, 30});
    auto f = FunctionTensor(Axis{"i", 3},
                            [](std::size_t i, int v) -> int {
                                return v + static_cast<int>(i);
                            });
    auto left = f * a;
    auto right = a * f;
    CHECK(left == right);
}

TEST_CASE("FunctionTensor rejects axis label mismatch") {
    Tensor<int, 1> a(Shape<1>{Axis{"i", 3}}, {1, 2, 3});
    auto f = FunctionTensor(Axis{"j", 3}, [](std::size_t, int v) { return v; });
    CHECK_THROWS_AS(a * f, std::invalid_argument);
}

TEST_CASE("FunctionTensor rejects axis extent mismatch") {
    Tensor<int, 1> a(Shape<1>{Axis{"i", 3}}, {1, 2, 3});
    auto f = FunctionTensor(Axis{"i", 4}, [](std::size_t, int v) { return v; });
    CHECK_THROWS_AS(a * f, std::invalid_argument);
}

TEST_CASE("ReferenceTensor reproduces 2016 README r * 3 = (9, 27, 81, 243, 729)") {
    auto r = ReferenceTensor<double>(3.0, Axis{"i", 5});
    auto out = r * 3.0;
    REQUIRE(out.size() == 5);
    CHECK(out(0) == doctest::Approx(9.0));
    CHECK(out(1) == doctest::Approx(27.0));
    CHECK(out(2) == doctest::Approx(81.0));
    CHECK(out(3) == doctest::Approx(243.0));
    CHECK(out(4) == doctest::Approx(729.0));
}

TEST_CASE("ReferenceTensor scalar * r matches r * scalar") {
    auto r = ReferenceTensor<int>(2, Axis{"i", 4});
    auto a = r * 2;
    auto b = 2 * r;
    CHECK(a == b);
    REQUIRE(a.size() == 4);
    CHECK(a(0) == 4);
    CHECK(a(1) == 8);
    CHECK(a(2) == 16);
    CHECK(a(3) == 32);
}

TEST_CASE("ReferenceTensor preserves the named axis") {
    auto r = ReferenceTensor<int>(1, Axis{"k", 3});
    auto out = r * 5;
    CHECK(out.shape()[0].label == "k");
    CHECK(out.shape()[0].extent == 3);
}
