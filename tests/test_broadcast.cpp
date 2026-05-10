// SPDX-License-Identifier: MIT
#include <doctest/doctest.h>

#include <tensor/core/axis.hpp>
#include <tensor/core/broadcast.hpp>
#include <tensor/core/dynamic_shape.hpp>

using tensor::core::Axis;
using tensor::core::broadcast_npos;
using tensor::core::broadcast_shapes;
using tensor::core::DynamicShape;
using tensor::core::increment_index;
using tensor::core::project_index;

TEST_CASE("broadcast_shapes: distinct labels produce outer product") {
    auto plan = broadcast_shapes(DynamicShape{Axis{"i", 5}},
                                 DynamicShape{Axis{"j", 3}});
    CHECK(plan.result.rank() == 2);
    CHECK(plan.result[0].label == "i");
    CHECK(plan.result[0].extent == 5);
    CHECK(plan.result[1].label == "j");
    CHECK(plan.result[1].extent == 3);
    CHECK(plan.a_source == std::vector<std::size_t>{0, broadcast_npos});
    CHECK(plan.b_source == std::vector<std::size_t>{broadcast_npos, 0});
}

TEST_CASE("broadcast_shapes: matching labels collapse to one axis") {
    auto plan = broadcast_shapes(DynamicShape{Axis{"i", 5}},
                                 DynamicShape{Axis{"i", 5}});
    CHECK(plan.result.rank() == 1);
    CHECK(plan.result[0].label == "i");
    CHECK(plan.a_source == std::vector<std::size_t>{0});
    CHECK(plan.b_source == std::vector<std::size_t>{0});
}

TEST_CASE("broadcast_shapes: extent mismatch on shared label throws") {
    CHECK_THROWS_AS(broadcast_shapes(DynamicShape{Axis{"i", 5}},
                                     DynamicShape{Axis{"i", 4}}),
                    std::invalid_argument);
}

TEST_CASE("broadcast_shapes: A axes preserve order; B-only axes appended") {
    DynamicShape a{Axis{"i", 2}, Axis{"j", 3}};
    DynamicShape b{Axis{"j", 3}, Axis{"k", 4}};
    auto plan = broadcast_shapes(a, b);
    REQUIRE(plan.result.rank() == 3);
    CHECK(plan.result[0].label == "i");
    CHECK(plan.result[1].label == "j");
    CHECK(plan.result[2].label == "k");
    CHECK(plan.a_source == std::vector<std::size_t>{0, 1, broadcast_npos});
    CHECK(plan.b_source == std::vector<std::size_t>{broadcast_npos, 0, 1});
}

TEST_CASE("increment_index walks row-major through a rank-2 shape") {
    DynamicShape s{Axis{"i", 2}, Axis{"j", 3}};
    std::vector<std::size_t> idx(2, 0);
    std::vector<std::pair<std::size_t, std::size_t>> visited;
    do {
        visited.emplace_back(idx[0], idx[1]);
    } while (increment_index(idx, s));
    REQUIRE(visited.size() == 6);
    CHECK(visited[0] == std::make_pair<std::size_t, std::size_t>(0, 0));
    CHECK(visited[1] == std::make_pair<std::size_t, std::size_t>(0, 1));
    CHECK(visited[2] == std::make_pair<std::size_t, std::size_t>(0, 2));
    CHECK(visited[3] == std::make_pair<std::size_t, std::size_t>(1, 0));
    CHECK(visited[5] == std::make_pair<std::size_t, std::size_t>(1, 2));
}

TEST_CASE("project_index drops absent axes via npos") {
    std::vector<std::size_t> result_idx{2, 1};
    // a_source[0] == 0, a_source[1] == npos → A only sees the i axis
    auto a_idx = project_index(result_idx, {0, broadcast_npos}, 1);
    CHECK(a_idx == std::vector<std::size_t>{2});
    // b_source[0] == npos, b_source[1] == 0 → B only sees the j axis
    auto b_idx = project_index(result_idx, {broadcast_npos, 0}, 1);
    CHECK(b_idx == std::vector<std::size_t>{1});
}
