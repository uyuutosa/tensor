// SPDX-License-Identifier: MIT
//
// Phase 1.5 mop-up M3 — TypedTensor end-to-end: compile-time labels +
// runtime data + same-label arithmetic + conversion to DynamicTensor.

#include <doctest/doctest.h>

#include <string_view>
#include <type_traits>

#include <tensor/core/dynamic_tensor.hpp>
#include <tensor/core/typed_tensor.hpp>

using tensor::core::DynamicTensor;
using tensor::core::SameLabels;
using tensor::core::TypedTensor;

TEST_CASE("TypedTensor construction from extents + values") {
    TypedTensor<double, "i", "j"> a({2, 3}, {1, 2, 3, 4, 5, 6});
    REQUIRE(a.size() == 6);
    CHECK(a[0] == 1.0);
    CHECK(a[5] == 6.0);
    CHECK(decltype(a)::rank() == 2);
}

TEST_CASE("TypedTensor mismatched value count throws") {
    using TT = TypedTensor<double, "i", "j">;
    CHECK_THROWS_AS([[maybe_unused]] auto _ = TT({2, 3}, {1, 2, 3}), std::invalid_argument);
}

TEST_CASE("Label introspection: label_at<I>() returns the i-th compile-time label") {
    using TT = TypedTensor<double, "alpha", "beta">;
    constexpr auto l0 = TT::label_at<0>();
    constexpr auto l1 = TT::label_at<1>();
    static_assert(l0 == std::string_view{"alpha"});
    static_assert(l1 == std::string_view{"beta"});
    CHECK(l0 == "alpha");
    CHECK(l1 == "beta");
}

TEST_CASE("Same-label TypedTensor + works (compile-time accepted)") {
    TypedTensor<double, "i", "j"> a({2, 3}, {1, 2, 3, 4, 5, 6});
    TypedTensor<double, "i", "j"> b({2, 3}, {10, 20, 30, 40, 50, 60});
    auto c = a + b;
    REQUIRE(c.size() == 6);
    for (std::size_t i = 0; i < 6; ++i) CHECK(c[i] == doctest::Approx((i + 1) * 11.0));
}

TEST_CASE("Same-label TypedTensor * is element-wise") {
    TypedTensor<double, "i"> a({3}, {1, 2, 3});
    TypedTensor<double, "i"> b({3}, {10, 20, 30});
    auto c = a * b;
    CHECK(c[0] == 10);
    CHECK(c[2] == 90);
}

TEST_CASE("SameLabels concept distinguishes matching vs non-matching label packs") {
    using A = TypedTensor<double, "i", "j">;
    using B = TypedTensor<double, "i", "j">;
    using C = TypedTensor<double, "i", "k">;
    using D = TypedTensor<double, "i">;
    static_assert(SameLabels<A, B>);
    static_assert(!SameLabels<A, C>);   // last label differs
    static_assert(!SameLabels<A, D>);   // rank differs
}

TEST_CASE("to_dynamic produces a DynamicTensor whose shape carries the labels") {
    TypedTensor<double, "i", "j"> a({2, 3}, {1, 2, 3, 4, 5, 6});
    DynamicTensor<double> d = a.to_dynamic();
    REQUIRE(d.shape().rank() == 2);
    CHECK(d.shape()[0].label == "i");
    CHECK(d.shape()[1].label == "j");
    CHECK(d.shape()[0].extent == 2);
    CHECK(d.shape()[1].extent == 3);
    CHECK(d[0] == 1.0);
    CHECK(d[5] == 6.0);
}
