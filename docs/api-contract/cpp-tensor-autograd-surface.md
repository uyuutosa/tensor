---
status: Stable
owner: tensor
last-reviewed: 2026-05-14
---

# C++ public surface — `tensor::autograd`

> The public C++ symbols of the autograd module. Complete HOW: [`../detailed-design/tensor-autograd.md`](../detailed-design/tensor-autograd.md). The Python mirror: [`./python-public-surface.md` §3](./python-public-surface.md).

## 1. Types

| Type                                    | Header                                            | Purpose                                                              |
| --------------------------------------- | ------------------------------------------------- | -------------------------------------------------------------------- |
| `tensor::autograd::Variable<T, N>`      | `include/tensor/autograd/variable.hpp`            | Compile-time-rank autograd variable wrapping `Tensor<T, N>`.        |
| `tensor::autograd::DynamicVariable<T>`  | `include/tensor/autograd/dynamic_variable.hpp`    | Runtime-rank autograd variable wrapping `DynamicTensor<T>`.         |
| `tensor::autograd::Tape`                | `include/tensor/autograd/tape.hpp`                | Thread-local tape of registered backward closures.                   |
| `tensor::autograd::GradAccum<T, N>`     | `include/tensor/autograd/grad_accum.hpp`          | Per-variable gradient accumulator.                                   |

## 2. Activations (closed-form gradients)

| Function                                                       | Introduced | Header                                                    |
| -------------------------------------------------------------- | ---------- | --------------------------------------------------------- |
| `exp(v)`, `log(v)`, `relu(v)`, `neg(v)`                          | P2.M2      | `include/tensor/autograd/activations.hpp`                  |
| `sin(v)`, `cos(v)`, `sqrt(v)`                                   | Bundle B (PR #109) | same                                              |

Each is overloaded for both `Variable<T, N>` and `DynamicVariable<T>`.

## 3. Algebraic operations

| Operation                                  | Introduced | Header                                                    |
| ------------------------------------------ | ---------- | --------------------------------------------------------- |
| `operator+ / - / *`                        | P2.M1      | `include/tensor/autograd/ops.hpp`                          |
| `operator/`                                 | Bundle B    | `include/tensor/autograd/broadcast_ops.hpp`                |
| `dot(a, b)`                                 | P2.M4       | `include/tensor/autograd/contract_ops.hpp`                 |
| `sum_all(v)`                                | P2.M1       | `include/tensor/autograd/reduce.hpp`                      |
| `reduce_along_label(v, label)`              | Bundle B    | `include/tensor/autograd/reduce_ops.hpp`                  |

All operations are broadcast-aware; the backward path uses `unbroadcast` from `tensor::core` to project mismatched-shape gradients back onto the original operand shape.

## 4. Training-loop helpers

| Helper                            | Header                                          | Purpose                                                                  |
| --------------------------------- | ----------------------------------------------- | ------------------------------------------------------------------------ |
| `backward(loss)`                  | `include/tensor/autograd/backward.hpp`          | Reverse-mode walk from a scalar loss.                                    |
| `zero_grad(v)`                    | `include/tensor/autograd/grad_accum.hpp`        | Reset gradient accumulator(s).                                            |
| `sgd_update(v, lr)`               | `include/tensor/autograd/sgd_update.hpp`         | One vanilla SGD step; returns a new `DynamicTensor`.                     |
| `gradient_check(fn, vars)`         | `include/tensor/autograd/gradient_check.hpp`    | Finite-difference vs analytical gradient.                                |

## 5. Idiomatic usage

```cpp
#include <tensor/autograd/autograd.hpp>       // umbrella include

using namespace tensor::core;
using namespace tensor::autograd;

DynamicVariable<double> x(/* DynamicTensor input */, /*requires_grad=*/true);
DynamicVariable<double> y(/* target */);

auto pred = relu(x);
auto loss = sum_all((pred - y) * (pred - y));
backward(loss);

// x.grad() now populated; one SGD step:
auto new_x = sgd_update(x, 0.1);
```

## 6. Cross-references

- Detailed design: [`../detailed-design/tensor-autograd.md`](../detailed-design/tensor-autograd.md).
- Python mirror: [`./python-public-surface.md` §3](./python-public-surface.md).
- Glossary: [`../arc42/12-glossary/overview.md`](../arc42/12-glossary/overview.md) — `Variable`, `Tape`, `Gradient check`.
- Anchor ADRs: [ADR-0007](../arc42/09-decisions/0007-adopt-autograd-as-first-class-subsystem.md), [ADR-0009](../arc42/09-decisions/0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md).
- Reference (human-readable): [`../user-manual/reference/cpp-namespace-overview.md`](../user-manual/reference/cpp-namespace-overview.md) §`tensor::autograd`.
