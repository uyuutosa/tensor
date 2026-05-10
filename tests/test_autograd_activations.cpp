// SPDX-License-Identifier: MIT
#include <doctest/doctest.h>

#include <cmath>

#include <tensor/autograd/autograd.hpp>
#include <tensor/core/axis.hpp>
#include <tensor/core/shape.hpp>
#include <tensor/core/tensor.hpp>

using namespace tensor::autograd;
using tensor::core::Axis;
using tensor::core::Shape;
using tensor::core::Tensor;

TEST_CASE("exp forward + backward: d/dx exp(x) = exp(x)") {
    Tape::current().clear();
    Variable<double, 1> x(Tensor<double, 1>(Shape<1>{Axis{"i", 3}}, {0.0, 1.0, 2.0}), true);
    auto y = exp(x);
    CHECK(y.value()[0] == doctest::Approx(1.0));
    CHECK(y.value()[1] == doctest::Approx(std::exp(1.0)));
    CHECK(y.value()[2] == doctest::Approx(std::exp(2.0)));

    auto loss = sum_all(y);
    backward(loss);
    CHECK(x.grad()[0] == doctest::Approx(1.0));
    CHECK(x.grad()[1] == doctest::Approx(std::exp(1.0)));
    CHECK(x.grad()[2] == doctest::Approx(std::exp(2.0)));
}

TEST_CASE("log forward + backward: d/dx log(x) = 1/x") {
    Tape::current().clear();
    Variable<double, 1> x(Tensor<double, 1>(Shape<1>{Axis{"i", 3}}, {1.0, 2.0, 4.0}), true);
    auto loss = sum_all(log(x));
    backward(loss);
    CHECK(x.grad()[0] == doctest::Approx(1.0));
    CHECK(x.grad()[1] == doctest::Approx(0.5));
    CHECK(x.grad()[2] == doctest::Approx(0.25));
}

TEST_CASE("relu forward + backward: d/dx relu(x) = I(x > 0)") {
    Tape::current().clear();
    Variable<double, 1> x(Tensor<double, 1>(Shape<1>{Axis{"i", 4}}, {-2.0, -0.1, 0.5, 2.0}), true);
    auto y = relu(x);
    CHECK(y.value()[0] == doctest::Approx(0.0));
    CHECK(y.value()[1] == doctest::Approx(0.0));
    CHECK(y.value()[2] == doctest::Approx(0.5));
    CHECK(y.value()[3] == doctest::Approx(2.0));

    auto loss = sum_all(y);
    backward(loss);
    CHECK(x.grad()[0] == doctest::Approx(0.0));
    CHECK(x.grad()[1] == doctest::Approx(0.0));
    CHECK(x.grad()[2] == doctest::Approx(1.0));
    CHECK(x.grad()[3] == doctest::Approx(1.0));
}

TEST_CASE("unary minus: d/dx (-x) = -1") {
    Tape::current().clear();
    Variable<double, 1> x(Tensor<double, 1>(Shape<1>{Axis{"i", 3}}, {1.0, 2.0, 3.0}), true);
    auto loss = sum_all(-x);
    backward(loss);
    CHECK(loss.value()[0] == doctest::Approx(-6.0));
    CHECK(x.grad()[0] == doctest::Approx(-1.0));
    CHECK(x.grad()[1] == doctest::Approx(-1.0));
    CHECK(x.grad()[2] == doctest::Approx(-1.0));
}

TEST_CASE("gradient_check parity: sum(exp(x))") {
    Variable<double, 1> x(Tensor<double, 1>(Shape<1>{Axis{"i", 3}}, {0.1, 0.5, 1.0}), true);
    CHECK(gradient_check([](auto const& v) { return sum_all(exp(v)); }, x));
}

TEST_CASE("gradient_check parity: sum(log(x))") {
    Variable<double, 1> x(Tensor<double, 1>(Shape<1>{Axis{"i", 3}}, {1.0, 2.0, 3.0}), true);
    CHECK(gradient_check([](auto const& v) { return sum_all(log(v)); }, x));
}

TEST_CASE("gradient_check parity: sum(relu(x))") {
    // Use only points away from 0 (subgradient is conventionally 0 at x = 0).
    Variable<double, 1> x(Tensor<double, 1>(Shape<1>{Axis{"i", 4}}, {-1.0, 0.5, 1.5, -0.5}), true);
    CHECK(gradient_check([](auto const& v) { return sum_all(relu(v)); }, x));
}

TEST_CASE("gradient_check parity: composite sum(exp(x) * x)") {
    Variable<double, 1> x(Tensor<double, 1>(Shape<1>{Axis{"i", 3}}, {0.2, 0.7, 1.1}), true);
    CHECK(gradient_check(
        [](auto const& v) { return sum_all(exp(v) * v); }, x));
}
