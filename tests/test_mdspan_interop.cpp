// SPDX-License-Identifier: MIT
#include <doctest/doctest.h>

#include <tensor/core/axis.hpp>
#include <tensor/core/mdspan_interop.hpp>
#include <tensor/core/shape.hpp>
#include <tensor/core/tensor.hpp>

using tensor::core::Axis;
using tensor::core::Shape;
using tensor::core::Tensor;

TEST_CASE("mdview returns a non-owning view with matching extents") {
    Tensor<double, 1> a(Shape<1>{Axis{"i", 4}}, {1.0, 2.0, 3.0, 4.0});
    auto view = tensor::core::mdview(a);
    CHECK(view.extent(0) == 4);
    CHECK(view(0) == 1.0);
    CHECK(view(3) == 4.0);

    // mutate through the view, observe through the tensor
    view(2) = 99.0;
    CHECK(a(2) == 99.0);
}

TEST_CASE("mdview rank-2 round-trip preserves values") {
    Tensor<int, 2> m(Shape<2>{Axis{"i", 2}, Axis{"j", 3}}, {1, 2, 3, 4, 5, 6});
    auto view = tensor::core::mdview(m);
    REQUIRE(view.extent(0) == 2);
    REQUIRE(view.extent(1) == 3);
    CHECK(view(0, 0) == 1);
    CHECK(view(0, 2) == 3);
    CHECK(view(1, 0) == 4);
    CHECK(view(1, 2) == 6);

    auto rebuilt =
        tensor::core::from_mdspan(view, Shape<2>{Axis{"i", 2}, Axis{"j", 3}});
    CHECK(rebuilt == m);
}
