// SPDX-License-Identifier: MIT
#include <doctest/doctest.h>

#include <tensor/core/axis.hpp>
#include <tensor/core/concepts.hpp>
#include <tensor/core/shape.hpp>
#include <tensor/core/tensor.hpp>

using tensor::core::Axis;
using tensor::core::Shape;
using tensor::core::Tensor;

TEST_CASE("Tensor<double, 1> default-constructs to empty") {
    Tensor<double, 1> t;
    CHECK(t.size() == 0);
    CHECK(t.shape().total() == 0);
}

TEST_CASE("Tensor<double, 1> construction from shape and values matches the 2016 README") {
    Tensor<double, 1> a(Shape<1>{Axis{"i", 5}}, {1.0, 2.0, 3.0, 4.0, 5.0});
    CHECK(a.size() == 5);
    CHECK(a.shape()[0].label == "i");
    CHECK(a.shape()[0].extent == 5);
    CHECK(a(0) == 1.0);
    CHECK(a(4) == 5.0);
}

TEST_CASE("Tensor row-major flattening for rank 2") {
    Tensor<int, 2> m(Shape<2>{Axis{"i", 2}, Axis{"j", 3}}, {1, 2, 3, 4, 5, 6});
    CHECK(m(0, 0) == 1);
    CHECK(m(0, 1) == 2);
    CHECK(m(0, 2) == 3);
    CHECK(m(1, 0) == 4);
    CHECK(m(1, 1) == 5);
    CHECK(m(1, 2) == 6);
}

TEST_CASE("Tensor mismatched value count throws") {
    CHECK_THROWS_AS(
        (Tensor<double, 1>(Shape<1>{Axis{"i", 5}}, {1.0, 2.0, 3.0})),
        std::invalid_argument);
}

TEST_CASE("Tensor satisfies TensorLike concept") {
    static_assert(tensor::core::TensorLike<Tensor<double, 1>>);
    static_assert(tensor::core::TensorLike<Tensor<float, 3>>);
}
