// SPDX-License-Identifier: MIT
//
// Phase 2 P2.M4 — autograd-aware contraction (dot).

#include <doctest/doctest.h>

#include <tensor/autograd/autograd.hpp>
#include <tensor/core/axis.hpp>
#include <tensor/core/dynamic_shape.hpp>
#include <tensor/core/dynamic_tensor.hpp>

using namespace tensor::autograd;
using tensor::core::Axis;
using tensor::core::DynamicShape;
using tensor::core::DynamicTensor;

TEST_CASE("dot — matrix-vector: forward + backward gradients") {
    Tape::current().clear();
    DynamicVariable<double> W(DynamicTensor<double>(DynamicShape{Axis{"i", 2}, Axis{"j", 3}},
                                                    {1, 2, 3, 4, 5, 6}),
                              true);
    DynamicVariable<double> x(DynamicTensor<double>(DynamicShape{Axis{"j", 3}}, {10, 20, 30}), true);
    auto y = dot(W, x);
    REQUIRE(y.value().shape().rank() == 1);
    CHECK(y.value()[0] == doctest::Approx(140.0));
    CHECK(y.value()[1] == doctest::Approx(320.0));

    auto loss = sum_all(y);
    backward(loss);
    // dL/dW_{ij} = dL/dy_i · x_j; with dL/dy = (1, 1), expected = x broadcast on i
    CHECK(W.grad()[0] == doctest::Approx(10.0));  // i=0, j=0
    CHECK(W.grad()[1] == doctest::Approx(20.0));  // i=0, j=1
    CHECK(W.grad()[2] == doctest::Approx(30.0));  // i=0, j=2
    CHECK(W.grad()[3] == doctest::Approx(10.0));  // i=1, j=0
    CHECK(W.grad()[5] == doctest::Approx(30.0));  // i=1, j=2
    // dL/dx_j = Σ_i dL/dy_i · W_{ij}; with dL/dy = (1, 1), expected = col-sums of W
    CHECK(x.grad()[0] == doctest::Approx(5.0));   // 1+4
    CHECK(x.grad()[1] == doctest::Approx(7.0));   // 2+5
    CHECK(x.grad()[2] == doctest::Approx(9.0));   // 3+6
}

TEST_CASE("dot — matmul: y = sum(A·B) gradient on A and B") {
    Tape::current().clear();
    DynamicVariable<double> A(DynamicTensor<double>(DynamicShape{Axis{"i", 2}, Axis{"j", 2}},
                                                    {1, 2, 3, 4}),
                              true);
    DynamicVariable<double> B(DynamicTensor<double>(DynamicShape{Axis{"j", 2}, Axis{"k", 2}},
                                                    {5, 6, 7, 8}),
                              true);
    auto C = dot(A, B);
    auto loss = sum_all(C);
    backward(loss);
    // C_{ik} = Σ_j A_{ij} B_{jk}
    // dL/dA_{ij} = Σ_k dL/dC_{ik} · B_{jk} = sum over k of B_{jk} (since dL/dC = 1)
    // For j=0: B_{0k} = (5, 6); sum = 11. For j=1: B_{1k} = (7, 8); sum = 15.
    CHECK(A.grad()[0] == doctest::Approx(11.0));  // i=0, j=0
    CHECK(A.grad()[1] == doctest::Approx(15.0));  // i=0, j=1
    CHECK(A.grad()[2] == doctest::Approx(11.0));  // i=1, j=0
    CHECK(A.grad()[3] == doctest::Approx(15.0));  // i=1, j=1
    // dL/dB_{jk} = Σ_i dL/dC_{ik} · A_{ij} = sum over i of A_{ij}
    // For j=0: A_{i0} = (1, 3); sum = 4. For j=1: A_{i1} = (2, 4); sum = 6.
    CHECK(B.grad()[0] == doctest::Approx(4.0));   // j=0, k=0
    CHECK(B.grad()[1] == doctest::Approx(4.0));   // j=0, k=1
    CHECK(B.grad()[2] == doctest::Approx(6.0));   // j=1, k=0
    CHECK(B.grad()[3] == doctest::Approx(6.0));   // j=1, k=1
}

TEST_CASE("dot — inner product (vec · vec) yields scalar with correct grads") {
    Tape::current().clear();
    DynamicVariable<double> a(DynamicTensor<double>(DynamicShape{Axis{"i", 3}}, {1, 2, 3}), true);
    DynamicVariable<double> b(DynamicTensor<double>(DynamicShape{Axis{"i", 3}}, {4, 5, 6}), true);
    auto s = dot(a, b);
    REQUIRE(s.value().shape().rank() == 0);
    CHECK(s.value()[0] == doctest::Approx(32.0));
    auto loss = sum_all(s);
    backward(loss);
    // dL/da = b; dL/db = a
    CHECK(a.grad()[0] == doctest::Approx(4.0));
    CHECK(a.grad()[1] == doctest::Approx(5.0));
    CHECK(a.grad()[2] == doctest::Approx(6.0));
    CHECK(b.grad()[0] == doctest::Approx(1.0));
    CHECK(b.grad()[1] == doctest::Approx(2.0));
    CHECK(b.grad()[2] == doctest::Approx(3.0));
}

TEST_CASE("dot — outer product (no shared label) gives rank-2 grad chain") {
    Tape::current().clear();
    DynamicVariable<double> a(DynamicTensor<double>(DynamicShape{Axis{"i", 2}}, {1, 2}), true);
    DynamicVariable<double> b(DynamicTensor<double>(DynamicShape{Axis{"j", 3}}, {10, 20, 30}), true);
    auto o = dot(a, b);  // outer product → rank 2 (i, j)
    REQUIRE(o.value().shape().rank() == 2);
    auto loss = sum_all(o);
    backward(loss);
    // dL/da_i = Σ_j 1 · b_j = sum(b) = 60
    CHECK(a.grad()[0] == doctest::Approx(60.0));
    CHECK(a.grad()[1] == doctest::Approx(60.0));
    // dL/db_j = Σ_i 1 · a_i = sum(a) = 3
    CHECK(b.grad()[0] == doctest::Approx(3.0));
    CHECK(b.grad()[1] == doctest::Approx(3.0));
    CHECK(b.grad()[2] == doctest::Approx(3.0));
}
