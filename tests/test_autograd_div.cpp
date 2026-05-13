// SPDX-License-Identifier: MIT
//
// Forward + backward checks for the autograd-aware DynamicVariable
// element-wise division introduced in the Phase 6 autograd-surface-
// extensions PR. The closed-form gradients are:
//
//   d(a/b)/da =  1/b
//   d(a/b)/db = -a/b²
//
// Both with Einstein-style broadcast-aware unbroadcast on each side.

#include <doctest/doctest.h>

#include <tensor/autograd/autograd.hpp>
#include <tensor/autograd/broadcast_ops.hpp>
#include <tensor/autograd/dynamic_variable.hpp>
#include <tensor/core/axis.hpp>
#include <tensor/core/dynamic_shape.hpp>
#include <tensor/core/dynamic_tensor.hpp>

using namespace tensor::autograd;
using tensor::core::Axis;
using tensor::core::DynamicShape;
using tensor::core::DynamicTensor;

TEST_CASE("operator/ (DynamicVariable): forward elementwise") {
    Tape::current().clear();
    DynamicVariable<double> a(
        DynamicTensor<double>(DynamicShape{Axis{"i", 3}}, {6.0, 8.0, 10.0}));
    DynamicVariable<double> b(
        DynamicTensor<double>(DynamicShape{Axis{"i", 3}}, {2.0, 4.0, 5.0}));
    auto c = a / b;
    REQUIRE(c.value().size() == 3);
    CHECK(c.value()[0] == doctest::Approx(3.0));
    CHECK(c.value()[1] == doctest::Approx(2.0));
    CHECK(c.value()[2] == doctest::Approx(2.0));
}

TEST_CASE("operator/ (DynamicVariable): backward — same-shape, both require_grad") {
    Tape::current().clear();
    DynamicVariable<double> a(
        DynamicTensor<double>(DynamicShape{Axis{"i", 3}}, {6.0, 8.0, 10.0}),
        true);
    DynamicVariable<double> b(
        DynamicTensor<double>(DynamicShape{Axis{"i", 3}}, {2.0, 4.0, 5.0}),
        true);
    auto loss = sum_all(a / b);
    backward(loss);

    // dL/da = 1/b
    CHECK(a.grad()[0] == doctest::Approx(0.5));    // 1/2
    CHECK(a.grad()[1] == doctest::Approx(0.25));   // 1/4
    CHECK(a.grad()[2] == doctest::Approx(0.2));    // 1/5

    // dL/db = -a/b²
    CHECK(b.grad()[0] == doctest::Approx(-6.0 / 4));    // -6 / 2²
    CHECK(b.grad()[1] == doctest::Approx(-8.0 / 16));   // -8 / 4²
    CHECK(b.grad()[2] == doctest::Approx(-10.0 / 25));  // -10 / 5²
}

TEST_CASE("operator/ (DynamicVariable): backward with broadcast (scalar denominator)") {
    Tape::current().clear();
    DynamicVariable<double> a(
        DynamicTensor<double>(DynamicShape{Axis{"i", 3}}, {2.0, 4.0, 6.0}),
        true);
    DynamicVariable<double> b(
        DynamicTensor<double>(DynamicShape{}, {2.0}),  // rank-0 broadcast over i
        true);
    auto loss = sum_all(a / b);  // L = Σ a_i / 2 = (2+4+6)/2 = 6
    backward(loss);

    // dL/da_i = 1/b = 1/2
    CHECK(a.grad()[0] == doctest::Approx(0.5));
    CHECK(a.grad()[1] == doctest::Approx(0.5));
    CHECK(a.grad()[2] == doctest::Approx(0.5));

    // dL/db = Σ_i -a_i / b² = -(2+4+6)/4 = -3.0  (unbroadcast over i)
    CHECK(b.grad()[0] == doctest::Approx(-3.0));
}
