---
status: Draft
owner: tensor
last-reviewed: 2026-05-11
---

# `tensor` — Runtime View (arc42 §6)

| Metadata      | Value                                                          |
| ------------- | -------------------------------------------------------------- |
| Status        | Draft                                                          |
| Type          | arc42 §6 (Runtime View)                                        |
| Owner         | uyuutosa                                                       |
| Last Updated  | 2026-05-11                                                     |

> Per arc42 §6: this file shows **how the building blocks interact at runtime** for the project's most important scenarios. Not every scenario lives here — only those that exercise multiple containers in a way that benefits from being walked through.

## Scenario 1 — `c = a + b` via the runtime broadcast path

**Trigger.** A learner writes `auto c = a + b;` in a tutorial cell, where `a` and `b` are `DynamicTensor<double>` instances with axis labels `i` and `j` respectively.

**Walk-through.**

```text
1. operator+(a, b)                                 (tensor::core::ops)
2.  → broadcast_shapes(a.shape(), b.shape())       (tensor::core::broadcast)
3.    returns BroadcastPlan{result, a_source, b_source}
4.  → reference::Backend::broadcast_add(a, b, plan)  (selected adapter — KernelBackend port)
5.    iterates result-flat-index 0..result_size:
6.       a_flat = project_index(i, plan.a_source)
7.       b_flat = project_index(i, plan.b_source)
8.       out[i] = a[a_flat] + b[b_flat]
9. returns DynamicTensor<double>{plan.result, out}
```

**Why this is interesting.** The Domain (`tensor::core::ops` + `broadcast.hpp`) computes the *plan* without knowing which backend will execute. The selected adapter (`reference::Backend` or `eigen::Backend` or `webgpu::Backend`) consumes the plan and produces the output. Swapping `-DTENSOR_KERNEL_BACKEND=eigen` at configure time changes step 4 without touching steps 1–3.

## Scenario 2 — `_tex` formula evaluated end-to-end

**Trigger.** A user writes `auto expr = R"(c_{ij} = a_i b_j)"_tex; evaluator.bind("a", a).bind("b", b); auto c = evaluator.evaluate(expr);`.

**Walk-through.**

```text
1. operator""_tex(string_view src)                 (tensor::tex::operator""_tex)
2.  → tensor::tex::parse(src)
3.    produces Expression AST: Equation{LHS=IndexedVar{"c", {"i","j"}},
4.                                       RHS=BinOp{*, IndexedVar{"a", {"i"}},
5.                                                    IndexedVar{"b", {"j"}}}}
6. evaluator.bind("a", a)                          (records DynamicTensor binding)
7. evaluator.bind("b", b)
8. evaluator.evaluate(expr)                        (tensor::tex::Evaluator)
9.  → traverse RHS:
10.     visit BinOp{*}: → a_i * b_j
11.       → tensor::core::broadcast_shapes(a_i, b_j) → outer product plan
12.       → reference::Backend::broadcast_mul(a, b, plan)
13. returns DynamicTensor<double> with shape {Axis{"i", N}, Axis{"j", M}}
```

**Why this is interesting.** The TeX surface (DrivingAdapter) consumes `tensor::core` types but never the other way around. The same operator+ implementation runs whether the call originates from `operator+(a, b)` (Scenario 1) or from `parse + evaluate` (Scenario 2). This is the [§4 G-3 goal](../01-introduction-and-goals/overview.md): "the formula is the program".

## Scenario 3 — Autograd backward pass

**Trigger.** A user computes `auto loss = (W * x + b - y).sum_all(); loss.backward();`.

**Walk-through.**

```text
1. Forward pass — every operator pushes a closure onto tensor::autograd::Tape::current():
   - W.mul(x)        → registers closure that propagates grad_out → (grad_W += grad_out * x.T,
                                                                     grad_x += W.T * grad_out)
   - (W*x).add(b)    → registers closure that propagates grad_out → (grad_(W*x) += grad_out,
                                                                     grad_b += unbroadcast(grad_out, …))
   - (...).sub(y)    → similar
   - sum_all()       → registers closure that propagates scalar grad → broadcast back to result shape

2. loss.backward(/* grad = 1.0 */):
   - Walks the tape in reverse.
   - Invokes each registered closure with the appropriate grad_out.
   - Each closure updates the GradAccum for its input Variables.

3. After backward, W.grad(), x.grad(), b.grad() hold accumulated gradients.
```

**Why this is interesting.** The Tape is `thread_local std::vector<std::function<void()>>` — no special infrastructure beyond the standard library. Gradients flow through the *same* port the forward pass used: `unbroadcast` lives in `tensor::core`, called both by the forward broadcast and by the backward closure to undo the broadcast. Per [ADR-0009](../09-decisions/0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md), autograd is the Domain *extended*, not a sibling.

## Scenario 4 — Backend swap at configure time

**Trigger.** A learner runs `cmake --preset=default -DTENSOR_KERNEL_BACKEND=eigen` after the default `reference` build.

**Walk-through.**

```text
1. CMakeLists.txt sees the cache variable change.
2. TENSOR_KERNEL_BACKEND=eigen triggers:
   - find_package(Eigen3 CONFIG REQUIRED)
   - target_link_libraries(tensor INTERFACE Eigen3::Eigen)
   - target_compile_definitions(tensor INTERFACE TENSOR_HAS_EIGEN=1)
3. Headers under include/tensor/core/backend/eigen.hpp are now visible.
4. The user's source includes #include <tensor/core/backend/eigen.hpp> and instantiates:
   tensor::core::backend::eigen::Backend backend;
5. Compile-time concept check: static_assert(KernelBackend<eigen::Backend>) — confirms the
   adapter implements all 15 methods.
6. Runtime: backend.add(a, b) uses Eigen::Map<Eigen::ArrayXd> + element-wise + as a SIMD path
   for double; falls through to reference for other types.
```

**Why this is interesting.** Step 5 — `static_assert(KernelBackend<eigen::Backend>)` — is the compile-time guarantee that the swap is safe. If a future contributor adds a `Backend` that fails to implement one of the 15 methods, the build fails with the constraint that was violated; the cross-validation test then catches numerical disagreements within tolerance.

## Why these four scenarios

The §1 §G-1..§G-6 goals each show up here:

- Scenario 1 → G-1 (named axes), G-2 (modern C++).
- Scenario 2 → G-3 (*the formula is the program*).
- Scenario 3 → G-4 (end-to-end teaching arc), G-2.
- Scenario 4 → G-1 (Hexagonal payoff), G-5 (zero-friction install / configure).

## Cross-references

- §5 building blocks the scenarios traverse: [`../05-building-blocks/overview.md`](../05-building-blocks/overview.md)
- detailed-design backing each scenario: [`../../detailed-design/tensor-core.md`](../../detailed-design/tensor-core.md)
- ADRs that shaped the scenarios' design: [ADR-0007](../09-decisions/0007-adopt-autograd-as-first-class-subsystem.md), [ADR-0009](../09-decisions/0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md), [ADR-0011](../09-decisions/0011-kernel-backend-port-api.md).
