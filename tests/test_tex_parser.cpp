// SPDX-License-Identifier: MIT
#include <doctest/doctest.h>

#include <tensor/tex/tex.hpp>

using tensor::tex::BinOpKind;
using tensor::tex::Expression;
using tensor::tex::NodeKind;
using tensor::tex::parse;
using tensor::tex::to_latex;

TEST_CASE("Parse a single indexed variable a_i") {
    auto e = parse("a_i");
    REQUIRE_FALSE(e.empty());
    auto const& n = e.root();
    CHECK(n.kind == NodeKind::IndexedVar);
    CHECK(n.name == "a");
    REQUIRE(n.indices.size() == 1);
    CHECK(n.indices[0] == "i");
}

TEST_CASE("Parse a multi-letter subscript a_{ij}") {
    auto e = parse("a_{ij}");
    auto const& n = e.root();
    CHECK(n.kind == NodeKind::IndexedVar);
    CHECK(n.name == "a");
    REQUIRE(n.indices.size() == 2);
    CHECK(n.indices[0] == "i");
    CHECK(n.indices[1] == "j");
}

TEST_CASE("Parse a + b with subscripts") {
    auto e = parse("a_i + b_j");
    auto const& n = e.root();
    REQUIRE(n.kind == NodeKind::BinOp);
    CHECK(n.binop_kind == BinOpKind::Add);
    REQUIRE(n.children.size() == 2);
    CHECK(n.children[0]->kind == NodeKind::IndexedVar);
    CHECK(n.children[0]->name == "a");
    CHECK(n.children[1]->name == "b");
}

TEST_CASE("Parse implicit multiplication via juxtaposition") {
    auto e = parse("a_i b_i");
    auto const& n = e.root();
    REQUIRE(n.kind == NodeKind::BinOp);
    CHECK(n.binop_kind == BinOpKind::Mul);
}

TEST_CASE("Parse \\cdot as explicit multiplication") {
    auto e = parse("a_i \\cdot b_j");
    auto const& n = e.root();
    REQUIRE(n.kind == NodeKind::BinOp);
    CHECK(n.binop_kind == BinOpKind::Mul);
}

TEST_CASE("Parse equation lhs = rhs") {
    auto e = parse("c_{ij} = a_i + b_j");
    auto const& n = e.root();
    REQUIRE(n.kind == NodeKind::Equation);
    REQUIRE(n.children.size() == 2);
    CHECK(n.children[0]->name == "c");
    CHECK(n.children[1]->kind == NodeKind::BinOp);
}

TEST_CASE("Parse \\sum") {
    auto e = parse(R"(\sum_i {a_i b_i})");
    auto const& n = e.root();
    REQUIRE(n.kind == NodeKind::Sum);
    REQUIRE(n.indices.size() == 1);
    CHECK(n.indices[0] == "i");
    CHECK(n.children[0]->kind == NodeKind::BinOp);
}

TEST_CASE("Round-trip property: parse(render(e)) == e") {
    auto inputs = {
        std::string_view{"a_i"},
        std::string_view{"a_{ij}"},
        std::string_view{"a_i + b_j"},
        std::string_view{"a_i - b_j"},
        std::string_view{"a_i * b_j"},
        std::string_view{"a_i / b_j"},
        std::string_view{"a_i b_i"},
        std::string_view{"c_{ij} = a_i + b_j"},
        std::string_view{R"(\sum_i {a_i b_i})"},
        std::string_view{"a_i + b_j + c_k"},
    };
    for (auto src : inputs) {
        auto e = parse(src);
        auto rendered = to_latex(e);
        auto re_parsed = parse(rendered);
        CHECK_MESSAGE(e == re_parsed, "round-trip failed for: " << src);
    }
}

TEST_CASE("Trailing characters throw") {
    // Use a non-atom-start, non-operator character so the parser cannot
    // absorb it via juxtaposition. `?` is not alphabetic and is not a
    // recognised operator; require_eof() at the end of parse_top() flags
    // it as unconsumed input.
    CHECK_THROWS_AS(parse("a_i ?"), std::runtime_error);
}

TEST_CASE("Bad subscript throws") {
    CHECK_THROWS_AS(parse("a_{1}"), std::runtime_error);
}

TEST_CASE("_tex UDL parses identically to parse()") {
    using namespace tensor::tex::literals;
    auto from_udl = R"(c_{ij} = a_i + b_j)"_tex;
    auto from_parse = parse("c_{ij} = a_i + b_j");
    CHECK(from_udl == from_parse);
}
