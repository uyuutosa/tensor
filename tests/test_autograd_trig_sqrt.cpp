// SPDX-License-Identifier: MIT
//
// Forward + backward checks for the sin / cos / sqrt activations
// introduced in the Phase 6 autograd-surface-extensions PR.
// Mirror pattern of tests/test_autograd_activations.cpp.

#include <doctest/doctest.h>

#include <cmath>

#include <tensor/autograd/activations.hpp>
#include <tensor/autograd/dynamic_variable.hpp>
#include <tensor/autograd/tape.hpp>
#include <tensor/autograd/variable.hpp>
#include <tensor/core/axis.hpp>
#include <tensor/core/dynamic_shape.hpp>
#include <tensor/core/dynamic_tensor.hpp>
#include <tensor/core/shape.hpp>
#include <tensor/core/tensor.hpp>

using namespace tensor::autograd;
using tensor::core::Axis;
using tensor::core::DynamicShape;
using tensor::core::DynamicTensor;
using tensor::core::Shape;
using tensor::core::Tensor;

// ─── sin ──────────────────────────────────────────────────────────────────────

TEST_CASE("sin: forward + d/dx sin(x) = cos(x)") {
    Tape::current().clear();
    Variable<double, 1> x(
        Tensor<double, 1>(Shape<1>{Axis{"i", 3}}, {0.0, 0.5, 1.0}),
        /*requires_grad=*/true);
    auto y = sin(x);
    CHECK(y.value()[0] == doctest::Approx(std::sin(0.0)));
    CHECK(y.value()[1] == doctest::Approx(std::sin(0.5)));
    CHECK(y.value()[2] == doctest::Approx(std::sin(1.0)));

    auto loss = sum_all(y);
    backward(loss);
    CHECK(x.grad()[0] == doctest::Approx(std::cos(0.0)));
    CHECK(x.grad()[1] == doctest::Approx(std::cos(0.5)));
    CHECK(x.grad()[2] == doctest::Approx(std::cos(1.0)));
}

TEST_CASE("sin (DynamicVariable): d/dx sin(x) = cos(x)") {
    Tape::current().clear();
    DynamicVariable<double> x(
        DynamicTensor<double>(DynamicShape{Axis{"i", 3}}, {0.1, 0.7, 1.3}),
        true);
    auto loss = sum_all(sin(x));
    backward(loss);
    CHECK(x.grad()[0] == doctest::Approx(std::cos(0.1)));
    CHECK(x.grad()[1] == doctest::Approx(std::cos(0.7)));
    CHECK(x.grad()[2] == doctest::Approx(std::cos(1.3)));
}

// ─── cos ──────────────────────────────────────────────────────────────────────

TEST_CASE("cos: forward + d/dx cos(x) = -sin(x)") {
    Tape::current().clear();
    Variable<double, 1> x(
        Tensor<double, 1>(Shape<1>{Axis{"i", 3}}, {0.0, 0.5, 1.0}),
        true);
    auto y = cos(x);
    CHECK(y.value()[0] == doctest::Approx(std::cos(0.0)));
    CHECK(y.value()[1] == doctest::Approx(std::cos(0.5)));
    CHECK(y.value()[2] == doctest::Approx(std::cos(1.0)));

    auto loss = sum_all(y);
    backward(loss);
    CHECK(x.grad()[0] == doctest::Approx(-std::sin(0.0)));
    CHECK(x.grad()[1] == doctest::Approx(-std::sin(0.5)));
    CHECK(x.grad()[2] == doctest::Approx(-std::sin(1.0)));
}

TEST_CASE("cos (DynamicVariable): d/dx cos(x) = -sin(x)") {
    Tape::current().clear();
    DynamicVariable<double> x(
        DynamicTensor<double>(DynamicShape{Axis{"i", 2}}, {0.3, 1.1}),
        true);
    auto loss = sum_all(cos(x));
    backward(loss);
    CHECK(x.grad()[0] == doctest::Approx(-std::sin(0.3)));
    CHECK(x.grad()[1] == doctest::Approx(-std::sin(1.1)));
}

// ─── sqrt ─────────────────────────────────────────────────────────────────────

TEST_CASE("sqrt: forward + d/dx sqrt(x) = 1 / (2 sqrt(x))") {
    Tape::current().clear();
    Variable<double, 1> x(
        Tensor<double, 1>(Shape<1>{Axis{"i", 3}}, {1.0, 4.0, 9.0}),
        true);
    auto y = sqrt(x);
    CHECK(y.value()[0] == doctest::Approx(1.0));
    CHECK(y.value()[1] == doctest::Approx(2.0));
    CHECK(y.value()[2] == doctest::Approx(3.0));

    auto loss = sum_all(y);
    backward(loss);
    CHECK(x.grad()[0] == doctest::Approx(0.5));      // 1 / (2 * 1)
    CHECK(x.grad()[1] == doctest::Approx(0.25));     // 1 / (2 * 2)
    CHECK(x.grad()[2] == doctest::Approx(1.0 / 6));  // 1 / (2 * 3)
}

TEST_CASE("sqrt (DynamicVariable): d/dx sqrt(x) = 1 / (2 sqrt(x))") {
    Tape::current().clear();
    DynamicVariable<double> x(
        DynamicTensor<double>(DynamicShape{Axis{"i", 2}}, {1.0, 9.0}),
        true);
    auto loss = sum_all(sqrt(x));
    backward(loss);
    CHECK(x.grad()[0] == doctest::Approx(0.5));
    CHECK(x.grad()[1] == doctest::Approx(1.0 / 6));
}

// ─── Rodrigues smoke: sin/cos composed reproduce a rotation ──────────────────
//
// Sanity check that downstream consumers (perspective bundle adjustment
// in the Python tutorial) can compose sin/cos on a scalar Variable to
// build a 2D rotation. Property: a rotation by 2π returns to identity.

TEST_CASE("sin/cos: rotation by 2π returns to identity (smoke)") {
    Tape::current().clear();
    Variable<double, 0> theta(
        Tensor<double, 0>(Shape<0>{}, {0.0}), /*requires_grad=*/false);
    Variable<double, 0> two_pi(
        Tensor<double, 0>(Shape<0>{}, {2.0 * 3.14159265358979323846}), false);
    auto theta_full = theta + two_pi;  // theta + 2π
    auto c = cos(theta_full);
    auto s = sin(theta_full);
    CHECK(c.value()[0] == doctest::Approx(1.0).epsilon(1e-12));
    CHECK(s.value()[0] == doctest::Approx(0.0).epsilon(1e-12));
}
