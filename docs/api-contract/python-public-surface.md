---
status: Stable
owner: tensor
last-reviewed: 2026-05-13
---

# Python public surface — `tensor-named-axis` (`import tensor`)

> The stable public Python surface of the `tensor` SDK, with the version each symbol was introduced. Pinned by `python/tests/test_smoke.py::test_public_surface_is_minimal` so unintended additions and accidental removals both fail CI.

## 1. Stability promise

Pre-`1.0.0`, **no ABI / removal-cost guarantees** ([TC-8](../arc42/02-architecture-constraints/overview.md) + [ADR-0010](../arc42/09-decisions/0010-refine-positioning-to-educational-first-production-capable.md)). What this contract documents:

- The minimal set the test suite asserts. Anything not in this list may be added, renamed, or removed without prior notice. Anything *in* this list won't change without a CHANGELOG entry under the "Changed" or "Removed" sections of the release that drops it.
- Numerical agreement guarantee: every Python entry point produces results that agree element-wise with the C++ canonical answer within `1e-12` for `double` / `1e-5` for `float`. See [arc42 §10 QO-4](../arc42/10-quality/overview.md).
- ABI surface: the underlying `_tensor_native.so` extension is *not* covered by Python's stable ABI; the cibuildwheel matrix ships per-CPython-minor wheels (3.9–3.13) explicitly.

## 2. Top-level (`tensor.*`)

| Symbol                      | Kind        | Introduced | Description                                                                  | Source DD                                                            |
| --------------------------- | ----------- | ---------- | ---------------------------------------------------------------------------- | -------------------------------------------------------------------- |
| `tensor.__version__`        | `str`       | `0.1.0+dev` (PR #96) | The installed wheel's version. Matches `pyproject.toml::version`. | — |
| `tensor.hello() -> str`     | function    | `0.1.0+dev` (PR #96) | Returns `"hello from tensor::core"`. Smoke / "did the install work?". | — |
| `tensor.Axis`               | class       | `0.1.0+dev` (PR #100) | A `(label: str, extent: int)` pair. | [`tensor-core.md`](../detailed-design/tensor-core.md) |
| `tensor.DynamicShape`       | class       | `0.1.0+dev` (PR #100) | Ordered list of `Axis`. `.rank()`, `.total()`, `.axes()`. | [`tensor-core.md`](../detailed-design/tensor-core.md) |
| `tensor.DynamicTensor`      | class       | `0.1.0+dev` (PR #100) | Rank-erased dense tensor over `double`. Einstein-style broadcast on `+ - * /`. | [`tensor-core.md`](../detailed-design/tensor-core.md) |
| `tensor.DynamicTensorF32`   | class       | `0.1.0+dev` (PR #100) | Same as `DynamicTensor` over `float`. | [`tensor-core.md`](../detailed-design/tensor-core.md) |
| `tensor.contract`           | function    | `0.1.0+dev` (PR #102) | Einstein-sum named-axis contraction; element-wise-equal to `np.einsum`. | [`tensor-core.md`](../detailed-design/tensor-core.md) |
| `tensor.from_numpy`         | function    | `0.1.0+dev` (PR #102) | `(arr: np.ndarray, labels: list[str]) -> DynamicTensor`. Copy semantics. | [`python-sdk-binding-surface.md`](../detailed-design/python-sdk-binding-surface.md) §4 |
| `tensor.autograd`           | submodule   | `0.1.0+dev` (PR #103/#104) | See §3 below. | [`tensor-autograd.md`](../detailed-design/tensor-autograd.md) |
| `tensor.tex`                | submodule   | `0.1.0+dev` (PR #105/#106) | See §4 below. | [`tensor-tex.md`](../detailed-design/tensor-tex.md) |

**Planned (Phase 6.5)**:

| Symbol                            | Kind     | Target version | Description                                                                                                                  |
| --------------------------------- | -------- | -------------- | ---------------------------------------------------------------------------------------------------------------------------- |
| `tensor.set_backend(name)`        | function | `0.3.0`        | Selects between installed backends (`reference` / `eigen` / `webgpu`). Raises `RuntimeError` with install instructions when the requested backend isn't installed. |
| `tensor.current_backend() -> str` | function | `0.3.0`        | Returns the currently active backend name.                                                                                   |
| `tensor.list_available_backends() -> list[str]` | function | `0.3.0` | Returns the installed backends.                                                                                              |

## 3. `tensor.autograd`

| Symbol                                              | Kind     | Introduced | Description                                                                  |
| --------------------------------------------------- | -------- | ---------- | ---------------------------------------------------------------------------- |
| `tensor.autograd.DynamicVariable`                   | class    | `0.1.0+dev` (PR #103) | Wraps `DynamicTensor` with optional gradient tracking.                       |
| `tensor.autograd.DynamicVariableF32`                | class    | `0.1.0+dev` (PR #103) | Same, over `float`.                                                          |
| `DynamicVariable.value` / `.grad` / `.requires_grad` | properties | `0.1.0+dev` (PR #103) | Accessors.                                                                   |
| `DynamicVariable.zero_grad()` / `.seed_grad()`      | methods   | `0.1.0+dev` (PR #103) | Tape lifecycle.                                                              |
| `DynamicVariable.__add__/__sub__/__mul__/__truediv__` | operators | `0.1.0+dev` (PR #103 + Bundle B / PR #109) | Element-wise arithmetic with autograd registration.                          |
| `tensor.autograd.exp` / `log` / `relu` / `neg`      | functions | `0.1.0+dev` (PR #103) | Closed-form-gradient activations.                                            |
| `tensor.autograd.sin` / `cos` / `sqrt`              | functions | `0.1.0+dev` (Bundle B / PR #109) | Trigonometric / sqrt activations with closed-form gradients.                 |
| `tensor.autograd.dot(a, b)`                         | function | `0.1.0+dev` (PR #103) | Named-axis Einstein-sum contraction with autograd.                           |
| `tensor.autograd.sum_all(v)`                        | function | `0.1.0+dev` (PR #103) | Collapses to a scalar `_ScalarVariable`.                                     |
| `tensor.autograd.reduce_along_label(v, label)`      | function | `0.1.0+dev` (Bundle B / PR #109) | Autograd-aware single-axis sum.                                              |
| `tensor.autograd.backward(loss)`                    | function | `0.1.0+dev` (PR #103) | Kicks off the reverse-mode tape walk from a scalar loss.                     |
| `tensor.autograd.sgd_update(v, lr)`                 | function | `0.1.0+dev` (PR #103) | Returns a fresh `DynamicTensor` with one SGD step applied.                   |
| `tensor.autograd.zero_grad(v)`                      | function | `0.1.0+dev` (PR #103) | Free-function form of `v.zero_grad()`.                                       |

## 4. `tensor.tex`

| Symbol                              | Kind     | Introduced            | Description                                                          |
| ----------------------------------- | -------- | --------------------- | -------------------------------------------------------------------- |
| `tensor.tex.parse(s) -> Expression` | function | `0.1.0+dev` (PR #105) | Parses a LaTeX-subset string into an `Expression` AST.               |
| `tensor.tex.to_latex(expr) -> str`  | function | `0.1.0+dev` (PR #105) | Renders an `Expression` AST back to canonical LaTeX.                 |
| `tensor.tex.Expression`             | class    | `0.1.0+dev` (PR #106) | Opaque AST; default-constructible per the M5 fix.                    |
| `tensor.tex.Evaluator`              | class    | `0.1.0+dev` (PR #105) | `.bind(name, tensor)` named tensors to AST leaves; `.evaluate(expr)` → `DynamicTensor`. |
| `tensor.tex.EvaluatorF32`           | class    | `0.1.0+dev` (PR #105) | Same, over `float`.                                                  |

## 5. What is NOT public

Symbols starting with `_` (e.g. `_tensor_native`, `_ScalarVariable`) are implementation details and may change between minor versions without notice. The `nb::module_::def` for these is intentionally `nb::module_local()` where applicable.

## 6. Verification

The smoke test `python/tests/test_smoke.py::test_public_surface_is_minimal` constructs the expected set from this contract's §2 and asserts it equals `set(dir(tensor)) - {dunders}`. If you add a new public symbol:

1. Add it to the contract in this file.
2. Add it to the test's expected set.
3. Cite the introducing PR in §2/§3/§4 above.

If the test passes but this contract isn't updated, the introducing PR is incomplete.

## 7. Cross-references

- [`docs/detailed-design/python-sdk-binding-surface.md`](../detailed-design/python-sdk-binding-surface.md) — how the boundary patterns work; refer here when adding new public symbols.
- [`tensor-core.md`](../detailed-design/tensor-core.md), [`tensor-autograd.md`](../detailed-design/tensor-autograd.md), [`tensor-tex.md`](../detailed-design/tensor-tex.md) — the C++ Domain modules these symbols delegate into.
- [`ADR-0018`](../arc42/09-decisions/0018-phase-6-python-sdk-entry-via-nanobind.md) — the entry decisions.
- [`ADR-0019`](../arc42/09-decisions/0019-phase-6-5-runtime-backend-selection-via-extras.md) — the upcoming `set_backend()` surface.
