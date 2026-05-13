---
status: Stable
owner: tensor
last-reviewed: 2026-05-14
---

# C++ public surface — `tensor::tex`

> The public C++ symbols of the `_tex` LaTeX-subset DSL. Complete HOW: [`../detailed-design/tensor-tex.md`](../detailed-design/tensor-tex.md).

## 1. Types

| Type                                  | Header                                  | Purpose                                                                |
| ------------------------------------- | --------------------------------------- | ---------------------------------------------------------------------- |
| `tensor::tex::Expression`             | `include/tensor/tex/expression.hpp`      | The parsed AST (opaque to user; visitable internally).                  |
| `tensor::tex::Evaluator<T>`           | `include/tensor/tex/evaluator.hpp`       | Binds named tensors to AST `IndexedVar` leaves; `.evaluate(expr)` → `DynamicTensor<T>`. |

## 2. Functions

| Function                              | Header                                          | Purpose                                                                |
| ------------------------------------- | ----------------------------------------------- | ---------------------------------------------------------------------- |
| `tensor::tex::parse(s)`               | `include/tensor/tex/parse.hpp`                   | Runtime parser; same grammar as the compile-time UDL.                  |
| `tensor::tex::to_latex(expr)`         | `include/tensor/tex/to_latex.hpp`                | Canonical LaTeX renderer; round-trips: `parse(to_latex(e)) == e`.       |

## 3. UDL

| UDL                  | Header                          | Expansion                                                                   |
| -------------------- | ------------------------------- | --------------------------------------------------------------------------- |
| `operator""_tex`     | `include/tensor/tex/tex.hpp`     | `R"(c_{ij} = a_i b_j)"_tex` → `Expression`. Parses **at compile time**.     |

The `_tex` UDL is the headline G-3 feature: a LaTeX-subset string parses inside the compiler to the same AST that `parse(s)` produces at runtime.

## 4. Supported LaTeX subset

| Production       | Examples                                  | AST node                          |
| ---------------- | ----------------------------------------- | --------------------------------- |
| Indexed variable | `a`, `a_i`, `c_{ij}`                       | `IndexedVar`                       |
| Binary operator  | `a + b`, `a - b`, `a b` (juxtaposition = mul), `a / b` | `BinOp` |
| Sum              | `\sum_i {a_i b_i}`                         | `Sum`                              |
| Equation         | `c_{ij} = a_i + b_j` (LHS = shape annotation, RHS = expression) | `Equation`           |
| Group            | `{a_i + b_i}`                              | `Group` (evaluation-transparent)   |

Out-of-scope: `\prod`, `\delta`, `\nabla`, `\frac`, `\sin`, `\cos`, fractions with non-trivial denominators. Tracked as future work in [`../detailed-design/tensor-tex.md` §8](../detailed-design/tensor-tex.md).

## 5. Idiomatic usage

```cpp
#include <tensor/tex/tex.hpp>

using namespace tensor::core;
using namespace tensor::tex;

DynamicTensor<double> a({{"i", 5}}, {1, 2, 3, 4, 5});
DynamicTensor<double> b({{"j", 2}}, {10, 20});

auto expr = R"(c_{ij} = a_i b_j)"_tex;     // compile-time parse
Evaluator<double> ev;
ev.bind("a", a).bind("b", b);
auto c = ev.evaluate(expr);                 // 5×2 outer product
```

## 6. Cross-references

- Detailed design: [`../detailed-design/tensor-tex.md`](../detailed-design/tensor-tex.md).
- Python mirror: [`./python-public-surface.md` §4](./python-public-surface.md).
- Glossary: [`../arc42/12-glossary/overview.md`](../arc42/12-glossary/overview.md) — `_tex` UDL, Expression, Evaluator.
- Anchor ADRs: [ADR-0005](../arc42/09-decisions/0005-adopt-tex-lyx-as-authoring-surface.md), [ADR-0009](../arc42/09-decisions/0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md).
- The slogan essay: [`../user-manual/explanation/formula-is-the-program-essay.md`](../user-manual/explanation/formula-is-the-program-essay.md).
- LyX integration: [`lyx-export/README.md`](../../lyx-export/README.md).
