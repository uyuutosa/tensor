// SPDX-License-Identifier: MIT
//
// tensor::tex::Expression — AST for the LaTeX-subset DSL.
//
// `tensor::tex` is a DrivingAdapter per ADR-0009: it produces expression
// trees that downstream code (the evaluator bridge in M4-followup, the
// notebook renderer in M6, the WGSL codegen in M3-gpu) can consume.
//
// The AST is a tag + children variant, deliberately kept simple so the
// recursive-descent parser stays fewer than 200 lines and the round-trip
// `parse(render(e)) == e` property holds for the supported subset.

#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace tensor::tex {

// Forward declaration so Node can hold std::unique_ptr<Node> children.
struct Node;
using NodePtr = std::unique_ptr<Node>;

enum class NodeKind {
    IndexedVar,  // e.g. a_i, a_{ij}
    BinOp,       // +, -, *, /, juxtaposition (=> implicit *)
    Sum,         // \sum_i {body}, \sum_{ij} {body}
    Equation,    // {lhs} = {rhs}
    Group,       // {body} — preserved for round-trip fidelity of explicit grouping
};

enum class BinOpKind { Add, Sub, Mul, Div };

struct Node {
    NodeKind kind;

    // IndexedVar / Sum payload
    std::string name;                 // variable name for IndexedVar; unused for Sum
    std::vector<std::string> indices; // subscripts (e.g. {"i", "j"} for a_{ij})

    // BinOp payload
    BinOpKind binop_kind{BinOpKind::Add};

    // Children (binary ops use children[0], children[1]; Sum uses children[0]
    // for body; Equation uses [0]=lhs, [1]=rhs; Group uses [0]=body)
    std::vector<NodePtr> children;

    Node() = default;
    explicit Node(NodeKind k) : kind(k) {}
};

// ─── Constructors (free helpers; AST is value-shaped) ──────────────────────────
[[nodiscard]] inline NodePtr make_indexed_var(std::string name,
                                              std::vector<std::string> indices) {
    auto n = std::make_unique<Node>(NodeKind::IndexedVar);
    n->name = std::move(name);
    n->indices = std::move(indices);
    return n;
}

[[nodiscard]] inline NodePtr make_binop(BinOpKind op, NodePtr lhs, NodePtr rhs) {
    auto n = std::make_unique<Node>(NodeKind::BinOp);
    n->binop_kind = op;
    n->children.push_back(std::move(lhs));
    n->children.push_back(std::move(rhs));
    return n;
}

[[nodiscard]] inline NodePtr make_sum(std::vector<std::string> indices, NodePtr body) {
    auto n = std::make_unique<Node>(NodeKind::Sum);
    n->indices = std::move(indices);
    n->children.push_back(std::move(body));
    return n;
}

[[nodiscard]] inline NodePtr make_equation(NodePtr lhs, NodePtr rhs) {
    auto n = std::make_unique<Node>(NodeKind::Equation);
    n->children.push_back(std::move(lhs));
    n->children.push_back(std::move(rhs));
    return n;
}

[[nodiscard]] inline NodePtr make_group(NodePtr body) {
    auto n = std::make_unique<Node>(NodeKind::Group);
    n->children.push_back(std::move(body));
    return n;
}

// ─── Structural equality (for round-trip property tests) ──────────────────────
[[nodiscard]] inline bool equal(Node const& a, Node const& b) {
    if (a.kind != b.kind) return false;
    if (a.name != b.name) return false;
    if (a.indices != b.indices) return false;
    if (a.binop_kind != b.binop_kind) return false;
    if (a.children.size() != b.children.size()) return false;
    for (std::size_t i = 0; i < a.children.size(); ++i) {
        if (!a.children[i] || !b.children[i]) return false;
        if (!equal(*a.children[i], *b.children[i])) return false;
    }
    return true;
}

// `Expression` is a thin owning wrapper around the root NodePtr.
class Expression {
public:
    Expression() = default;
    explicit Expression(NodePtr root) : root_(std::move(root)) {}

    [[nodiscard]] Node const& root() const { return *root_; }
    [[nodiscard]] Node& root() { return *root_; }
    [[nodiscard]] bool empty() const noexcept { return root_ == nullptr; }

    friend bool operator==(Expression const& lhs, Expression const& rhs) {
        if (lhs.empty() && rhs.empty()) return true;
        if (lhs.empty() || rhs.empty()) return false;
        return equal(*lhs.root_, *rhs.root_);
    }

private:
    NodePtr root_{};
};

}  // namespace tensor::tex
