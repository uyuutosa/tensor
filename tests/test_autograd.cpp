// SPDX-License-Identifier: MIT
//
// Phase 2 MVP — autograd tape smoke + gradient-check parity.

#include <doctest/doctest.h>

#include <tensor/autograd/autograd.hpp>
#include <tensor/core/axis.hpp>
#include <tensor/core/shape.hpp>
#include <tensor/core/tensor.hpp>

using tensor::autograd::backward;
using tensor::autograd::gradient_check;
using tensor::autograd::sum_all;
using tensor::autograd::Tape;
using tensor::autograd::Variable;
using tensor::core::Axis;
using tensor::core::Shape;
using tensor::core::Tensor;

TEST_CASE("Variable construction tracks requires_grad") {
    Tensor<double, 1> v(Shape<1>{Axis{"i", 3}}, {1.0, 2.0, 3.0});
    Variable<double, 1> a(v, true);
    Variable<double, 1> b(v, false);
    CHECK(a.requires_grad());
    CHECK_FALSE(b.requires_grad());
    CHECK(a.value()[0] == 1.0);
}

TEST_CASE("backward of sum(a + b) gives grad of all-ones") {
    Tape::current().clear();
    Variable<double, 1> a(Tensor<double, 1>(Shape<1>{Axis{"i", 3}}, {1.0, 2.0, 3.0}), true);
    Variable<double, 1> b(Tensor<double, 1>(Shape<1>{Axis{"i", 3}}, {10.0, 20.0, 30.0}), true);
    auto loss = sum_all(a + b);
    CHECK(loss.value()[0] == doctest::Approx(66.0));
    backward(loss);
    for (std::size_t i = 0; i < 3; ++i) {
        CHECK(a.grad()[i] == doctest::Approx(1.0));
        CHECK(b.grad()[i] == doctest::Approx(1.0));
    }
}

TEST_CASE("backward of sum(a * a) gives 2 * a") {
    Tape::current().clear();
    Variable<double, 1> a(Tensor<double, 1>(Shape<1>{Axis{"i", 3}}, {1.0, 2.0, 3.0}), true);
    auto loss = sum_all(a * a);
    CHECK(loss.value()[0] == doctest::Approx(14.0));
    backward(loss);
    CHECK(a.grad()[0] == doctest::Approx(2.0));
    CHECK(a.grad()[1] == doctest::Approx(4.0));
    CHECK(a.grad()[2] == doctest::Approx(6.0));
}

TEST_CASE("backward of sum(a - b) gives (1) for a, (-1) for b") {
    Tape::current().clear();
    Variable<double, 1> a(Tensor<double, 1>(Shape<1>{Axis{"i", 3}}, {5.0, 7.0, 11.0}), true);
    Variable<double, 1> b(Tensor<double, 1>(Shape<1>{Axis{"i", 3}}, {1.0, 2.0, 3.0}), true);
    auto loss = sum_all(a - b);
    backward(loss);
    for (std::size_t i = 0; i < 3; ++i) {
        CHECK(a.grad()[i] == doctest::Approx(1.0));
        CHECK(b.grad()[i] == doctest::Approx(-1.0));
    }
}

TEST_CASE("Mixed graph: backward of sum((a + b) * c) gives c on a and b, (a+b) on c") {
    Tape::current().clear();
    Variable<double, 1> a(Tensor<double, 1>(Shape<1>{Axis{"i", 2}}, {1.0, 2.0}), true);
    Variable<double, 1> b(Tensor<double, 1>(Shape<1>{Axis{"i", 2}}, {3.0, 4.0}), true);
    Variable<double, 1> c(Tensor<double, 1>(Shape<1>{Axis{"i", 2}}, {5.0, 6.0}), true);
    auto loss = sum_all((a + b) * c);
    // sum((a+b)*c) = (1+3)*5 + (2+4)*6 = 20 + 36 = 56
    CHECK(loss.value()[0] == doctest::Approx(56.0));
    backward(loss);
    // d/da_i = c_i; d/db_i = c_i; d/dc_i = (a+b)_i
    CHECK(a.grad()[0] == doctest::Approx(5.0));
    CHECK(a.grad()[1] == doctest::Approx(6.0));
    CHECK(b.grad()[0] == doctest::Approx(5.0));
    CHECK(b.grad()[1] == doctest::Approx(6.0));
    CHECK(c.grad()[0] == doctest::Approx(4.0));   // a+b at 0 = 4
    CHECK(c.grad()[1] == doctest::Approx(6.0));   // a+b at 1 = 6
}

TEST_CASE("gradient_check: sum(x * x) parity") {
    Variable<double, 1> x(Tensor<double, 1>(Shape<1>{Axis{"i", 3}}, {1.0, 2.0, 3.0}), true);
    bool ok = gradient_check(
        [](Variable<double, 1> const& v) { return sum_all(v * v); }, x, 1e-4, 1e-3);
    CHECK(ok);
}

TEST_CASE("gradient_check: sum((x+x) * x) parity") {
    Variable<double, 1> x(Tensor<double, 1>(Shape<1>{Axis{"i", 4}}, {0.5, 1.5, -1.0, 2.0}), true);
    bool ok = gradient_check(
        [](Variable<double, 1> const& v) {
            return sum_all((v + v) * v);
        },
        x, 1e-4, 1e-3);
    CHECK(ok);
}

TEST_CASE("Variable without requires_grad does not allocate grad accumulator") {
    Variable<double, 1> a(Tensor<double, 1>(Shape<1>{Axis{"i", 3}}, {1.0, 2.0, 3.0}), false);
    CHECK_THROWS_AS([[maybe_unused]] auto const& g = a.grad(), std::runtime_error);
}

TEST_CASE("Element-wise op with mixed requires_grad still records when at least one needs grad") {
    Tape::current().clear();
    Variable<double, 1> a(Tensor<double, 1>(Shape<1>{Axis{"i", 2}}, {1.0, 2.0}), true);
    Variable<double, 1> b(Tensor<double, 1>(Shape<1>{Axis{"i", 2}}, {3.0, 4.0}), false);
    auto loss = sum_all(a + b);  // tape entry recorded for a only
    backward(loss);
    CHECK(a.grad()[0] == doctest::Approx(1.0));
    CHECK(a.grad()[1] == doctest::Approx(1.0));
}
