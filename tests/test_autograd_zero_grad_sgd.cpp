// SPDX-License-Identifier: MIT
//
// Phase 1.5 — Variable::zero_grad() + sgd_update() training-loop ergonomics.

#include <doctest/doctest.h>

#include <tensor/autograd/autograd.hpp>
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

TEST_CASE("Without zero_grad, backward accumulates into the same Variable") {
    Tape::current().clear();
    Variable<double, 1> x(Tensor<double, 1>(Shape<1>{Axis{"i", 3}}, {1, 2, 3}), true);

    auto loss1 = sum_all(x * x);
    backward(loss1);
    CHECK(x.grad()[0] == doctest::Approx(2.0));

    Tape::current().clear();
    auto loss2 = sum_all(x * x);
    backward(loss2);
    // Accumulated: (2+2, 4+4, 6+6)
    CHECK(x.grad()[0] == doctest::Approx(4.0));
    CHECK(x.grad()[1] == doctest::Approx(8.0));
    CHECK(x.grad()[2] == doctest::Approx(12.0));
}

TEST_CASE("zero_grad resets accumulator and following backward is fresh") {
    Tape::current().clear();
    Variable<double, 1> x(Tensor<double, 1>(Shape<1>{Axis{"i", 3}}, {1, 2, 3}), true);

    backward(sum_all(x * x));
    x.zero_grad();
    Tape::current().clear();

    backward(sum_all(x * x));
    CHECK(x.grad()[0] == doctest::Approx(2.0));
    CHECK(x.grad()[1] == doctest::Approx(4.0));
    CHECK(x.grad()[2] == doctest::Approx(6.0));
}

TEST_CASE("zero_grad is a no-op on requires_grad=false Variable") {
    Variable<double, 1> x(Tensor<double, 1>(Shape<1>{Axis{"i", 2}}, {1, 2}), false);
    x.zero_grad();  // must not throw
    CHECK_FALSE(x.requires_grad());
}

TEST_CASE("DynamicVariable::zero_grad mirrors Variable::zero_grad") {
    Tape::current().clear();
    DynamicVariable<double> x(
        DynamicTensor<double>(DynamicShape{Axis{"i", 2}}, {2, 5}), true);
    backward(sum_all(x * x));
    CHECK(x.grad()[0] == doctest::Approx(4.0));
    x.zero_grad();
    CHECK(x.grad()[0] == doctest::Approx(0.0));
    CHECK(x.grad()[1] == doctest::Approx(0.0));
}

TEST_CASE("sgd_update produces x - lr * grad") {
    Tape::current().clear();
    Variable<double, 1> x(Tensor<double, 1>(Shape<1>{Axis{"i", 3}}, {1, 2, 3}), true);
    backward(sum_all(x * x));
    // grad = (2, 4, 6); lr = 0.1 → new values (0.8, 1.6, 2.4)
    auto x_new = sgd_update(x, 0.1);
    REQUIRE(x_new.size() == 3);
    CHECK(x_new[0] == doctest::Approx(0.8));
    CHECK(x_new[1] == doctest::Approx(1.6));
    CHECK(x_new[2] == doctest::Approx(2.4));
}

TEST_CASE("sgd_update works on DynamicVariable") {
    Tape::current().clear();
    DynamicVariable<double> w(DynamicTensor<double>(DynamicShape{}, {5.0}), true);
    DynamicVariable<double> x(DynamicTensor<double>(DynamicShape{Axis{"i", 2}}, {1, 2}), false);
    // loss = sum(w * x) ; d/dw = sum(x) = 3
    auto loss = sum_all(w * x);
    backward(loss);
    CHECK(w.grad()[0] == doctest::Approx(3.0));
    auto w_new = sgd_update(w, 0.1);
    CHECK(w_new[0] == doctest::Approx(5.0 - 0.3));
}

TEST_CASE("Training-loop pattern with zero_grad converges (toy linear regression)") {
    // Learn y = 2x + 1 from 5 points using zero_grad + sgd_update.
    DynamicTensor<double> x_data(DynamicShape{Axis{"sample", 5}}, {1, 2, 3, 4, 5});
    DynamicTensor<double> y_target(DynamicShape{Axis{"sample", 5}}, {3, 5, 7, 9, 11});

    double W_val = 0.0;
    double b_val = 0.0;
    double const lr = 0.01;

    for (int epoch = 0; epoch < 300; ++epoch) {
        Tape::current().clear();
        DynamicVariable<double> W(DynamicTensor<double>(DynamicShape{}, {W_val}), true);
        DynamicVariable<double> b(DynamicTensor<double>(DynamicShape{}, {b_val}), true);
        DynamicVariable<double> X(x_data, false);
        DynamicVariable<double> Y(y_target, false);

        auto y_pred = W * X + b;
        auto residual = y_pred - Y;
        auto loss = sum_all(residual * residual);
        backward(loss);

        W_val -= lr * W.grad()[0];
        b_val -= lr * b.grad()[0];
    }
    CHECK(W_val == doctest::Approx(2.0).epsilon(0.05));
    CHECK(b_val == doctest::Approx(1.0).epsilon(0.05));
}
