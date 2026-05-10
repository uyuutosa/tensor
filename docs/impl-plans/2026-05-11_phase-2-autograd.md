---
status: Active
owner: tensor
last-reviewed: 2026-05-11
---

# Phase 2 — `tensor` autograd build-out toward a small trainable MLP

| Metadata       | Value                                                          |
| -------------- | -------------------------------------------------------------- |
| Status         | Active                                                         |
| Type           | Layer B — implementation plan (dated, append-only)             |
| Owner          | uyuutosa                                                       |
| Predecessor    | [`2026-05-10_revival-phase-1.md`](./2026-05-10_revival-phase-1.md) (closed; see [`../reports/2026-05-11_phase-1-retrospective.md`](../reports/2026-05-11_phase-1-retrospective.md)) |
| Related        | [ADR-0007](../arc42/09-decisions/0007-adopt-autograd-as-first-class-subsystem.md), [ADR-0009](../arc42/09-decisions/0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md) |
| Target window  | 2026-05-11 → 2026-07-31                                        |
| Exit version   | `0.0.2-alpha`                                                  |

## TL;DR

Phase 2 takes the **autograd MVP** that just shipped (element-wise `+ - *` on same-shape `Variable<T, N>`, `sum_all` reduction, scalar `backward()`, numerical FD checker) and grows it into the minimal coherent subsystem capable of training a **small MLP on a toy dataset** in a Jupyter notebook. The exit deliverable is `tutorials/07_mlp-on-toy.ipynb` working end-to-end, with `tutorials/05_autograd-from-scratch.ipynb` as the pedagogical companion that derives every primitive.

This phase does **not** add WebGPU acceleration (Phase 3) or label-broadcast forward / mdspan polyfill / `_ax` NTTP (Phase 1.5 mop-up).

## Phase 2 milestones

Numbering continues with `P2.M*` to avoid collision with Phase 1's `M1..M6`.

### P2.M1 — Element-wise autograd MVP

> **Status: ✅ shipped via PR #10 (autograd MVP).**

Element-wise `+ - *` (same-shape only), `sum_all` reduction, `backward(scalar)` driver, `gradient_check` numerical FD harness. Tests cover primitive ops, chain-rule on mixed graphs, FD parity for `sum(x*x)` and `sum((x+x)*x)`.

### P2.M2 — Activation primitives (week 2)

> **Realises** the elementary callable surface a learner needs to build MLP forward passes.

- [ ] `exp(v)` element-wise; backward dL/dv = dL/dout · exp(v).
- [ ] `log(v)` element-wise (with v > 0 precondition documented); backward dL/dv = dL/dout / v.
- [ ] `relu(v)` element-wise; backward dL/dv = dL/dout · I(v > 0).
- [ ] `neg(v)` and `Variable<T, N> operator-(Variable<T, N> const&)` unary minus.
- [ ] Tests: gradient_check for each at non-trivial points.

**Exit**: the loss building blocks for cross-entropy / MSE / softmax can be composed from the primitives.

### P2.M3 — Broadcast-aware backward (weeks 3–4)

> **Realises** the autograd counterpart to `tensor::core`'s Einstein-style label broadcast.

- [ ] `DynamicVariable<T>` — runtime-rank analogue of `Variable<T, N>`. Wraps `DynamicTensor<T>` plus a `shared_ptr<GradAccum>` whose grad is a `DynamicTensor<T>` of the same shape.
- [ ] `Variable<T, M> + Variable<T, N> → DynamicVariable<T>` operator overloads.  Result shape follows `broadcast_shapes` from `tensor::core::broadcast`. Backward closure captures the `BroadcastPlan` and **reduces** dL/dout over axes that are absent on the input side.
- [ ] Same for `- * /`.
- [ ] Tests: gradient_check on cases that exercise both broadcast paths
  (`a_i + b_j` → rank 2 with reduce over j on dL/da and over i on dL/db;
  `a_{ij} + b_j` → reduce over i on dL/db).

**Exit**: the README four-arithmetic-operations example is differentiable.

### P2.M4 — Contraction (matmul as named-axis dot) (weeks 4–5)

> **Realises** the operator a real MLP layer needs.

- [ ] `dot(a, b)` — sum over the shared axis labels of `a` and `b`. For example
  `dot(W_{ij}, x_j)` produces `y_i = Σ_j W_{ij} x_j`.
- [ ] Backward: dL/dW_{ij} = dL/dy_i · x_j; dL/dx_j = Σ_i dL/dy_i · W_{ij}.
- [ ] Tests: gradient_check against analytical gradients on rank-2 × rank-1 and rank-2 × rank-2 cases.

**Exit**: a `Linear` layer can be expressed as `dot(W, x) + b`.

### P2.M5 — `tutorials/05_autograd-from-scratch.ipynb` (week 5)

> **Realises** ADR-0007's pedagogical promise.

- [ ] Notebook walks through:
  - Section 1 — the `Variable` wrapper and tape concept.
  - Section 2 — backward by hand on `sum(x*x)`; compare to `gradient_check`.
  - Section 3 — chain rule on `sum((a+b)*c)`.
  - Section 4 — activations (`exp`, `log`, `relu`) with derivations.
  - Section 5 — broadcast backward intuition (reduce over collapsed axes).
  - Section 6 — `dot` and the gradient of a linear layer.

**Exit**: notebook runs end-to-end on the xeus-cling kernel; outputs match `gradient_check` parity tests.

### P2.M6 — `tutorials/07_mlp-on-toy.ipynb` (weeks 6–8)

> **Realises** the Phase 2 capstone: a working small training loop.

- [ ] Build a 2-layer MLP (`y = ReLU(W1 x + b1); z = W2 y + b2`).
- [ ] MSE loss for a toy regression problem (e.g. fit `y = sin(πx)` on 50 points).
- [ ] Manual gradient descent (autograd grads + a plain `for` loop).
- [ ] Training-loss curve plotted via simple `printf` or matplotlib-equivalent rendering.

**Exit**: training loss decreases monotonically over ≥ 200 iterations; the notebook reproduces a recognisable fit to `sin(πx)`.

## Cross-cutting concerns

- **CMake / vcpkg**: no new dependencies anticipated. If matplotlib-equivalent rendering needs xeus-cling display hooks beyond `std::cout`, a JSON helper is added in P2.M6 only.
- **Branching**: each milestone is one `feature/autograd-<topic>` branch off `develop`. Merge commits, no squash (Git Flow + subtree rule).
- **Versioning**: `0.0.2-alpha` is tagged when P2.M6 closes. No GitHub release until `0.1.0`.
- **CI**: must remain green throughout. Any milestone PR that breaks CI is reverted, not papered over.
- **Documentation per code change**: every PR that lands code under `include/tensor/autograd/` updates either `arc42/05-building-blocks/` or `detailed-design/`.

## Risks and mitigations

| Risk                                                                          | Likelihood | Impact | Mitigation                                                                  |
| ----------------------------------------------------------------------------- | ---------- | ------ | --------------------------------------------------------------------------- |
| Broadcast backward design ergonomics (DynamicVariable vs templated)           | Medium     | High   | Time-box P2.M3 design phase to one week; prototype both variants and pick. |
| Numerical instability in `log` near zero on FD checker                        | High       | Low    | `gradient_check` uses central differences with eps = 1e-3 and tol = 1e-3; perturb-points away from boundary in tests. |
| xeus-cling memory leaks during long-running training loop in P2.M6           | Medium     | Medium | Recommend kernel restart between sections; document workaround in tutorial. |
| Scope creep into Phase 3 (GPU) territory                                      | Medium     | Medium | Hard rule: this plan ships only CPU; GPU comes in a separate dated plan.   |

## Cut lines (what to drop if behind schedule)

In priority order — drop top first:

1. P2.M6 plotting; ship loss values to `stdout` instead.
2. P2.M5 sections 4–6; ship sections 1–3 only.
3. P2.M4 rank-2 × rank-2 case; keep rank-2 × rank-1 (sufficient for MLP).
4. P2.M3 division backward; keep `+ - *`.

P2.M1 (shipped), P2.M2 minimum (exp, relu), and P2.M6 minimum (loss-decreases proof) are non-negotiable for `0.0.2-alpha`.

## Follow-ups beyond Phase 2

- Plan Phase 3 (WebGPU) in `2026-08-XX_phase-3-webgpu.md` when this file closes.
- Phase 1.5 mop-up plan (`2026-05-XX_phase-1-5-mopup.md`) — separately and in parallel — addresses M3 NTTP compile-time path, M4 evaluator bridge, and the mdspan `Kokkos::` namespace adapter.

## References

- [ADR-0007 — autograd as first-class subsystem](../arc42/09-decisions/0007-adopt-autograd-as-first-class-subsystem.md)
- [ADR-0009 — DDD + Hexagonal lite](../arc42/09-decisions/0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md)
- [Phase 1 retrospective](../reports/2026-05-11_phase-1-retrospective.md)
- [Phase 1 plan (predecessor, closed)](./2026-05-10_revival-phase-1.md)
