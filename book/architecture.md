# Architecture at a glance

`tensor` follows the [pentaglyph-docs](https://github.com/uyuutosa/pentaglyph-docs) documentation kit — arc42 + C4 + MADR + Diátaxis + TiSDD bound by one workflow. The full architecture surface lives under [`docs/`](https://github.com/uyuutosa/tensor/tree/develop/docs); the entry points are:

| Entry point | Purpose |
| ----------- | ------- |
| [`docs/arc42/01-introduction-and-goals/overview.md`](https://github.com/uyuutosa/tensor/blob/develop/docs/arc42/01-introduction-and-goals/overview.md) | Goals (G-1..G-7), quality goals, success criteria. |
| [`docs/arc42/04-solution-strategy/strategy.md`](https://github.com/uyuutosa/tensor/blob/develop/docs/arc42/04-solution-strategy/strategy.md) | Top-7 design decisions and how they hang together. |
| [`docs/arc42/05-building-blocks/overview.md`](https://github.com/uyuutosa/tensor/blob/develop/docs/arc42/05-building-blocks/overview.md) | Container decomposition with Hexagonal classification. |
| [`docs/arc42/09-decisions/`](https://github.com/uyuutosa/tensor/tree/develop/docs/arc42/09-decisions) | All eleven foundational ADRs. |

## The Hexagonal "lite" layering

Per [ADR-0009](https://github.com/uyuutosa/tensor/blob/develop/docs/arc42/09-decisions/0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md):

```text
                ┌─────────────────────────────────────┐
   Driving      │                                     │
   adapters ──► │   tensor::core  +  tensor::autograd │ ◄── Driven adapters
   (input)      │       (the Domain hexagon)          │     (output)
                │                                     │
                └─────────────────────────────────────┘
```

- **Domain** (`tensor::core`, `tensor::autograd`) — named-axis algebra, expression graphs, autograd rules. No third-party dependencies; pure C++20.
- **Driving adapters** (`tensor::tex` with `_tex` UDL; planned LyX module; future Python bindings) — produce expression graphs the Domain consumes.
- **Driven adapters** (`tensor::core::backend::reference`, `tensor::core::backend::eigen`; planned `tensor::core::backend::webgpu`) — execute the Domain's lowered operations.

## The `KernelBackend` port

Per [ADR-0011](https://github.com/uyuutosa/tensor/blob/develop/docs/arc42/09-decisions/0011-kernel-backend-port-api.md), every adapter implements 15 methods covering element-wise binary/unary, broadcast variants, contraction, reduction, and the autograd `unbroadcast` helper. Selecting an adapter is a CMake configure-time choice:

```bash
cmake --preset=default                                     # reference (default)
cmake --preset=default -DTENSOR_KERNEL_BACKEND=eigen       # Eigen SIMD + GEMM
cmake --preset=default -DTENSOR_KERNEL_BACKEND=webgpu      # WebGPU (Phase 3, planned)
```

The same Domain code, the same tests, the same notebooks — different runtime characteristics. See `08_swappable-backends.ipynb` for the live demonstration.
