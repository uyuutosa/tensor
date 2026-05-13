---
status: Stable
owner: tensor
last-reviewed: 2026-05-14
---

# C++ public surface — `KernelBackend` port

> The 15-method `tensor::core::concepts::KernelBackend` C++20 concept that every backend adapter must satisfy. Complete HOW: [`../detailed-design/kernel-backend-port.md`](../detailed-design/kernel-backend-port.md). To implement a new adapter: [`../user-manual/how-to/add-a-new-backend.md`](../user-manual/how-to/add-a-new-backend.md).

## 1. The port itself

| Concept                                          | Header                                         |
| ------------------------------------------------ | ---------------------------------------------- |
| `tensor::core::concepts::KernelBackend`          | `include/tensor/core/concepts.hpp`              |

## 2. The 15 required methods

A type `Backend` satisfies the concept iff it provides all 15:

### Element-wise binary (4)

| Method     | Semantics                                  |
| ---------- | ------------------------------------------ |
| `add`      | `c = a + b` element-wise.                   |
| `sub`      | `c = a - b` element-wise.                   |
| `mul`      | `c = a * b` element-wise.                   |
| `div`      | `c = a / b` element-wise.                   |

### Element-wise unary (4)

| Method     | Semantics                                  |
| ---------- | ------------------------------------------ |
| `exp`      | `c = exp(a)` element-wise.                  |
| `log`      | `c = log(a)` element-wise.                  |
| `relu`     | `c = max(0, a)` element-wise.               |
| `neg`      | `c = -a` element-wise.                      |

### Contraction (1)

| Method     | Semantics                                          |
| ---------- | -------------------------------------------------- |
| `contract` | Einstein-sum over shared axis labels. Bound on the GPU adapter to the simple-GEMM case; multi-axis delegates to `reference`. |

### Broadcast (3)

| Method            | Semantics                                                                            |
| ----------------- | ------------------------------------------------------------------------------------ |
| `broadcast_add`    | `add` with broadcast — operands' shapes need not match.                              |
| `broadcast_sub`    | same shape rules, `-`.                                                               |
| `broadcast_mul`    | same shape rules, `*`.                                                               |

### Reduction + unbroadcast (3)

| Method            | Semantics                                                                            |
| ----------------- | ------------------------------------------------------------------------------------ |
| `reduce_sum`       | Sum reduction over a named axis. Currently delegated to `reference` on the WebGPU adapter. |
| `unbroadcast`      | The inverse of broadcast — used by autograd backward paths. Currently delegated to `reference` on the WebGPU adapter. |
| `reduce_along_label` | Bundle B addition — single-axis sum with autograd support.                          |

## 3. Adapter implementations as of 2026-05-14

| Adapter                                       | Header root                                       | Methods on real fast path                                                | Notes |
| --------------------------------------------- | ------------------------------------------------- | ------------------------------------------------------------------------ | ----- |
| `tensor::core::backend::reference::Backend`   | `include/tensor/core/backend/reference/`           | 15 of 15 (canonical CPU)                                                  | Always available.                                                                   |
| `tensor::core::backend::eigen::Backend`       | `include/tensor/core/backend/eigen/`               | 8 of 15 element-wise unary/binary + GEMM simple-contract via Eigen        | Out-of-scope methods delegate to a `reference::Backend` member.                      |
| `tensor::core::backend::webgpu::Backend`      | `include/tensor/core/backend/webgpu/`              | 12 of 15 on real Dawn WGSL (4 binary + 4 unary + simple-contract + 3 broadcast) | `reduce_sum`, `unbroadcast`, multi-axis `contract` delegate to `reference`. |

## 4. The contract reads

```cpp
template <typename B>
concept KernelBackend = requires(B b, /* canonical operands */) {
    { b.add(a, b)        } -> std::same_as<DynamicTensor<T>>;
    { b.sub(a, b)        } -> std::same_as<DynamicTensor<T>>;
    // ... 13 more
};
```

(See `include/tensor/core/concepts.hpp` for the exact requires-clause; the 15 methods are listed with their full signatures.)

## 5. Conformance check pattern

Every adapter ends its `backend.hpp` with:

```cpp
static_assert(tensor::core::concepts::KernelBackend<Backend>);
```

This is the compile-time correctness witness. The runtime correctness witness is cross-validation against `reference::Backend` within the QO-1 tolerance envelope (`1e-9` for `double`, `1e-5` for `float`).

## 6. Phase 6.5 — runtime selection from Python

`tensor.set_backend("eigen")` ([ADR-0019](../arc42/09-decisions/0019-phase-6-5-runtime-backend-selection-via-extras.md), planned `0.3.0`) selects the active backend at Python runtime. The C++ port surface is unchanged; only the Python adapter that consumes the port gains lazy module loading + rebinding logic. See [`../detailed-design/python-sdk-binding-surface.md` §3.5](../detailed-design/python-sdk-binding-surface.md).

## 7. Cross-references

- Detailed design: [`../detailed-design/kernel-backend-port.md`](../detailed-design/kernel-backend-port.md).
- How to add a new adapter: [`../user-manual/how-to/add-a-new-backend.md`](../user-manual/how-to/add-a-new-backend.md).
- The runtime selector (Python side): [`../user-manual/how-to/use-set-backend.md`](../user-manual/how-to/use-set-backend.md).
- Glossary: [`../arc42/12-glossary/overview.md`](../arc42/12-glossary/overview.md) — `Port`, `Adapter`, `KernelBackend`.
- Anchor ADRs: [ADR-0009](../arc42/09-decisions/0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md), [ADR-0010](../arc42/09-decisions/0010-refine-positioning-to-educational-first-production-capable.md), [ADR-0011](../arc42/09-decisions/0011-kernel-backend-port-api.md), [ADR-0012](../arc42/09-decisions/0012-webgpu-adapter-implementation-design.md), [ADR-0016](../arc42/09-decisions/0016-substrate-refinement-drop-gpu-cpp-talk-to-dawn-directly.md).
