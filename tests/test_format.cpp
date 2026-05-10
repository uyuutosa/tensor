// SPDX-License-Identifier: MIT
#include <doctest/doctest.h>

#include <sstream>
#include <string>

#include <tensor/core/axis.hpp>
#include <tensor/core/format.hpp>
#include <tensor/core/shape.hpp>
#include <tensor/core/tensor.hpp>

using tensor::core::Axis;
using tensor::core::Shape;
using tensor::core::Tensor;
using tensor::core::to_string;

TEST_CASE("operator<< writes the info banner with axis labels and rank") {
    Tensor<double, 1> a(Shape<1>{Axis{"i", 5}}, {1.0, 2.0, 3.0, 4.0, 5.0});
    auto out = to_string(a);
    CHECK(out.find("Total size  : 5") != std::string::npos);
    CHECK(out.find("Shape       : (i: 5)") != std::string::npos);
    CHECK(out.find("Num. of Dim.: 1") != std::string::npos);
}

TEST_CASE("operator<< rank-1 values block") {
    Tensor<int, 1> a(Shape<1>{Axis{"i", 3}}, {7, 8, 9});
    auto out = to_string(a);
    CHECK(out.find("[ 7, 8, 9 ]") != std::string::npos);
}

TEST_CASE("operator<< rank-2 values block has nested brackets") {
    Tensor<int, 2> m(Shape<2>{Axis{"i", 2}, Axis{"j", 3}}, {1, 2, 3, 4, 5, 6});
    auto out = to_string(m);
    CHECK(out.find("Shape       : (i: 2, j: 3)") != std::string::npos);
    CHECK(out.find("[ 1, 2, 3 ]") != std::string::npos);
    CHECK(out.find("[ 4, 5, 6 ]") != std::string::npos);
}
