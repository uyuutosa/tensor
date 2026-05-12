// SPDX-License-Identifier: MIT
//
// Phase 2 P2.M3 — broadcast-aware backward on DynamicVariable.

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

TEST_CASE("Distinct-label broadcast: a_i + b_j → c_{ij}; grads reduce over collapsed axes") {
    Tape::current().clear();
    DynamicVariable<double> a(DynamicTensor<double>(DynamicShape{Axis{"i", 3}}, {1.0, 2.0, 3.0}), true);
    DynamicVariable<double> b(DynamicTensor<double>(DynamicShape{Axis{"j", 2}}, {10.0, 20.0}), true);
    auto c = a + b;
    REQUIRE(c.value().shape().rank() == 2);
    REQUIRE(c.value().size() == 6);

    auto loss = sum_all(c);
    // sum(c_{ij}) = 2 sum(a) + 3 sum(b) = 2*6 + 3*30 = 102
    CHECK(loss.value()[0] == doctest::Approx(102.0));
    backward(loss);
    // dL/da_i = sum_j 1 = 2 (j has extent 2)
    // dL/db_j = sum_i 1 = 3 (i has extent 3)
    for (std::size_t i = 0; i < 3; ++i) CHECK(a.grad()[i] == doctest::Approx(2.0));
    for (std::size_t j = 0; j < 2; ++j) CHECK(b.grad()[j] == doctest::Approx(3.0));
}

TEST_CASE("Same-label broadcast collapses to element-wise: rank preserved") {
    Tape::current().clear();
    DynamicVariable<double> a(DynamicTensor<double>(DynamicShape{Axis{"i", 3}}, {1.0, 2.0, 3.0}), true);
    DynamicVariable<double> b(DynamicTensor<double>(DynamicShape{Axis{"i", 3}}, {10.0, 20.0, 30.0}), true);
    auto c = a + b;
    REQUIRE(c.value().shape().rank() == 1);
    CHECK(c.value()[0] == 11.0);
    CHECK(c.value()[2] == 33.0);

    auto loss = sum_all(c);
    backward(loss);
    for (std::size_t i = 0; i < 3; ++i) {
        CHECK(a.grad()[i] == doctest::Approx(1.0));
        CHECK(b.grad()[i] == doctest::Approx(1.0));
    }
}

TEST_CASE("Multiplication broadcast: d/da of sum(a_i * b_j) = sum over j of b_j") {
    Tape::current().clear();
    DynamicVariable<double> a(DynamicTensor<double>(DynamicShape{Axis{"i", 2}}, {1.0, 2.0}), true);
    DynamicVariable<double> b(DynamicTensor<double>(DynamicShape{Axis{"j", 3}}, {10.0, 20.0, 30.0}), true);
    auto loss = sum_all(a * b);
    // sum(a_i * b_j) = (sum a)(sum b) = 3 * 60 = 180
    CHECK(loss.value()[0] == doctest::Approx(180.0));
    backward(loss);
    // dL/da_i = sum_j b_j = 60
    for (std::size_t i = 0; i < 2; ++i) CHECK(a.grad()[i] == doctest::Approx(60.0));
    // dL/db_j = sum_i a_i = 3
    for (std::size_t j = 0; j < 3; ++j) CHECK(b.grad()[j] == doctest::Approx(3.0));
}

TEST_CASE("Subtraction broadcast: d/db is the negated reduction") {
    Tape::current().clear();
    DynamicVariable<double> a(DynamicTensor<double>(DynamicShape{Axis{"i", 2}}, {5.0, 7.0}), true);
    DynamicVariable<double> b(DynamicTensor<double>(DynamicShape{Axis{"j", 3}}, {1.0, 2.0, 3.0}), true);
    auto loss = sum_all(a - b);
    backward(loss);
    // dL/da_i = sum_j 1 = 3
    for (std::size_t i = 0; i < 2; ++i) CHECK(a.grad()[i] == doctest::Approx(3.0));
    // dL/db_j = sum_i (-1) = -2
    for (std::size_t j = 0; j < 3; ++j) CHECK(b.grad()[j] == doctest::Approx(-2.0));
}

TEST_CASE("Mixed shared/distinct labels: a_{ij} + b_j with broadcast over i on b") {
    Tape::current().clear();
    DynamicVariable<double> a(
        DynamicTensor<double>(DynamicShape{Axis{"i", 2}, Axis{"j", 3}},
                              {1.0, 2.0, 3.0, 4.0, 5.0, 6.0}),
        true);
    DynamicVariable<double> b(DynamicTensor<double>(DynamicShape{Axis{"j", 3}}, {10.0, 20.0, 30.0}), true);
    auto c = a + b;
    REQUIRE(c.value().shape().rank() == 2);
    REQUIRE(c.value().size() == 6);
    auto loss = sum_all(c);
    backward(loss);
    // dL/da_{ij} = 1
    for (std::size_t i = 0; i < 6; ++i) CHECK(a.grad()[i] == doctest::Approx(1.0));
    // dL/db_j = sum_i 1 = 2 (i has extent 2)
    for (std::size_t j = 0; j < 3; ++j) CHECK(b.grad()[j] == doctest::Approx(2.0));
}

TEST_CASE("DynamicVariable lift from Variable preserves value and rank") {
    Variable<double, 1> v(Tensor<double, 1>(Shape<1>{Axis{"i", 3}}, {1.0, 2.0, 3.0}), false);
    DynamicVariable<double> dv = v;
    CHECK(dv.value().shape().rank() == 1);
    CHECK(dv.value()[0] == 1.0);
    CHECK(dv.value()[2] == 3.0);
    CHECK_FALSE(dv.requires_grad());  // forwarded from v
}

TEST_CASE("DynamicVariable::to_static throws when grad is being tracked") {
    DynamicVariable<double> dv(DynamicTensor<double>(DynamicShape{Axis{"i", 2}}, {1.0, 2.0}), true);
    CHECK_THROWS_AS([[maybe_unused]] auto v = dv.to_static<1>(), std::runtime_error);
}
