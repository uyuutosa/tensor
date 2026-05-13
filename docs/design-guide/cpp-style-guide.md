---
status: Stable
owner: tensor
last-reviewed: 2026-05-14
---

# C++ style guide — project-specific conventions

> The C++20 baseline + modern idioms inherited from the C++ Core Guidelines plus the project-specific conventions that go *beyond* the baseline. The hard rules in [`./architectural-discipline.md`](./architectural-discipline.md) are about dependency direction; this file is about everything else.

## 1. Baseline

- **C++20 is the minimum** ([TC-1](../arc42/02-architecture-constraints/overview.md), [ADR-0002](../arc42/09-decisions/0002-rewrite-on-cpp20-baseline-with-mdspan-interop.md)). C++23 features allowed under `__cpp_*` feature-test gating.
- **C++ Core Guidelines** are the implicit baseline. Anything CG-recommended applies unless contradicted below.
- **clang-format + clang-tidy** are CI-enforced. The `.clang-format` and `.clang-tidy` files at repo root are authoritative.

## 2. Naming

| Element                 | Convention                                       | Example                                  |
| ----------------------- | ------------------------------------------------ | ---------------------------------------- |
| Namespace               | lower_snake_case, plural where it groups types   | `tensor::core`, `tensor::core::concepts` |
| Class / struct          | UpperCamelCase                                    | `DynamicTensor`, `KernelBackend`         |
| Function / method        | lower_snake_case                                  | `broadcast_shapes`, `zero_grad`          |
| Variable                | lower_snake_case                                  | `axis_count`, `result_shape`             |
| Template parameter       | UpperCamelCase                                    | `template <typename Backend>`             |
| NTTP template parameter  | descriptive (not single-letter)                   | `template <FixedString Label>`            |
| Concept                  | UpperCamelCase (matches class style)              | `KernelBackend`, `AxisLike`              |
| File (header)            | lower_snake_case                                  | `dynamic_tensor.hpp`, `label_tag.hpp`    |
| Compile-time constant    | `k`-prefixed UpperCamelCase                       | `kGemmF32` (WGSL kernel sources)          |
| Macro (unavoidable)      | UPPER_SNAKE_CASE with `TENSOR_` prefix            | `TENSOR_HAS_EIGEN`                       |
| Tests                    | `test_<module>.cpp` mirroring the header           | `test_dynamic_tensor.cpp` mirrors `dynamic_tensor.hpp` |

**No abbreviations** in public names (per arc42 §8 §5). `gpu`, not `g`; `autograd`, not `ag`; `KernelBackend`, not `KBackend`. Single-letter template parameters (`T`, `N`) are accepted because they match math-literature notation.

## 3. Headers

- **Header-only.** No `.cpp` source files in `include/`. Bodies inline; `inline` keyword optional but recommended for non-template functions.
- **Include order**:
  1. Corresponding `.hpp` (if this is a test/bench file).
  2. C++ standard library `<...>`.
  3. Third-party `<...>` (`<fmt/format.h>`, `<Eigen/Dense>`, `<webgpu/webgpu_cpp.h>`).
  4. Project `<tensor/...>`.
  Each group separated by a blank line; `clang-format` enforces.
- **Forward-declarations** are preferred over includes when the header only mentions a type.
- **No transitive include reliance** — every header's `#include`s explicitly cover every type it uses.

## 4. Error handling

- **No exceptions for control flow.** `std::invalid_argument` only at constructor / API boundary on guard violations (e.g. "value count mismatches shape product").
- **`static_assert`** for compile-time contracts. The diagnostic text *must* include the fix per arc42 §8 §5.2.
- **No `assert(...)`** in headers — replace with a `static_assert` or a throw at the API boundary.
- **No exception specifications** (`noexcept` is fine; `throw(...)` is not — deprecated since C++17).

## 5. Memory + ownership

- **No raw `new`** in non-test code. `std::make_shared` / `std::make_unique` where dynamic allocation is genuinely needed.
- **Value semantics by default.** `DynamicTensor<T>` is a value type; `Tape` is a thread-local singleton; nothing in the public surface returns a pointer.
- **`std::span` / `std::mdspan` for views** over data the function does not own.
- **No `friend` keyword** unless absolutely needed for operator implementation — public surface should be all the surface there is.

## 6. Templates

- **Concepts > SFINAE.** Use C++20 concepts for constraints; `enable_if_t` only when concepts can't express the constraint (rare in this codebase).
- **`template <typename T>` over `template <class T>`.** Consistency.
- **NTTPs for compile-time labels.** Per [ADR-0004](../arc42/09-decisions/0004-adopt-hybrid-named-axis-api.md), `FixedString` is the NTTP carrier for `_ax` UDL and `TypedTensor` labels.
- **CTAD where it reads naturally.** `TypedTensor t{{2, 3}, {1, 2, 3, 4, 5, 6}}` deduces the rank and types. But explicit `<T>` is preferred when CTAD's deduction guides would surprise the reader.

## 7. Modern features the project leans on

| C++20 feature              | Used for                                                                        | Reference                                                                |
| -------------------------- | ------------------------------------------------------------------------------- | ------------------------------------------------------------------------ |
| `<concepts>`                | `KernelBackend`, `AxisLike`, `ShapeLike`                                          | `include/tensor/core/concepts.hpp`                                       |
| Class-type NTTP             | `FixedString`, `LabelTag`, `TypedTensor` labels                                   | `include/tensor/core/{fixed_string,label_tag,typed_tensor}.hpp`           |
| `consteval` / `constexpr`   | `FixedString` ops; UDL parsing of `R"(...)"_tex`                                  | `include/tensor/tex/parse.hpp`                                            |
| `<ranges>`                  | Where iteration semantics improve readability                                     | (sparingly — only when it reads better than `for` loops)                  |
| `std::span`                  | Read-only views over `std::vector<T>` data                                        | broadcast / contract helpers                                              |
| `std::mdspan` (polyfill)    | Multi-dimensional view; `tensor::core::mdspan_interop::mdview`                    | `include/tensor/core/mdspan_interop.hpp`                                  |
| Coroutines                  | NOT USED. Adds compile-time cost without payoff for educational tensor code.    | —                                                                        |

## 8. Comments

- **Doxygen-style for public surface.** Triple-slash `///` or `/** */` block above the declaration; describe what + invariants. Body comments only where the *why* is non-obvious (per the project-wide commenting rule).
- **No history comments.** "// changed by X in PR #Y" — the git log is the source of truth.
- **TODOs include an owner + a tracking pointer.** `// TODO(uyuutosa): see ADR-NNNN for the resolution path`.

## 9. Tests

- **doctest** is the framework. `TEST_CASE("<one sentence describing the property>")`.
- **One file per concern.** `tests/test_<module>_<feature>.cpp`. Listed in [`../arc42/05-building-blocks/overview.md` §"Container ownership table"](../arc42/05-building-blocks/overview.md).
- **Cross-backend tests use `Approx`** with the QO-1 tolerances (`1e-9` for `double`, `1e-5` for `float`).
- **Test names are sentences** that the failure log reads cleanly: `TEST_CASE("DynamicTensor + DynamicTensor with disjoint axes produces rank-2 broadcast")`.

## 10. What NOT to do

- **Don't introduce a new dependency** without an ADR. The `[TC-9](../arc42/02-architecture-constraints/overview.md)` vendoring discipline applies even to header-only deps.
- **Don't widen `KernelBackend`** without an ADR. The 15-method count is in [ADR-0011](../arc42/09-decisions/0011-kernel-backend-port-api.md); adding a 16th is a port-API change.
- **Don't reach into adapter internals** from the Domain. Domain uses only the port (concepts); adapter internals are private to the adapter.
- **Don't optimise prematurely.** Performance ranks below clarity per [G-4 quality goal](../arc42/01-introduction-and-goals/overview.md).

## 11. Cross-references

- C++ Core Guidelines: <https://isocpp.github.io/CppCoreGuidelines/>
- The architectural rule that this style serves: [`./architectural-discipline.md`](./architectural-discipline.md).
- Public symbol contracts: [`../api-contract/cpp-tensor-namespace-summary.md`](../api-contract/cpp-tensor-namespace-summary.md).
- Detailed designs that illustrate the conventions: [`../detailed-design/`](../detailed-design/).
- Glossary entries for the vocabulary: [`../arc42/12-glossary/overview.md`](../arc42/12-glossary/overview.md).
