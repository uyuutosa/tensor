// SPDX-License-Identifier: MIT
//
// tensor::tex::parse — recursive-descent parser for the LaTeX subset.
//
// Grammar (informal, in priority order):
//
//   equation    := expression ('=' expression)?
//   expression  := term (('+' | '-') term)*
//   term        := factor (('*' | '\cdot' | (juxtaposition)) factor)*
//   factor      := ('-' factor) | unary
//   unary       := atom (('/' atom)*)
//   atom        := '(' expression ')'
//                | '\sum' '_' subscript group
//                | name subscript?
//   subscript   := '_' (single_letter | '{' letters '}')
//   group       := '{' expression '}' | atom
//
// Whitespace is ignored. Implicit multiplication is detected by the term
// parser when it sees an unexpected start-of-atom token.
//
// This is a runtime parser; the `_tex` UDL in `udl.hpp` calls it. A
// follow-up will lift the parser into `consteval` once a fixed-size AST
// representation lands (currently AST uses unique_ptr which cannot
// escape constant evaluation in C++20).

#pragma once

#include <cctype>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "tensor/tex/expression.hpp"

namespace tensor::tex {

namespace detail {

class Parser {
public:
    explicit Parser(std::string_view src) : src_(src) {}

    [[nodiscard]] Expression parse_top() {
        skip_ws();
        auto lhs = parse_expression();
        skip_ws();
        if (peek() == '=') {
            consume('=');
            auto rhs = parse_expression();
            skip_ws();
            require_eof();
            return Expression(make_equation(std::move(lhs), std::move(rhs)));
        }
        require_eof();
        return Expression(std::move(lhs));
    }

private:
    NodePtr parse_expression() {
        auto lhs = parse_term();
        while (true) {
            skip_ws();
            char c = peek();
            if (c == '+') {
                consume('+');
                auto rhs = parse_term();
                lhs = make_binop(BinOpKind::Add, std::move(lhs), std::move(rhs));
            } else if (c == '-') {
                consume('-');
                auto rhs = parse_term();
                lhs = make_binop(BinOpKind::Sub, std::move(lhs), std::move(rhs));
            } else {
                break;
            }
        }
        return lhs;
    }

    NodePtr parse_term() {
        auto lhs = parse_unary();
        while (true) {
            skip_ws();
            char c = peek();
            if (c == '*') {
                consume('*');
                auto rhs = parse_unary();
                lhs = make_binop(BinOpKind::Mul, std::move(lhs), std::move(rhs));
            } else if (starts_with("\\cdot")) {
                pos_ += 5;
                auto rhs = parse_unary();
                lhs = make_binop(BinOpKind::Mul, std::move(lhs), std::move(rhs));
            } else if (is_atom_start(c)) {
                // Juxtaposition = implicit multiplication.
                auto rhs = parse_unary();
                lhs = make_binop(BinOpKind::Mul, std::move(lhs), std::move(rhs));
            } else {
                break;
            }
        }
        return lhs;
    }

    NodePtr parse_unary() {
        auto lhs = parse_atom();
        while (true) {
            skip_ws();
            if (peek() == '/') {
                consume('/');
                auto rhs = parse_atom();
                lhs = make_binop(BinOpKind::Div, std::move(lhs), std::move(rhs));
            } else {
                break;
            }
        }
        return lhs;
    }

    NodePtr parse_atom() {
        skip_ws();
        char c = peek();
        if (c == '(') {
            consume('(');
            auto inner = parse_expression();
            skip_ws();
            consume(')');
            return inner;
        }
        if (c == '{') {
            consume('{');
            auto inner = parse_expression();
            skip_ws();
            consume('}');
            return make_group(std::move(inner));
        }
        if (starts_with("\\sum")) {
            pos_ += 4;
            skip_ws();
            consume('_');
            auto indices = parse_subscript_list();
            skip_ws();
            // Body — accept either a group {...} or a single atom.
            NodePtr body;
            if (peek() == '{') {
                consume('{');
                body = parse_expression();
                skip_ws();
                consume('}');
            } else {
                body = parse_atom();
            }
            return make_sum(std::move(indices), std::move(body));
        }
        // Otherwise: name (one identifier-letter chunk) plus optional subscript.
        std::string name = parse_name();
        if (name.empty()) {
            throw std::runtime_error(
                std::string("tex::parse: unexpected character at position ") +
                std::to_string(pos_));
        }
        std::vector<std::string> indices;
        skip_ws();
        if (peek() == '_') {
            consume('_');
            indices = parse_subscript_list();
        }
        return make_indexed_var(std::move(name), std::move(indices));
    }

    std::vector<std::string> parse_subscript_list() {
        std::vector<std::string> out;
        skip_ws();
        if (peek() == '{') {
            consume('{');
            // Accept one or more single-letter index identifiers; commas allowed
            // but ignored. Permissive subset; richer subscript content (numbers,
            // expressions) deferred.
            while (true) {
                skip_ws();
                char c = peek();
                if (c == '}') break;
                if (c == ',') {
                    consume(',');
                    continue;
                }
                if (std::isalpha(static_cast<unsigned char>(c))) {
                    std::string idx(1, c);
                    ++pos_;
                    out.push_back(std::move(idx));
                } else {
                    throw std::runtime_error(
                        std::string("tex::parse: bad index char at position ") +
                        std::to_string(pos_));
                }
            }
            consume('}');
        } else {
            // Single-letter subscript without braces: a_i
            char c = peek();
            if (!std::isalpha(static_cast<unsigned char>(c))) {
                throw std::runtime_error(
                    std::string("tex::parse: subscript must be a letter at position ") +
                    std::to_string(pos_));
            }
            out.emplace_back(1, c);
            ++pos_;
        }
        return out;
    }

    std::string parse_name() {
        std::string name;
        while (pos_ < src_.size() &&
               std::isalpha(static_cast<unsigned char>(src_[pos_]))) {
            name.push_back(src_[pos_]);
            ++pos_;
        }
        return name;
    }

    // ─── Lex helpers ──────────────────────────────────────────────────────
    [[nodiscard]] char peek() const noexcept {
        return pos_ < src_.size() ? src_[pos_] : '\0';
    }
    void consume(char c) {
        if (peek() != c) {
            throw std::runtime_error(
                std::string("tex::parse: expected '") + c +
                "' at position " + std::to_string(pos_));
        }
        ++pos_;
    }
    void skip_ws() {
        while (pos_ < src_.size() &&
               std::isspace(static_cast<unsigned char>(src_[pos_]))) {
            ++pos_;
        }
    }
    [[nodiscard]] bool starts_with(std::string_view s) const noexcept {
        return src_.substr(pos_, s.size()) == s;
    }
    [[nodiscard]] bool is_atom_start(char c) const noexcept {
        return std::isalpha(static_cast<unsigned char>(c)) || c == '(' ||
               c == '\\';
    }
    void require_eof() {
        skip_ws();
        if (pos_ != src_.size()) {
            throw std::runtime_error(
                std::string("tex::parse: trailing characters at position ") +
                std::to_string(pos_));
        }
    }

    std::string_view src_;
    std::size_t pos_{0};
};

}  // namespace detail

[[nodiscard]] inline Expression parse(std::string_view src) {
    return detail::Parser(src).parse_top();
}

}  // namespace tensor::tex
