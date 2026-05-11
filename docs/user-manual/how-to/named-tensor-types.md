---
status: Stable
owner: tensor
last-reviewed: 2026-05-11
---

# Which named-tensor type should I use?

`tensor` ships three named-axis tensor types. They map onto a 2-axis design space:

| Type                                          | Rank known at         | Axis labels known at  |
| --------------------------------------------- | --------------------- | --------------------- |
| `tensor::core::Tensor<T, N>` and `Variable<T, N>` | compile time          | runtime (string)      |
| `tensor::core::DynamicTensor<T>` and `DynamicVariable<T>` | runtime           | runtime (string)      |
| `tensor::core::TypedTensor<T, "i", "j", …>`   | compile time          | compile time (NTTP)   |

Each row corresponds to a real trade-off; the three exist because no single point dominates the others. This how-to walks you through choosing one for a given use case.

---

## Decision tree

```text
Q1: Do you know the shape ranks at compile time?
  no  → use DynamicTensor / DynamicVariable.
  yes ↓

Q2: Do you want the compiler to catch axis-label mismatches?
  no  → use Tensor<T, N> / Variable<T, N>.
  yes → use TypedTensor<T, "i", "j", …>.
```

That's the whole rule. The rest of this document explains why.

---

## `Tensor<T, N>` / `Variable<T, N>` — static rank, runtime labels

**Use when**: you know your tensor's rank at compile time (a vector is rank-1, a matrix rank-2, etc.) but the *labels* of the axes might come from runtime sources — a config file, a tutorial cell, a parser output.

```cpp
#include <tensor/core/core.hpp>
using tensor::core::Axis;
using tensor::core::Shape;
using tensor::core::Tensor;

Tensor<double, 1> a(Shape<1>{Axis{"i", 5}}, {1, 2, 3, 4, 5});
Tensor<double, 1> b(Shape<1>{Axis{"j", 5}}, {1, 2, 3, 4, 5});
auto c = a + b;   // returns DynamicTensor<double>, axes (i: 5, j: 5)
```

**Trade-offs**.
- Static rank lets the compiler size buffers, eliminate one indirection vs runtime-rank.
- Operators that produce a result of *uncertain* rank (Einstein-style broadcast like `a_i + b_j`) drop into `DynamicTensor` automatically. You'll see this in the return type of `operator+` etc.
- Mismatched axis labels produce a runtime exception, not a compile error.

**Typical readers / users**: Phase 1 and Phase 2 tutorial notebooks. The 2016 README examples are reproduced on this API in `tutorials/00_intro.ipynb`.

---

## `DynamicTensor<T>` / `DynamicVariable<T>` — runtime rank, runtime labels

**Use when**: rank depends on runtime data — most commonly, the result of a broadcast operation; or when feeding the `tensor::tex::Evaluator` since AST evaluation produces tensors whose rank depends on the expression.

```cpp
#include <tensor/core/core.hpp>
#include <tensor/tex/tex.hpp>
using namespace tensor::tex::literals;

tensor::tex::Evaluator<double> eval;
eval.bind("a", tensor::core::DynamicTensor<double>(
    tensor::core::DynamicShape{tensor::core::Axis{"i", 5}}, {1, 2, 3, 4, 5}));
eval.bind("b", tensor::core::DynamicTensor<double>(
    tensor::core::DynamicShape{tensor::core::Axis{"j", 5}}, {1, 2, 3, 4, 5}));
auto outer = eval.evaluate(R"(a_i + b_j)"_tex);
// outer is DynamicTensor<double>, rank 2.
```

**Trade-offs**.
- Extra heap allocation for the runtime shape; small per-tensor overhead.
- Used as the result type of broadcast and contraction operators on the other two types, so you'll receive a `DynamicTensor` from many expressions even if your inputs were static-rank.
- Convertible to `Tensor<T, N>` via `to_static<N>()` (throws on rank mismatch).

**Typical readers / users**: anyone consuming the `_tex` evaluator, any training-loop iteration that rebuilds parameters per step (see `tutorials/07_mlp-on-toy.ipynb`).

---

## `TypedTensor<T, "i", "j", …>` — static rank, compile-time labels

**Use when**: you want the compiler to refuse mismatched axis labels at compile time. This is the most production-flavoured of the three types — applications that wrap `tensor` and want bug-resistance at compile time should reach for `TypedTensor`.

```cpp
#include <tensor/core/typed_tensor.hpp>
using tensor::core::TypedTensor;

TypedTensor<double, "i", "j"> a({2, 3}, {1, 2, 3, 4, 5, 6});
TypedTensor<double, "i", "j"> b({2, 3}, {10, 20, 30, 40, 50, 60});
auto c = a + b;     // ✅ same labels → compiles.

TypedTensor<double, "i"> x({3}, {1, 2, 3});
TypedTensor<double, "j"> y({3}, {4, 5, 6});
auto r = x + y;     // ❌ static_assert: "axis labels must match at compile time".
```

**Trade-offs**.
- The label literals must be string literals (NTTP); they cannot come from a runtime variable.
- Operators only work on same-label pairs (broadcast across heterogeneous label packs is not implemented). For broadcast, drop into `DynamicTensor` via `.to_dynamic()`.
- The static-assert error message points at `to_dynamic()` as the escape hatch.

**Typical readers / users**: code that wants compile-time bug detection on a known graph; the `08_swappable-backends.ipynb` tutorial.

---

## Conversions between the three

```cpp
// static-rank Variable → DynamicVariable (implicit)
Variable<T, N> v{...};
DynamicVariable<T> dv = v;

// DynamicVariable → static-rank Variable (explicit; throws if rank ≠ N)
auto v_back = dv.to_static<N>();

// TypedTensor → DynamicTensor (explicit; gives up compile-time labels)
TypedTensor<T, "i", "j"> t({2, 3}, {...});
DynamicTensor<T> dt = t.to_dynamic();
```

The reverse — `DynamicTensor` → `TypedTensor` — requires the user to supply the label list, and there is no built-in helper. Runtime tensors meet compile-time-labelled tensors at construction time, never via inference.

---

## When in doubt

Use `DynamicTensor` / `DynamicVariable`. It's the most permissive type, and the cost is small heap overhead per tensor. Drop into `Tensor<T, N>` for known-rank kernels, `TypedTensor` when you want the compiler's help.

---

## References

- [ADR-0004 — hybrid named-axis API](../../arc42/09-decisions/0004-adopt-hybrid-named-axis-api.md): the design rationale that justified having both runtime and compile-time paths.
- [`include/tensor/core/typed_tensor.hpp`](../../../include/tensor/core/typed_tensor.hpp): `TypedTensor` source.
- [`include/tensor/core/label_tag.hpp`](../../../include/tensor/core/label_tag.hpp): NTTP `FixedString` and `_ax` UDL primitives.
- [`tutorials/05_autograd-from-scratch.ipynb`](../../../tutorials/05_autograd-from-scratch.ipynb): the `_ax` UDL in action through the autograd tape.
