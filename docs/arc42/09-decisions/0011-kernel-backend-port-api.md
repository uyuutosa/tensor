---
status: Accepted
owner: tensor
last-reviewed: 2026-05-11
---

# ADR-0011: `KernelBackend` port API design

| Metadata  | Value                                                                  |
| --------- | ---------------------------------------------------------------------- |
| Status    | **Accepted**                                                           |
| Type      | ADR (Type 5 / MADR v3.0 / arc42 §9)                                    |
| Date      | 2026-05-11                                                             |
| Deciders  | uyuutosa                                                               |
| Consulted | Claude (session of 2026-05-11)                                         |
| Informed  | future contributors / backend authors                                  |
| Ticket    | —                                                                      |
| Realises  | the follow-up from [ADR-0010](0010-refine-positioning-to-educational-first-production-capable.md). |

---

## Context and Problem Statement

[ADR-0010](0010-refine-positioning-to-educational-first-production-capable.md) commits the project to "educational-first, production-capable via backend adapters". The mechanism is the `KernelBackend` port declared anticipatorily in [`include/tensor/core/concepts.hpp`](../../../include/tensor/core/concepts.hpp). The port is currently a marker (`requires { typename B::backend_tag; }`), which means *no adapter can actually satisfy it* — there are no methods to implement.

Phase 2.5 cannot start the Eigen adapter (P2.5.M3) until the port's API is concrete. This ADR fixes that API.

The design space spans:

- **Granularity**: one method per logical op (`add`, `mul`, `contract`, …) vs. a generic dispatch (`apply<Op>(...)`).
- **Buffer ownership model**: does the port return owned tensors or write into caller-allocated outputs?
- **Where broadcast lives**: in the port or in the Domain layer that calls into it?
- **Backward composability**: does the port include unbroadcast / shape-restoration primitives, or are those Domain code that composes port primitives?

---

## Decision Drivers

- **DD-1**: Adapter writers (especially community contributors writing a Kokkos or BLAS adapter) want a small, clear, per-op API.
- **DD-2**: Each backend (Eigen, BLAS, WebGPU, …) has its own preferred granularity; the port must not force them through a slow generic dispatch when a native fast path exists.
- **DD-3**: Domain code (autograd, named-axis broadcast logic) must remain backend-agnostic; no Eigen / Kokkos types may leak into the Domain headers.
- **DD-4**: Tests must keep passing identically across backends; the port's contract must be observable and unambiguous.
- **DD-5**: The existing reference implementation (`ops.hpp`, `broadcast_ops.hpp`, `contract.hpp`) needs to satisfy the new port with a refactor, not a rewrite.

---

## Considered Options

1. **Generic dispatch** — single `apply<Op>(a, b)` template; backends implement one function that dispatches on `Op` tags.
2. **Per-op methods (chosen)** — backend exposes named methods (`add`, `mul`, `contract`, …); the Domain calls them directly.
3. **Visitor pattern** — backend implements `visit(Expression)`; Domain builds an expression tree and hands it to the backend.

---

## Decision Outcome

**Chosen option: 2 — per-op methods.**

The `KernelBackend` concept is a template parameter expressing a *set of method signatures* a type must provide. The Domain instantiates kernels through a `Backend` parameter, and at CMake configure time the build selects which concrete backend is used via `TENSOR_KERNEL_BACKEND={reference, eigen, ...}` — this drives a typedef `tensor::core::backend::active_t` to point at the active adapter type.

### The concept (initial surface)

```cpp
template <class B>
concept KernelBackend = requires(B b) {
    typename B::backend_tag;  // marker — for error messages and tag-dispatch helpers
} && requires(B b,
              tensor::core::DynamicTensor<double> const& a1,
              tensor::core::DynamicTensor<double> const& a2,
              tensor::core::BroadcastPlan const& bplan,
              tensor::core::ContractPlan   const& cplan,
              std::vector<std::size_t>     const& source_map,
              tensor::core::DynamicShape   const& source_shape) {
    // ── Element-wise binary ops (same shape) ───────────────────────────
    { b.add(a1, a2) } -> std::same_as<tensor::core::DynamicTensor<double>>;
    { b.sub(a1, a2) } -> std::same_as<tensor::core::DynamicTensor<double>>;
    { b.mul(a1, a2) } -> std::same_as<tensor::core::DynamicTensor<double>>;
    { b.div(a1, a2) } -> std::same_as<tensor::core::DynamicTensor<double>>;

    // ── Element-wise unary ops ─────────────────────────────────────────
    { b.exp(a1) } -> std::same_as<tensor::core::DynamicTensor<double>>;
    { b.log(a1) } -> std::same_as<tensor::core::DynamicTensor<double>>;
    { b.relu(a1) } -> std::same_as<tensor::core::DynamicTensor<double>>;
    { b.neg(a1) } -> std::same_as<tensor::core::DynamicTensor<double>>;

    // ── Broadcast element-wise (the rank-changing path) ────────────────
    { b.broadcast_add(a1, a2, bplan) } -> std::same_as<tensor::core::DynamicTensor<double>>;
    { b.broadcast_sub(a1, a2, bplan) } -> std::same_as<tensor::core::DynamicTensor<double>>;
    { b.broadcast_mul(a1, a2, bplan) } -> std::same_as<tensor::core::DynamicTensor<double>>;

    // ── Contraction (Einstein-style) ───────────────────────────────────
    { b.contract(a1, a2, cplan) } -> std::same_as<tensor::core::DynamicTensor<double>>;

    // ── Reductions ─────────────────────────────────────────────────────
    { b.reduce_sum(a1) } -> std::same_as<double>;

    // ── Unbroadcast (used by autograd) ─────────────────────────────────
    { b.unbroadcast(a1, source_map, source_shape) }
        -> std::same_as<tensor::core::DynamicTensor<double>>;
};
```

`double` in the snippet above is illustrative; the real concept is templated over an element type `T` (see implementation).

### Buffer ownership model

- Every method returns a freshly allocated `DynamicTensor<T>`.
- No write-into-caller-allocated-output variant in the MVP (additive future work).
- Backends that want zero-copy fusion (Eigen's expression templates, WebGPU pipelined kernels) implement those *internally*; the port surface stays value-shaped.

### Where broadcast lives

- `BroadcastPlan` and `ContractPlan` (the metadata) are computed by the **Domain** (`tensor::core::broadcast_shapes`, `tensor::core::contract_plan`) and passed into the backend.
- The backend only executes the lowered, planned operation. This keeps the named-axis algebra in the Domain and the math kernels in the adapter.

### Backward composability

- `unbroadcast` is part of the port because autograd needs it for reverse-mode of broadcast ops, and its efficient implementation (reduce along specific axes) deserves backend-specific tuning. Eigen / Kokkos can implement it more efficiently than the generic walker.
- Per-op backward closures (`dexp = y · dL/dout`, etc.) are **not** part of the port — they are composed in `tensor::autograd` from the port's forward primitives.

### Compile-time selection

```cmake
# CMakeLists.txt
set(TENSOR_KERNEL_BACKEND "reference" CACHE STRING "Active KernelBackend adapter")
set_property(CACHE TENSOR_KERNEL_BACKEND PROPERTY STRINGS reference eigen)

if(TENSOR_KERNEL_BACKEND STREQUAL "eigen")
    target_compile_definitions(tensor INTERFACE TENSOR_ACTIVE_BACKEND_EIGEN=1)
    find_package(Eigen3 CONFIG REQUIRED)
    target_link_libraries(tensor INTERFACE Eigen3::Eigen)
else()  # reference (default)
    target_compile_definitions(tensor INTERFACE TENSOR_ACTIVE_BACKEND_REFERENCE=1)
endif()
```

The Domain headers then pick the active backend type alias:

```cpp
namespace tensor::core::backend {
#if defined(TENSOR_ACTIVE_BACKEND_EIGEN)
    using active_t = eigen::Backend;
#else
    using active_t = reference::Backend;
#endif
    inline active_t& active() { static active_t b; return b; }
}
```

Domain call sites become `tensor::core::backend::active().add(a, b)` (or similar wrapped helpers).

### Y-statement summary

> In the context of **operationalising ADR-0010's "production-capable via backend adapters" promise**, facing **the design of the `KernelBackend` port API**, we decided for **per-op named methods returning fresh `DynamicTensor<T>`s, with `BroadcastPlan` / `ContractPlan` computed in the Domain and passed in, and compile-time backend selection via a CMake cache variable**, to achieve **a clear small API that each adapter can fast-path natively without dispatch overhead**, accepting **the small redundancy of one method per operator instead of a single generic dispatcher**.

---

## Pros and Cons of the Options

### Option 1: Generic dispatch (`apply<Op>`)

- Pros: tiny port surface; adapter writes one method.
- Cons: forces all backends through a single dispatch path; defeats Eigen's expression templates; cannot specialise `matmul` separately from `add`.

### Option 2: Per-op methods (chosen)

- Pros: each backend implements only the ops it cares about (with a default that delegates to reference); Eigen, BLAS, WebGPU each get their natural fast path.
- Cons: ~15 method signatures to satisfy; more boilerplate for new adapters. Mitigated by a `tensor::core::backend::DefaultBackend<Derived>` CRTP helper that provides reference implementations of methods a `Derived` does not override.

### Option 3: Visitor pattern over Expression

- Pros: maximally flexible — the backend can fuse, JIT, schedule.
- Cons: massive design and implementation cost; defers shipping by months; mismatched with the "small and readable" educational identity.

---

## Consequences

### Positive

- The next Phase 2.5 PR (P2.5.M2) can refactor the existing kernels into an explicit `tensor::core::backend::reference` namespace that satisfies the concept.
- P2.5.M3 (Eigen adapter) implements a smaller subset (Eigen handles `add` / `sub` / `mul` / `div` element-wise natively; falls back to `DefaultBackend` for the rest).
- P2.5.M4 (perf comparison) compares fixed cases (matmul, element-wise) across the two backends.
- Phase 3 (WebGPU) becomes a third adapter following the same pattern.

### Negative

- The port has 15+ methods. Adapter authors who only want to specialise one or two need the `DefaultBackend` CRTP helper to keep boilerplate sane.
- Element-type plumbing through `template <class T>` everywhere is verbose. Mitigated by `using value_type = T` typedefs on `DynamicTensor`.

### Neutral

- The Domain's named-axis algebra (`broadcast_shapes`, `contract_plan`) does not change. Adapters consume the same metadata.

### Follow-ups

- [ ] P2.5.M2: implement `tensor::core::backend::reference::Backend` satisfying the concept, refactored from current `ops.hpp` / `broadcast_ops.hpp` / `contract.hpp`.
- [ ] P2.5.M3: implement `tensor::core::backend::eigen::Backend` for element-wise + matmul; default-fall-through for the rest.
- [ ] CMake option `TENSOR_KERNEL_BACKEND` with `STRINGS reference eigen`.
- [ ] `DefaultBackend<Derived>` CRTP helper providing reference-implementations of every concept method (so partial adapters compile).
- [ ] Phase 2.5 plan: mark P2.5.M1 done; P2.5.M2 unblocked.

---

## Compliance / Validation

- **Verification**: each concrete adapter has a CI job that runs the full doctest suite with `TENSOR_KERNEL_BACKEND=<adapter>`; results must match across backends within a documented tolerance for floating-point ops (1e-6 absolute for fp64).
- **Frequency**: every PR.

---

## More Information

### Related ADRs

- Refined positioning that motivates this port: [ADR-0010](0010-refine-positioning-to-educational-first-production-capable.md).
- Hexagonal lite that introduces the port concept: [ADR-0009](0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md).
- C++20 baseline that the concept requires: [ADR-0002](0002-rewrite-on-cpp20-baseline-with-mdspan-interop.md).
- Specific future backends: [ADR-0006 (WebGPU)](0006-adopt-webgpu-as-gpu-backend.md), and an upcoming ADR for Eigen if the integration grows beyond a thin adapter.

### References

- [Eigen Array API](https://eigen.tuxfamily.org/dox/group__TutorialArrayClass.html) — informs the element-wise method signatures.
- [Kokkos View / parallel_for](https://github.com/kokkos/kokkos) — informs the future multi-backend adapter shape.
- [tinygrad ops layer](https://github.com/tinygrad/tinygrad/tree/master/tinygrad/ops) — precedent for a small per-op surface.
