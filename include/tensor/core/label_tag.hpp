// SPDX-License-Identifier: MIT
//
// tensor::core::LabelTag — compile-time axis label identity.
//
// Realises the NTTP fast path of ADR-0004's hybrid named-axis API.
// `LabelTag<"i">` is a stateless type whose identity carries an axis
// label as a C++20 non-type template parameter (NTTP). Operations that
// accept `LabelTag` instances can perform axis-mismatch checks at
// compile time — the runtime label string is never consulted.
//
// The user-defined literal `"i"_ax` (in `tensor::core::literals`)
// constructs a `LabelTag` whose template parameter is deduced from the
// string literal. The same `LabelTag` is callable as a function with a
// runtime extent to produce a runtime `Axis`:
//
//   auto a_axis = "i"_ax(5);   // Axis{"i", 5}, but with the *type* carrying "i".
//
// Two distinct labels produce two distinct types; mismatched-axis
// detection is then just type-equality at the call site.

#pragma once

#include <algorithm>
#include <cstddef>
#include <string_view>
#include <type_traits>

#include "tensor/core/axis.hpp"

namespace tensor::core {

// ─── FixedString ─────────────────────────────────────────────────────────────
//
// Class-type NTTPs in C++20 must be *structural*: trivially copyable, all
// public, no user-declared destructor, etc. FixedString is the minimal
// such carrier for a compile-time character sequence.
template <std::size_t N>
struct FixedString {
    char data[N]{};

    constexpr FixedString(char const (&s)[N]) {
        std::copy_n(s, N, data);
    }

    [[nodiscard]] constexpr std::string_view view() const noexcept {
        // Strip the trailing null terminator that the array literal carries.
        return std::string_view(data, N > 0 ? N - 1 : 0);
    }

    [[nodiscard]] constexpr std::size_t length() const noexcept {
        return N > 0 ? N - 1 : 0;
    }

    friend constexpr bool operator==(FixedString const&, FixedString const&) = default;
};

// Deduction guide so `FixedString("i")` infers N from the literal length.
template <std::size_t N>
FixedString(char const (&)[N]) -> FixedString<N>;

// ─── LabelTag ────────────────────────────────────────────────────────────────
//
// Stateless type carrying the label as a NTTP. Two LabelTags are the
// *same type* iff their `S` parameters compare equal, so type-equality
// becomes the compile-time label check.
template <FixedString S>
struct LabelTag {
    using value_type = decltype(S);

    [[nodiscard]] static constexpr std::string_view label() noexcept {
        return S.view();
    }

    // Construct a runtime Axis with this compile-time label and the given
    // extent. The Axis is a value-type with a runtime-stored label string;
    // the type identity is forgotten on the way out (but the label string
    // is preserved).
    //
    // NOT marked `constexpr` because `Axis` is not a literal type
    // (`std::string` member). GCC 11 silently allowed `constexpr` here
    // since the function was never invoked in a constexpr context, but
    // Clang 21 (and xeus-cpp's Clang-Repl) rejects at declaration-time.
    // The function is callable at runtime only — which is all the
    // tutorials + tests ever did anyway.
    [[nodiscard]] Axis operator()(std::size_t extent) const {
        return Axis{S.view(), extent};
    }
};

// Two LabelTag types are equal iff their FixedString NTTPs are equal —
// which the compiler already enforces structurally. We expose this as a
// concept for use in higher-level constructs.
template <class A, class B>
concept SameLabel = std::is_same_v<A, B>;

// Compile-time helper: assert at instantiation time that two LabelTags
// have the same label. Used inside operators that require label match.
template <class A, class B>
constexpr void assert_same_label() {
    static_assert(SameLabel<A, B>,
                  "tensor::core::LabelTag: axis label mismatch detected at compile time.");
}

// ─── User-defined literal: "i"_ax ────────────────────────────────────────────
//
// C++20 NTTP-templated UDL: `"i"_ax` deduces `S = FixedString<2>{"i"}` and
// returns a stateless `LabelTag<S>` instance. The user does not write the
// template arguments.
namespace literals {

template <FixedString S>
constexpr LabelTag<S> operator""_ax() noexcept {
    return {};
}

}  // namespace literals

}  // namespace tensor::core
