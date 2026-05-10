// SPDX-License-Identifier: MIT
#include <doctest/doctest.h>

#include <tensor/core/axis.hpp>
#include <tensor/core/concepts.hpp>
#include <tensor/core/dynamic_shape.hpp>
#include <tensor/core/dynamic_tensor.hpp>
#include <tensor/core/shape.hpp>
#include <tensor/core/tensor.hpp>

using tensor::core::Axis;
using tensor::core::DynamicShape;
using tensor::core::DynamicTensor;
using tensor::core::Shape;
using tensor::core::Tensor;

TEST_CASE("DynamicShape construction and basic queries") {
    DynamicShape s{Axis{"i", 5}, Axis{"j", 3}};
    CHECK(s.rank() == 2);
    CHECK(s.total() == 15);
    CHECK(s[0].label == "i");
    CHECK(s.has("j"));
    CHECK_FALSE(s.has("missing"));
    CHECK(s.index_of("i") == 0);
    CHECK(s.index_of("missing") == s.rank());
}

TEST_CASE("DynamicTensor construction from values") {
    DynamicTensor<int> t(DynamicShape{Axis{"i", 3}}, {7, 8, 9});
    CHECK(t.size() == 3);
    CHECK(t[0] == 7);
    CHECK(t[2] == 9);
    CHECK(t.shape().rank() == 1);
}

TEST_CASE("DynamicTensor mismatched value count throws") {
    CHECK_THROWS_AS(
        (DynamicTensor<int>(DynamicShape{Axis{"i", 3}}, {1, 2})),
        std::invalid_argument);
}

TEST_CASE("Tensor<T,N> implicitly converts to DynamicTensor<T>") {
    Tensor<double, 1> a(Shape<1>{Axis{"i", 4}}, {1.0, 2.0, 3.0, 4.0});
    DynamicTensor<double> d = a;
    CHECK(d.size() == 4);
    CHECK(d.shape().rank() == 1);
    CHECK(d.shape()[0].label == "i");
    CHECK(d[0] == 1.0);
    CHECK(d[3] == 4.0);
}

TEST_CASE("DynamicTensor::to_static round-trips back to Tensor<T,N>") {
    Tensor<int, 2> m(Shape<2>{Axis{"i", 2}, Axis{"j", 2}}, {1, 2, 3, 4});
    DynamicTensor<int> d = m;
    auto restored = d.to_static<2>();
    CHECK(restored == m);
}

TEST_CASE("DynamicTensor::to_static rank mismatch throws") {
    DynamicTensor<int> d(DynamicShape{Axis{"i", 2}}, {1, 2});
    CHECK_THROWS_AS(d.to_static<3>(), std::invalid_argument);
}

TEST_CASE("DynamicTensor satisfies TensorLike concept") {
    static_assert(tensor::core::TensorLike<DynamicTensor<double>>);
    static_assert(tensor::core::TensorLike<DynamicTensor<int>>);
}

TEST_CASE("DynamicShape satisfies ShapeLike concept") {
    static_assert(tensor::core::ShapeLike<DynamicShape>);
}
