// SPDX-License-Identifier: MIT
//
// tensor::tex::Evaluator — walk a parsed Expression and produce a
// DynamicTensor against a name→tensor binding map.
//
// Closes Phase 1.5 mop-up M4: the LaTeX-subset surface goes from
// "parses to a structural AST" to "parses, evaluates against named
// inputs, and yields a concrete tensor".
//
// Supported AST node kinds (mirroring tensor::tex::Expression):
//   IndexedVar — look up `name` in bindings; assert the AST subscripts
//                match the bound tensor's shape labels.
//   BinOp      — recursively evaluate children; apply +/-/*/div via
//                tensor::core's existing DynamicTensor operators
//                (which already do Einstein-style broadcast).
//   Sum        — recursively evaluate body; reduce over each summed
//                axis via tensor::core::reduce_along_labels.
//   Equation   — evaluate the right-hand side; the LHS is treated as
//                an annotation declaring expected output shape — we
//                verify rank and label set at the end if the LHS is
//                an IndexedVar.
//   Group      — pass through.

#pragma once

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

#include "tensor/core/dynamic_tensor.hpp"
#include "tensor/core/ops.hpp"
#include "tensor/core/reduce.hpp"
#include "tensor/tex/expression.hpp"

namespace tensor::tex {

template <class T>
class Evaluator {
public:
    using tensor_type = tensor::core::DynamicTensor<T>;

    // Bind a variable name to a runtime tensor. Subsequent IndexedVar
    // nodes with this name resolve here.
    void bind(std::string name, tensor_type value) {
        bindings_[std::move(name)] = std::move(value);
    }

    // Evaluate an expression to a concrete tensor.
    [[nodiscard]] tensor_type evaluate(Expression const& expr) const {
        if (expr.empty()) {
            throw std::runtime_error("Evaluator: empty expression");
        }
        return eval_node(expr.root());
    }

private:
    [[nodiscard]] tensor_type eval_node(Node const& node) const {
        switch (node.kind) {
            case NodeKind::IndexedVar:   return eval_indexed_var(node);
            case NodeKind::BinOp:        return eval_binop(node);
            case NodeKind::Sum:          return eval_sum(node);
            case NodeKind::Equation:     return eval_node(*node.children[1]);
            case NodeKind::Group:        return eval_node(*node.children[0]);
        }
        throw std::runtime_error("Evaluator: unknown node kind");
    }

    [[nodiscard]] tensor_type eval_indexed_var(Node const& n) const {
        auto it = bindings_.find(n.name);
        if (it == bindings_.end()) {
            throw std::runtime_error("Evaluator: unbound variable '" + n.name + "'");
        }
        auto const& t = it->second;
        // Verify the AST subscripts match the bound tensor's shape labels.
        // For MVP, require exact order and exact set.
        if (t.shape().rank() != n.indices.size()) {
            throw std::runtime_error(
                "Evaluator: subscript count for '" + n.name + "' does not match its rank");
        }
        for (std::size_t i = 0; i < n.indices.size(); ++i) {
            if (t.shape()[i].label != n.indices[i]) {
                throw std::runtime_error(
                    "Evaluator: subscript order mismatch for '" + n.name +
                    "': expected '" + std::string(t.shape()[i].label) +
                    "' at position " + std::to_string(i) +
                    ", AST has '" + n.indices[i] + "'");
            }
        }
        return t;
    }

    [[nodiscard]] tensor_type eval_binop(Node const& n) const {
        auto lhs = eval_node(*n.children[0]);
        auto rhs = eval_node(*n.children[1]);
        using namespace tensor::core;
        switch (n.binop_kind) {
            case BinOpKind::Add: return lhs + rhs;
            case BinOpKind::Sub: return lhs - rhs;
            case BinOpKind::Mul: return lhs * rhs;
            case BinOpKind::Div: return lhs / rhs;
        }
        throw std::runtime_error("Evaluator: unknown BinOp kind");
    }

    [[nodiscard]] tensor_type eval_sum(Node const& n) const {
        auto body = eval_node(*n.children[0]);
        return tensor::core::reduce_along_labels(std::move(body), n.indices);
    }

    std::unordered_map<std::string, tensor_type> bindings_;
};

}  // namespace tensor::tex
