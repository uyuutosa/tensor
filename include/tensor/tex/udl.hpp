// SPDX-License-Identifier: MIT
//
// tensor::tex::operator""_tex — user-defined literal entry point.
//
// At present this is a runtime wrapper around tensor::tex::parse(...)
// because the AST uses std::unique_ptr, which cannot escape a C++20
// constant-evaluated context. ADR-0005 calls for a `consteval` parser;
// that will become the primary path once a fixed-size AST representation
// (or C++23 lifted-restrictions-on-constexpr-allocations) lands.

#pragma once

#include <cstddef>
#include <string_view>

#include "tensor/tex/expression.hpp"
#include "tensor/tex/parser.hpp"

namespace tensor::tex::literals {

[[nodiscard]] inline Expression operator""_tex(char const* s, std::size_t n) {
    return tensor::tex::parse(std::string_view(s, n));
}

}  // namespace tensor::tex::literals
