---
status: Stable
owner: tensor
last-reviewed: 2026-05-14
---

# C++ namespace summary — `tensor::*` quick lookup

> The 4-namespace overview of the C++ public surface, with one-line summaries and links to per-module contracts. For a discoverable reference, see [`../user-manual/reference/cpp-namespace-overview.md`](../user-manual/reference/cpp-namespace-overview.md). For HOW per module, see [`../detailed-design/`](../detailed-design/).

| Namespace                                       | One-line summary                                                                                                      | Per-module contract                                                          | Detailed design                                                              |
| ----------------------------------------------- | --------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------- | ---------------------------------------------------------------------------- |
| `tensor::core`                                  | Domain centerpiece — `Axis`, `Shape<N>`, `DynamicShape`, `Tensor<T,N>`, `DynamicTensor<T>`, `TypedTensor`, broadcast / contract / format / mdspan-interop. | [`./cpp-tensor-core-surface.md`](./cpp-tensor-core-surface.md)               | [`../detailed-design/tensor-core.md`](../detailed-design/tensor-core.md)     |
| `tensor::autograd`                              | Tape-based reverse-mode autograd over `Variable<T,N>` + `DynamicVariable<T>`; activations, contractions, training-loop helpers. | [`./cpp-tensor-autograd-surface.md`](./cpp-tensor-autograd-surface.md)       | [`../detailed-design/tensor-autograd.md`](../detailed-design/tensor-autograd.md) |
| `tensor::tex`                                   | LaTeX-subset DSL — `parse(s)`, `to_latex(expr)`, `Evaluator<T>`, and the headline `_tex` UDL.                          | [`./cpp-tensor-tex-surface.md`](./cpp-tensor-tex-surface.md)                 | [`../detailed-design/tensor-tex.md`](../detailed-design/tensor-tex.md)       |
| `tensor::core::concepts::KernelBackend` (port)  | The 15-method port that every backend adapter implements.                                                              | [`./cpp-kernel-backend-port-surface.md`](./cpp-kernel-backend-port-surface.md) | [`../detailed-design/kernel-backend-port.md`](../detailed-design/kernel-backend-port.md) |
| `tensor::core::backend::{reference,eigen,webgpu}` | The three concrete adapters — `reference` (always available), `eigen` (SIMD+GEMM), `webgpu` (Dawn-backed; 12 of 15 on GPU). | (covered by the port surface above)                                          | The WebGPU adapter trio: [`../detailed-design/webgpu-element-wise-kernels.md`](../detailed-design/webgpu-element-wise-kernels.md), [`../detailed-design/webgpu-gemm-kernel.md`](../detailed-design/webgpu-gemm-kernel.md), [`../detailed-design/webgpu-broadcast-kernels.md`](../detailed-design/webgpu-broadcast-kernels.md). |

## Header-include cheatsheet

```cpp
#include <tensor/core/dynamic_tensor.hpp>           // tensor::core umbrella include
#include <tensor/core/typed_tensor.hpp>             // tensor::core::TypedTensor + LabelTag NTTPs
#include <tensor/core/label_tag.hpp>                // _ax UDL
#include <tensor/core/concepts.hpp>                 // KernelBackend concept
#include <tensor/core/backend/reference/backend.hpp> // reference adapter
#include <tensor/core/backend/eigen/backend.hpp>    // eigen adapter (requires -DTENSOR_KERNEL_BACKEND=eigen)
#include <tensor/core/backend/webgpu/backend.hpp>   // webgpu adapter (requires -DTENSOR_KERNEL_BACKEND=webgpu)
#include <tensor/autograd/autograd.hpp>             // tensor::autograd umbrella
#include <tensor/tex/tex.hpp>                       // tensor::tex umbrella + _tex UDL
```

## Stability promise

Pre-`1.0.0`: no ABI commitments per [TC-8](../arc42/02-architecture-constraints/overview.md). The vocabulary is stable in the citability-discipline sense (every public name traces to a §12 glossary entry which traces to a math source); the binary layout is not.

## Cross-references

- Python mirror surface: [`./python-public-surface.md`](./python-public-surface.md).
- Architecture context: [`../arc42/05-building-blocks/overview.md`](../arc42/05-building-blocks/overview.md), [`../arc42/06-runtime/overview.md`](../arc42/06-runtime/overview.md).
- Glossary: [`../arc42/12-glossary/overview.md`](../arc42/12-glossary/overview.md).
- Reference quadrant (user-facing): [`../user-manual/reference/cpp-namespace-overview.md`](../user-manual/reference/cpp-namespace-overview.md).
