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
#include <vector>

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

// ─── KernelBackend ────────────────────────────────────────────────────────────
//
// The port that production-grade adapters (Eigen, BLAS, Kokkos, WebGPU, …)
// implement to plug into the Domain. See ADR-0010 (positioning) and ADR-0011
// (port API design). The active adapter is selected at CMake configure time
// via TENSOR_KERNEL_BACKEND={reference, eigen, ...}.
//
// Per ADR-0011:
//   - Each method returns a freshly-allocated DynamicTensor<T>.
//   - BroadcastPlan / ContractPlan are computed in the Domain
//     (tensor::core::broadcast_shapes / contract_plan) and passed in.
//   - unbroadcast is part of the port (reduce-over-collapsed-axes belongs
//     in the backend so Eigen / Kokkos can specialise it).
//   - Per-op backward closures are NOT part of the port — they live in
//     tensor::autograd and compose port methods.
//
// `tensor::core::backend::DefaultBackend<Derived>` (introduced in
// P2.5.M2) provides reference-quality implementations of every method so
// adapter authors can selectively override only the operations they want to
// fast-path.

// Forward declarations for the types the KernelBackend concept refers to.
// concepts.hpp is included from tensor.hpp (for TensorLike static_assert),
// so it cannot pull in dynamic_tensor.hpp without a circular include.
// The forward declarations are sufficient because the concept uses these
// types only in unevaluated contexts inside the `requires` clauses.
template <class T> class DynamicTensor;
class DynamicShape;
struct BroadcastPlan;
struct ContractPlan;

template <class B>
concept KernelBackend = requires(B b) {
    typename B::backend_tag;
} && requires(B b,
              DynamicTensor<double> const& a,
              DynamicTensor<double> const& a2,
              BroadcastPlan const& bplan,
              ContractPlan const& cplan,
              std::vector<std::size_t> const& source_map,
              DynamicShape const& source_shape) {
    // Element-wise binary, same shape.
    { b.add(a, a2) } -> std::same_as<DynamicTensor<double>>;
    { b.sub(a, a2) } -> std::same_as<DynamicTensor<double>>;
    { b.mul(a, a2) } -> std::same_as<DynamicTensor<double>>;
    { b.div(a, a2) } -> std::same_as<DynamicTensor<double>>;

    // Element-wise unary.
    { b.exp(a) }  -> std::same_as<DynamicTensor<double>>;
    { b.log(a) }  -> std::same_as<DynamicTensor<double>>;
    { b.relu(a) } -> std::same_as<DynamicTensor<double>>;
    { b.neg(a) }  -> std::same_as<DynamicTensor<double>>;

    // Broadcast element-wise.
    { b.broadcast_add(a, a2, bplan) } -> std::same_as<DynamicTensor<double>>;
    { b.broadcast_sub(a, a2, bplan) } -> std::same_as<DynamicTensor<double>>;
    { b.broadcast_mul(a, a2, bplan) } -> std::same_as<DynamicTensor<double>>;

    // Contraction (Einstein-style).
    { b.contract(a, a2, cplan) } -> std::same_as<DynamicTensor<double>>;

    // Reduction.
    { b.reduce_sum(a) } -> std::same_as<double>;

    // Unbroadcast (used by autograd to reduce dL/dout to an input's shape).
    { b.unbroadcast(a, source_map, source_shape) }
        -> std::same_as<DynamicTensor<double>>;
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
