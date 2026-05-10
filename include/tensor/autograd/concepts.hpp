// SPDX-License-Identifier: MIT
//
// tensor::autograd::concepts — Domain-extension port surface.
//
// Per ADR-0009, tensor::autograd is a Domain extension on top of
// tensor::core. This file declares the autograd-side ports; downstream
// adapters (a future tensor::gpu autograd backend, alternative tape
// implementations) implement these.

#pragma once

#include <concepts>
#include <cstddef>

namespace tensor::autograd {

// ─── Differentiable ───────────────────────────────────────────────────────────
// A type satisfies Differentiable if it exposes a forward `value()`, a
// `grad()` accessor that returns the accumulated gradient, and a runtime
// `requires_grad()` flag. `Variable<T, N>` is the canonical implementation.
template <class V>
concept Differentiable = requires(V const& v) {
    { v.value() };
    { v.grad() };
    { v.requires_grad() } -> std::convertible_to<bool>;
};

// ─── BackwardOp ───────────────────────────────────────────────────────────────
// A BackwardOp is anything callable with no arguments that propagates the
// gradient of one tape step into its inputs' grad accumulators. Tape
// entries are constructed from BackwardOps; the tape walks them in reverse
// during `backward()`.
template <class F>
concept BackwardOp = requires(F f) {
    { f() } -> std::same_as<void>;
};

// ─── BackwardSink ─────────────────────────────────────────────────────────────
// A BackwardSink receives gradient contributions during the backward
// pass. The default sink is GradAccum<T, N>, but an alternative
// (e.g. a streaming sink that writes to disk for large graphs) can plug
// in here.
template <class S>
concept BackwardSink = requires(S& s) {
    { s.initialized } -> std::convertible_to<bool>;
};

}  // namespace tensor::autograd
