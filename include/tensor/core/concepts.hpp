// SPDX-License-Identifier: MIT
//
// tensor::core::concepts — the Domain-side port surface (ADR-0009).
//
// This is the **only** header under include/tensor/core/ that adapters
// (tensor::tex, tensor::gpu, tensor::autograd) are allowed to depend on
// directly. Concrete domain types may be referenced by adapters, but
// only via the concepts declared here. See
// docs/design-guide/architectural-discipline.md for the dependency rule.

#pragma once

#include <concepts>
#include <cstddef>
#include <string_view>
#include <type_traits>

namespace tensor::core {

// ─── AxisLike ─────────────────────────────────────────────────────────────────
// An Axis carries a label (e.g. "i", "j") and an extent. Axis identity is
// runtime by default (per ADR-0004); the NTTP compile-time fast path
// (LabelTag<"i">) introduces a refinement satisfying the same concept in M3.
template <class A>
concept AxisLike = requires(A const& a) {
    { a.label } -> std::convertible_to<std::string_view>;
    { a.extent } -> std::convertible_to<std::size_t>;
};

// ─── ShapeLike ────────────────────────────────────────────────────────────────
// A Shape is an ordered sequence of AxisLike entries.
//
// The concept is parameterised over the *decayed* type so that references
// and cv-qualified shapes also satisfy it. The rank is exposed via a
// runtime method `.rank()` — both static-rank `Shape<N>` and runtime-rank
// `DynamicShape` provide one, so the concept does not need to peek at
// static members.
template <class S>
concept ShapeLike = requires(std::remove_cvref_t<S> const& s) {
    { s.rank() } -> std::convertible_to<std::size_t>;
    { s[std::size_t{0}] };  // axis access
};

// ─── TensorLike ───────────────────────────────────────────────────────────────
// A TensorLike type owns or views a buffer of T-typed elements with a
// ShapeLike shape and exposes element access. Rank is reachable through
// `t.shape().rank()`; not required to be a static member, so the concept
// is satisfied by both static-rank `Tensor<T, N>` and runtime-rank
// `DynamicTensor<T>`.
template <class T>
concept TensorLike = requires(T const& t) {
    typename T::value_type;
    typename T::shape_type;
    requires ShapeLike<decltype(t.shape())>;
    { t.size() } -> std::convertible_to<std::size_t>;
};

// ─── KernelBackend (anticipatory) ─────────────────────────────────────────────
// A KernelBackend can execute a tensor expression and produce a result tensor.
// The CPU reference backend lives inside tensor::core; tensor::gpu (M3+)
// will be a separate adapter implementing this concept. Refined in later
// milestones once the expression-graph type is settled.
template <class B>
concept KernelBackend = requires {
    typename B::backend_tag;  // marker type identifying the backend
};

// ─── BufferExporter (anticipatory) ────────────────────────────────────────────
// Anything that can produce an mdspan-shaped non-owning view over its buffer.
// The mdspan view type itself is templated, so this concept is intentionally
// open until M2 wires up `mdview()` against the real polyfill / std type.
template <class E>
concept BufferExporter = requires(E const& e) {
    { e.data() };  // pointer to first element
    { e.size() } -> std::convertible_to<std::size_t>;
};

}  // namespace tensor::core
