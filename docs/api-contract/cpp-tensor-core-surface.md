---
status: Stable
owner: tensor
last-reviewed: 2026-05-14
---

# C++ public surface — `tensor::core`

> The public C++ symbols of the Domain centerpiece (`tensor::core` namespace). The complete HOW lives in [`../detailed-design/tensor-core.md`](../detailed-design/tensor-core.md); this file is the one-line-per-symbol *contract* — every reference in tutorials / how-tos / arc42 should resolve here.

## 1. Stability promise

Pre-`1.0.0`: no ABI commitments per [TC-8](../arc42/02-architecture-constraints/overview.md) + [ADR-0010](../arc42/09-decisions/0010-refine-positioning-to-educational-first-production-capable.md). Each public name is **stable in the citability-discipline sense** — public names trace to §12 glossary entries which trace to math-literature sources. The *binary* layout is not stable; the *vocabulary* is.

## 2. Types

| Type                                  | Header                                          | Purpose                                                                 |
| ------------------------------------- | ----------------------------------------------- | ----------------------------------------------------------------------- |
| `tensor::core::Axis`                  | `include/tensor/core/axis.hpp`                  | `(label: std::string, extent: size_t)` pair. Label-equality is identity. |
| `tensor::core::Shape<N>`              | `include/tensor/core/shape.hpp`                 | Compile-time-rank shape (`N` Axis values).                              |
| `tensor::core::DynamicShape`          | `include/tensor/core/dynamic_shape.hpp`         | Runtime-rank shape (ordered `Axis` vector). `.rank()`, `.total()`, `.axes()`. |
| `tensor::core::Tensor<T, N>`          | `include/tensor/core/tensor.hpp`                | Compile-time-rank tensor over scalar `T`.                                |
| `tensor::core::DynamicTensor<T>`      | `include/tensor/core/dynamic_tensor.hpp`        | Runtime-rank tensor — the educational default.                           |
| `tensor::core::TypedTensor<T, …>`     | `include/tensor/core/typed_tensor.hpp`           | Compile-time-labelled tensor (NTTP labels).                              |
| `tensor::core::LabelTag<S>`           | `include/tensor/core/label_tag.hpp`              | Compile-time label tag (consumed by `_ax` UDL).                          |
| `tensor::core::FixedString<N>`        | `include/tensor/core/fixed_string.hpp`           | String-literal-as-NTTP helper (C++20).                                   |
| `tensor::core::BroadcastPlan`         | `include/tensor/core/broadcast.hpp`              | Broadcast strides + result shape, computed once per op pair.            |
| `tensor::core::ContractPlan`          | `include/tensor/core/contract.hpp`               | Contraction index map + result shape.                                    |

## 3. Operations

| Operation                                              | Header                                  | Semantics                                                                    |
| ------------------------------------------------------ | --------------------------------------- | ---------------------------------------------------------------------------- |
| `operator+` / `operator-` / `operator*` / `operator/`   | `include/tensor/core/ops.hpp`           | Element-wise on same-label operands; Einstein-broadcast on disjoint-label operands. |
| `tensor::core::contract(a, b)`                          | `include/tensor/core/contract.hpp`      | Einstein-sum over all axes that appear in both operands.                     |
| `operator<<`                                            | `include/tensor/core/format.hpp`        | ASCII-box rendering. Mirrors the 2016 README byte-for-byte for the canonical examples. |
| `tensor::core::broadcast_shapes(a, b)`                  | `include/tensor/core/broadcast.hpp`     | Returns the broadcast result shape + `BroadcastPlan`.                        |

## 4. Concepts (ports)

| Concept                                       | Header                                      | What it constrains                                          |
| --------------------------------------------- | ------------------------------------------- | ----------------------------------------------------------- |
| `tensor::core::concepts::AxisLike`            | `include/tensor/core/concepts.hpp`          | Anything that exposes `label` + `extent` accessors.         |
| `tensor::core::concepts::ShapeLike`           | `include/tensor/core/concepts.hpp`          | Anything that exposes `.rank()` + `.axes()` accessors.      |
| `tensor::core::concepts::KernelBackend`       | `include/tensor/core/concepts.hpp`          | The 15-method backend port (see [kernel-backend-port.md](../detailed-design/kernel-backend-port.md)). |

## 5. Literals (UDL)

| UDL                              | Header                                      | Expansion                                                                  |
| -------------------------------- | ------------------------------------------- | -------------------------------------------------------------------------- |
| `operator""_ax`                  | `include/tensor/core/label_tag.hpp`         | `"i"_ax` → `LabelTag<FixedString{"i"}>{}`. Compile-time label production.    |
| `operator""_tex` (in `tensor::tex`) | `include/tensor/tex/tex.hpp`                | `R"(c_{ij} = a_i b_j)"_tex` → `tex::Expression` AST. See [`./cpp-tensor-tex-surface.md`](./cpp-tensor-tex-surface.md). |

## 6. Adapter subnamespaces

| Namespace                                       | Header root                                       | Purpose                                                              |
| ----------------------------------------------- | ------------------------------------------------- | -------------------------------------------------------------------- |
| `tensor::core::backend::reference`              | `include/tensor/core/backend/reference/`           | Canonical CPU implementation. Always available.                       |
| `tensor::core::backend::eigen`                  | `include/tensor/core/backend/eigen/`               | Eigen 3.4-backed (SIMD + GEMM). Requires `-DTENSOR_KERNEL_BACKEND=eigen`. |
| `tensor::core::backend::webgpu`                 | `include/tensor/core/backend/webgpu/`              | Dawn-backed. 12 of 15 methods on real GPU. Requires `-DTENSOR_KERNEL_BACKEND=webgpu` + Dawn vcpkg port. |

The full port contract is at [`./cpp-kernel-backend-port-surface.md`](./cpp-kernel-backend-port-surface.md).

## 7. Headers users include

The umbrella include is:

```cpp
#include <tensor/core/dynamic_tensor.hpp>   // pulls in DynamicShape, Axis, Tensor, ops, format
```

For specific sub-features:

```cpp
#include <tensor/core/typed_tensor.hpp>     // compile-time labels via NTTPs
#include <tensor/core/label_tag.hpp>        // _ax UDL
#include <tensor/core/contract.hpp>         // contract function
#include <tensor/core/concepts.hpp>         // KernelBackend concept
#include <tensor/core/backend/<name>.hpp>   // a specific adapter
```

## 8. Cross-references

- Detailed design: [`../detailed-design/tensor-core.md`](../detailed-design/tensor-core.md).
- Reference index (human-readable): [`../user-manual/reference/cpp-namespace-overview.md`](../user-manual/reference/cpp-namespace-overview.md).
- Python mirror surface: [`./python-public-surface.md`](./python-public-surface.md).
- Adapter contract: [`./cpp-kernel-backend-port-surface.md`](./cpp-kernel-backend-port-surface.md).
- Glossary: [`../arc42/12-glossary/overview.md`](../arc42/12-glossary/overview.md) — `Axis`, `Shape`, `Tensor`, `Broadcast`, `Contraction`, etc.
