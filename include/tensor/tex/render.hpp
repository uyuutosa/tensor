// SPDX-License-Identifier: MIT
//
// tensor::tex::to_latex — render an Expression back to its LaTeX string.
//
// Property test target: parse(to_latex(e)) == e for every expression in
// the corpus the parser accepts. We use single-letter braces for
// subscripts only when the index sequence has more than one letter
// ({ij}); single-letter subscripts are emitted without braces (a_i).

#pragma once

#include <sstream>
#include <string>

#include "tensor/tex/expression.hpp"

namespace tensor::tex {

namespace detail {

inline char binop_symbol(BinOpKind op) noexcept {
    switch (op) {
        case BinOpKind::Add: return '+';
        case BinOpKind::Sub: return '-';
        case BinOpKind::Mul: return '*';
        case BinOpKind::Div: return '/';
    }
    return '?';
}

inline void render_indices(std::ostream& os, std::vector<std::string> const& idx) {
    if (idx.size() == 1 && idx[0].size() == 1) {
        os << '_' << idx[0];
    } else {
        os << "_{";
        for (auto const& i : idx) {
            os << i;
        }
        os << '}';
    }
}

inline void render_node(std::ostream& os, Node const& n) {
    switch (n.kind) {
        case NodeKind::IndexedVar: {
            os << n.name;
            if (!n.indices.empty()) {
                render_indices(os, n.indices);
            }
            break;
        }
        case NodeKind::BinOp: {
            // Always parenthesise children of BinOps so the parser does not
            // need precedence-climbing at render time. Parens cost a few
            // bytes but make the round-trip property trivially provable.
            os << '(';
            render_node(os, *n.children[0]);
            os << ' ' << binop_symbol(n.binop_kind) << ' ';
            render_node(os, *n.children[1]);
            os << ')';
            break;
        }
        case NodeKind::Sum: {
            os << "\\sum";
            render_indices(os, n.indices);
            os << " {";
            render_node(os, *n.children[0]);
            os << "}";
            break;
        }
        case NodeKind::Equation: {
            render_node(os, *n.children[0]);
            os << " = ";
            render_node(os, *n.children[1]);
            break;
        }
        case NodeKind::Group: {
            os << '{';
            render_node(os, *n.children[0]);
            os << '}';
            break;
        }
    }
}

}  // namespace detail

[[nodiscard]] inline std::string to_latex(Expression const& expr) {
    if (expr.empty()) {
        return {};
    }
    std::ostringstream oss;
    detail::render_node(oss, expr.root());
    return oss.str();
}

}  // namespace tensor::tex
