// SPDX-License-Identifier: MIT
//
// Phase 1.5 mop-up — ADR-0004 compile-time path: `LabelTag<S>` + `_ax` UDL.

#include <doctest/doctest.h>

#include <string>
#include <type_traits>

#include <tensor/core/axis.hpp>
#include <tensor/core/label_tag.hpp>

// CHECK(rvalue_string_view == "literal") triggers a MSVC 14.44 parse bug
// inside <__msvc_string_view.hpp> when doctest instantiates
// Expression_lhs<const std::string_view &&>::operator==<const char(&)[N]>.
// Workaround: route the LHS through std::string so doctest sees the
// std::string overload (well-exercised, no template gymnastics). Drop
// once doctest / MSVC ship a fix and the CI matrix moves past the bug.

using namespace tensor::core::literals;
using tensor::core::Axis;
using tensor::core::FixedString;
using tensor::core::LabelTag;
using tensor::core::SameLabel;

TEST_CASE("FixedString reflects literal length and content") {
    constexpr FixedString fs("hello");
    static_assert(fs.length() == 5);
    CHECK(std::string(fs.view()) == "hello");
}

TEST_CASE("_ax UDL produces stable LabelTag types per label") {
    auto i1 = "i"_ax;
    auto i2 = "i"_ax;
    auto j  = "j"_ax;
    static_assert(std::is_same_v<decltype(i1), decltype(i2)>);
    static_assert(!std::is_same_v<decltype(i1), decltype(j)>);
}

TEST_CASE("LabelTag::label() yields the compile-time label as string_view") {
    constexpr auto lbl = decltype("k"_ax)::label();
    static_assert(lbl == "k");
    CHECK(std::string(lbl) == "k");
}

TEST_CASE("LabelTag is callable: forms a runtime Axis with the compile-time label") {
    Axis a = "i"_ax(5);
    CHECK(std::string(a.label) == "i");
    CHECK(a.extent == 5);
}

TEST_CASE("SameLabel concept is true iff types match") {
    static_assert(SameLabel<decltype("i"_ax), decltype("i"_ax)>);
    static_assert(!SameLabel<decltype("i"_ax), decltype("j"_ax)>);
}

TEST_CASE("assert_same_label is a static_assert helper (compile-time check)") {
    // Compiles fine because labels match.
    tensor::core::assert_same_label<decltype("i"_ax), decltype("i"_ax)>();
    // Mismatched-label case is not exercised here because static_assert
    // failure aborts the whole TU. The negative path is documented in
    // ADR-0004 and demonstrated in tutorial 01 (Phase 1.5 follow-up).
}

TEST_CASE("LabelTag is stateless / empty") {
    static_assert(sizeof(LabelTag<FixedString{"x"}>) == 1 ||
                  std::is_empty_v<LabelTag<FixedString{"x"}>>);
}
