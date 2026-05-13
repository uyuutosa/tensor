// SPDX-License-Identifier: MIT
//
// Autograd-aware single-axis reduction test. Verifies forward + backward
// on a 3×4 tensor reduced along "i" and along "j".

#include <doctest/doctest.h>

#include <tensor/autograd/autograd.hpp>
#include <tensor/autograd/dynamic_variable.hpp>
#include <tensor/autograd/reduce_ops.hpp>
#include <tensor/core/axis.hpp>
#include <tensor/core/dynamic_shape.hpp>
#include <tensor/core/dynamic_tensor.hpp>

using namespace tensor::autograd;
using tensor::core::Axis;
using tensor::core::DynamicShape;
using tensor::core::DynamicTensor;

TEST_CASE("reduce_along_label: forward sum along 'i' drops the axis") {
    Tape::current().clear();
    // 2x3 tensor labelled (i, j):
    //   1 2 3
    //   4 5 6
    DynamicTensor<double> t(
        DynamicShape{Axis{"i", 2}, Axis{"j", 3}},
        {1.0, 2.0, 3.0, 4.0, 5.0, 6.0});
    DynamicVariable<double> x(t);
    auto out = reduce_along_label(x, std::string{"i"});
    REQUIRE(out.value().shape().rank() == 1);
    REQUIRE(out.value().size() == 3);
    CHECK(out.value()[0] == doctest::Approx(5.0));   // 1 + 4
    CHECK(out.value()[1] == doctest::Approx(7.0));   // 2 + 5
    CHECK(out.value()[2] == doctest::Approx(9.0));   // 3 + 6
}

TEST_CASE("reduce_along_label: backward — gradient broadcasts back over reduced axis") {
    Tape::current().clear();
    DynamicVariable<double> x(
        DynamicTensor<double>(DynamicShape{Axis{"i", 2}, Axis{"j", 3}},
                              {1.0, 2.0, 3.0, 4.0, 5.0, 6.0}),
        /*requires_grad=*/true);
    // out_j = Σ_i x_{ij}; loss = Σ_j out_j → dL/dx_{ij} = 1 everywhere.
    auto loss = sum_all(reduce_along_label(x, std::string{"i"}));
    backward(loss);
    for (std::size_t k = 0; k < 6; ++k) {
        CHECK(x.grad()[k] == doctest::Approx(1.0));
    }
}

TEST_CASE("reduce_along_label: chained with weighted loss yields broadcasted weights") {
    Tape::current().clear();
    // out_j = Σ_i x_{ij}, then weight by w_j and sum:
    // loss = Σ_j w_j (Σ_i x_{ij})  →  dL/dx_{ij} = w_j (broadcast over i).
    DynamicVariable<double> x(
        DynamicTensor<double>(DynamicShape{Axis{"i", 2}, Axis{"j", 3}},
                              {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}),
        true);
    DynamicVariable<double> w(
        DynamicTensor<double>(DynamicShape{Axis{"j", 3}}, {10.0, 20.0, 30.0}));
    auto out = reduce_along_label(x, std::string{"i"});  // (j,)
    auto weighted = out * w;                              // pairwise on j
    auto loss = sum_all(weighted);
    backward(loss);
    // x has rank-2 (i, j) with values (1,1,1, 1,1,1). dL/dx_{ij} = w_j.
    CHECK(x.grad()[0] == doctest::Approx(10.0));  // x_{0,0}
    CHECK(x.grad()[1] == doctest::Approx(20.0));  // x_{0,1}
    CHECK(x.grad()[2] == doctest::Approx(30.0));  // x_{0,2}
    CHECK(x.grad()[3] == doctest::Approx(10.0));  // x_{1,0}
    CHECK(x.grad()[4] == doctest::Approx(20.0));  // x_{1,1}
    CHECK(x.grad()[5] == doctest::Approx(30.0));  // x_{1,2}
}
