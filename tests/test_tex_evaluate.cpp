// SPDX-License-Identifier: MIT
//
// Phase 1.5 mop-up M4 — tex::Evaluator end-to-end:
// parse(latex) → AST → evaluate(bindings) → DynamicTensor.

#include <doctest/doctest.h>

#include <tensor/core/axis.hpp>
#include <tensor/core/dynamic_shape.hpp>
#include <tensor/core/dynamic_tensor.hpp>
#include <tensor/core/reduce.hpp>
#include <tensor/tex/tex.hpp>

using namespace tensor::tex::literals;
using tensor::core::Axis;
using tensor::core::DynamicShape;
using tensor::core::DynamicTensor;
using tensor::tex::Evaluator;

TEST_CASE("reduce_along_label sums along a named axis") {
    DynamicTensor<double> t(DynamicShape{Axis{"i", 3}, Axis{"j", 2}},
                            {1, 2, 3, 4, 5, 6});
    // reduce over i → axes (j); j=0: 1+3+5=9, j=1: 2+4+6=12
    auto r = tensor::core::reduce_along_label(t, "i");
    REQUIRE(r.shape().rank() == 1);
    CHECK(r.shape()[0].label == "j");
    CHECK(r[0] == doctest::Approx(9.0));
    CHECK(r[1] == doctest::Approx(12.0));
}

TEST_CASE("reduce_along_label on missing axis throws") {
    DynamicTensor<double> t(DynamicShape{Axis{"i", 3}}, {1, 2, 3});
    CHECK_THROWS_AS([[maybe_unused]] auto r =
                        tensor::core::reduce_along_label(t, "k"),
                    std::invalid_argument);
}

TEST_CASE("Evaluator: IndexedVar resolves a bound tensor") {
    Evaluator<double> e;
    e.bind("a", DynamicTensor<double>(DynamicShape{Axis{"i", 3}}, {1, 2, 3}));
    auto r = e.evaluate(R"(a_i)"_tex);
    REQUIRE(r.size() == 3);
    CHECK(r[0] == 1.0);
    CHECK(r[2] == 3.0);
}

TEST_CASE("Evaluator: unbound variable throws") {
    Evaluator<double> e;
    auto expr = R"(missing_i)"_tex;
    CHECK_THROWS_AS([[maybe_unused]] auto r = e.evaluate(expr), std::runtime_error);
}

TEST_CASE("Evaluator: subscript order must match shape labels") {
    Evaluator<double> e;
    e.bind("a", DynamicTensor<double>(DynamicShape{Axis{"i", 2}, Axis{"j", 3}},
                                       {1, 2, 3, 4, 5, 6}));
    // Bound shape is (i, j); AST asks for a_{ij} → matches
    auto ok = e.evaluate(R"(a_{ij})"_tex);
    REQUIRE(ok.size() == 6);
    // AST asks for a_{ji} → order mismatch
    CHECK_THROWS_AS([[maybe_unused]] auto bad = e.evaluate(R"(a_{ji})"_tex),
                    std::runtime_error);
}

TEST_CASE("Evaluator: a_i + b_j reproduces 2016 README outer-product sum") {
    Evaluator<double> e;
    e.bind("a", DynamicTensor<double>(DynamicShape{Axis{"i", 5}}, {1, 2, 3, 4, 5}));
    e.bind("b", DynamicTensor<double>(DynamicShape{Axis{"j", 5}}, {1, 2, 3, 4, 5}));
    auto r = e.evaluate(R"(a_i + b_j)"_tex);
    REQUIRE(r.shape().rank() == 2);
    REQUIRE(r.size() == 25);
    // (i, j) = (0, 0): 1+1=2; (4, 4): 5+5=10; (0, 4): 1+5=6
    CHECK(r[0] == doctest::Approx(2.0));
    CHECK(r[24] == doctest::Approx(10.0));
    CHECK(r[4] == doctest::Approx(6.0));
}

TEST_CASE("Evaluator: equation form returns RHS value") {
    Evaluator<double> e;
    e.bind("a", DynamicTensor<double>(DynamicShape{Axis{"i", 3}}, {1, 2, 3}));
    e.bind("b", DynamicTensor<double>(DynamicShape{Axis{"j", 2}}, {10, 20}));
    auto r = e.evaluate(R"(c_{ij} = a_i + b_j)"_tex);
    REQUIRE(r.shape().rank() == 2);
    REQUIRE(r.size() == 6);
    // (i, j) = (0, 0): 1+10=11; (2, 1): 3+20=23
    CHECK(r[0] == doctest::Approx(11.0));
    CHECK(r[5] == doctest::Approx(23.0));
}

TEST_CASE("Evaluator: sum_i a_i reduces to a scalar") {
    Evaluator<double> e;
    e.bind("a", DynamicTensor<double>(DynamicShape{Axis{"i", 5}}, {1, 2, 3, 4, 5}));
    auto r = e.evaluate(R"(\sum_i {a_i})"_tex);
    REQUIRE(r.shape().rank() == 0);
    CHECK(r[0] == doctest::Approx(15.0));
}

TEST_CASE("Evaluator: sum_i a_i b_i is the inner product") {
    Evaluator<double> e;
    e.bind("a", DynamicTensor<double>(DynamicShape{Axis{"i", 3}}, {1, 2, 3}));
    e.bind("b", DynamicTensor<double>(DynamicShape{Axis{"i", 3}}, {4, 5, 6}));
    auto r = e.evaluate(R"(\sum_i {a_i b_i})"_tex);
    REQUIRE(r.shape().rank() == 0);
    // 1·4 + 2·5 + 3·6 = 32
    CHECK(r[0] == doctest::Approx(32.0));
}

TEST_CASE("Evaluator: nested binops compose correctly") {
    Evaluator<double> e;
    e.bind("a", DynamicTensor<double>(DynamicShape{Axis{"i", 2}}, {1, 2}));
    e.bind("b", DynamicTensor<double>(DynamicShape{Axis{"i", 2}}, {3, 4}));
    e.bind("c", DynamicTensor<double>(DynamicShape{Axis{"i", 2}}, {5, 6}));
    auto r = e.evaluate(R"((a_i + b_i) c_i)"_tex);
    // (1+3)*5=20; (2+4)*6=36
    CHECK(r[0] == doctest::Approx(20.0));
    CHECK(r[1] == doctest::Approx(36.0));
}
