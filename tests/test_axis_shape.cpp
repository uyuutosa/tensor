// SPDX-License-Identifier: MIT
#include <doctest/doctest.h>

#include <tensor/core/axis.hpp>
#include <tensor/core/concepts.hpp>
#include <tensor/core/shape.hpp>

using tensor::core::Axis;
using tensor::core::Shape;

TEST_CASE("Axis carries label and extent") {
    Axis a{"i", 5};
    CHECK(a.label == "i");
    CHECK(a.extent == 5);
}

TEST_CASE("Axis equality and ordering") {
    CHECK(Axis{"i", 5} == Axis{"i", 5});
    CHECK(Axis{"i", 5} != Axis{"i", 4});
    CHECK(Axis{"i", 5} != Axis{"j", 5});
}

TEST_CASE("Axis satisfies AxisLike concept") {
    static_assert(tensor::core::AxisLike<Axis>);
}

TEST_CASE("Shape<2> from initializer_list") {
    Shape<2> s{Axis{"i", 5}, Axis{"j", 3}};
    CHECK(s.size() == 2);
    CHECK(s[0].label == "i");
    CHECK(s[0].extent == 5);
    CHECK(s[1].label == "j");
    CHECK(s[1].extent == 3);
    CHECK(s.total() == 15);
}

TEST_CASE("Shape::index_of and has") {
    Shape<3> s{Axis{"i", 4}, Axis{"j", 2}, Axis{"k", 7}};
    CHECK(s.index_of("i") == 0);
    CHECK(s.index_of("j") == 1);
    CHECK(s.index_of("k") == 2);
    CHECK(s.index_of("missing") == 3);
    CHECK(s.has("j"));
    CHECK_FALSE(s.has("x"));
}

TEST_CASE("Shape size mismatch in initializer_list throws") {
    CHECK_THROWS_AS((Shape<2>{Axis{"i", 5}}), std::invalid_argument);
}

TEST_CASE("Shape satisfies ShapeLike concept") {
    static_assert(tensor::core::ShapeLike<Shape<1>>);
    static_assert(tensor::core::ShapeLike<Shape<3>>);
}
