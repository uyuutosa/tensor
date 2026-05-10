---
status: Active
owner: tensor
last-reviewed: 2026-05-11
---

# Phase 2.5 — `KernelBackend` port design and the Eigen adapter

| Metadata      | Value                                                                  |
| ------------- | ---------------------------------------------------------------------- |
| Status        | Active                                                                 |
| Type          | Layer B — implementation plan (dated, append-only)                     |
| Owner         | uyuutosa                                                               |
| Predecessor   | none (parallel to Phase 2's tail end and the Phase 1.5 mop-up backlog) |
| Related       | [ADR-0010](../arc42/09-decisions/0010-refine-positioning-to-educational-first-production-capable.md), [ADR-0009](../arc42/09-decisions/0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md), [ADR-0006](../arc42/09-decisions/0006-adopt-webgpu-as-gpu-backend.md) |
| Target window | 2026-05-12 → 2026-06-30                                                |
| Exit version  | `0.0.3-alpha`                                                          |

## TL;DR

Phase 2.5 takes the *anticipatory* `KernelBackend` port declared in [`include/tensor/core/concepts.hpp`](../../include/tensor/core/concepts.hpp) and gives it a real API. The reference CPU implementation is refactored to satisfy the port explicitly; **Eigen** is added as the first non-reference adapter (CPU SIMD + cached BLAS-flavoured GEMM). A new tutorial chapter demonstrates swapping backends at CMake configure time with no Domain code changes.

This is the milestone that **operationalises ADR-0010** ("educational-first, production-capable"). After it lands, the library actually delivers what the refined positioning promises.

WebGPU (Phase 3) becomes "the second non-reference adapter" rather than a fresh integration story.

## Phase 2.5 milestones

### P2.5.M1 — `KernelBackend` port API (ADR-0011) (week 1)

> **Realises** the design follow-up from ADR-0010.

- [ ] Write ADR-0011 (`KernelBackend` port API design) — names the methods every backend must provide, the buffer/extent ownership model, and the CMake configuration variable that selects the active backend.
- [ ] Surface area (initial):
  - `elementwise_apply<Op>(a, b) -> c` — for `+ - * /`.
  - `broadcast_apply<Op>(a, b, plan) -> c` — same with `BroadcastPlan`.
  - `contract(a, b, plan) -> c` — for `dot`.
  - `reduce_sum(a) -> scalar`.
  - The backward primitives (`unbroadcast`, etc.) are *not* part of the port — they compose `KernelBackend` operations.
- [ ] Update `include/tensor/core/concepts.hpp` to make `KernelBackend` concrete (move beyond the marker).
- [ ] Build option `TENSOR_KERNEL_BACKEND={reference,eigen,...}` selecting the active adapter.

**Exit**: `KernelBackend` concept compiles with a non-trivial body; reference adapter satisfies it; CMake selects backends; no consumer code changes outside the kernel call sites.

### P2.5.M2 — Refactor reference CPU adapter (week 2)

> **Realises** the cleanup needed before adding any second adapter.

- [ ] Extract the existing kernels (`ops.hpp`, `broadcast_ops.hpp`, `contract.hpp`) into an explicit `tensor::core::backend::reference` namespace satisfying the port.
- [ ] Domain code (`Tensor`, `DynamicTensor`, `Shape`, `Axis`, `Variable`, `DynamicVariable`) is unchanged. Only the kernel dispatch routes through the active backend.
- [ ] All existing tests pass against `TENSOR_KERNEL_BACKEND=reference`.

**Exit**: the build matrix gains a `backend=reference` axis without changing any test results.

### P2.5.M3 — Eigen adapter (weeks 2–3)

> **Realises** the first production-grade adapter.

- [ ] Add `eigen3` dependency to `vcpkg.json`.
- [ ] Implement `tensor::core::backend::eigen` satisfying `KernelBackend`:
  - Element-wise operations via `Eigen::Map<Eigen::Array>` for vectorised SIMD execution.
  - `contract` for rank-2 × rank-1 and rank-2 × rank-2 via `Eigen::Map<Eigen::Matrix>` GEMM (which Eigen routes to BLAS when available).
  - Higher-rank contractions fall back to looped Eigen calls; pure-Eigen Einstein notation is out of scope for this PR.
- [ ] CI matrix gains a `backend=eigen` axis.
- [ ] All tests pass against `TENSOR_KERNEL_BACKEND=eigen`.

**Exit**: a learner can configure with `cmake --preset=eigen` and observe the same test suite pass with Eigen-driven kernels.

### P2.5.M4 — Performance comparison report (week 3)

> **Realises** the educational payoff of the abstraction.

- [ ] Add `bench/` directory with a small Google-Benchmark or doctest-timed comparison.
- [ ] Benchmark cases:
  - `a + b` element-wise, 1M elements.
  - Matrix-vector multiply, 1024 × 1024 × 1024.
  - Matrix-matrix multiply, 512 × 512.
- [ ] Compare `reference` vs `eigen` per case. Numbers go into a `docs/reports/2026-XX-XX_backend-performance-comparison.md`.
- [ ] No PR merge gating on absolute numbers; the report exists so future regressions are visible.

**Exit**: a Layer-B report with the first comparative numbers committed.

### P2.5.M5 — `tutorials/08_swappable-backends.ipynb` (weeks 3–4)

> **Realises** ADR-0010's "the architecture itself is a teaching artifact".

- [ ] Notebook walks through:
  - Section 1 — the `KernelBackend` port and why this design exists.
  - Section 2 — switch backends at runtime via a CMake variable simulated in the notebook (or via two separately-built kernels).
  - Section 3 — run a matmul on `reference` vs `eigen`; print timings.
  - Section 4 — gradient check confirms outputs match across backends.
  - Section 5 — WebGPU is the next adapter (forward link to Phase 3).
- [ ] Validate notebook JSON; end-to-end execution remains a Phase 1.5 mop-up.

**Exit**: chapter shipped; tutorials/README.md updated.

### P2.5.M6 — Positioning docs sync (week 4)

> **Realises** ADR-0010's follow-up items.

- [ ] Update `README.md`'s first paragraph and Status table:
  - Tagline: "**educational-first, production-capable** via backend adapters".
  - Disclaimer: "production users adopt as-is, no ABI / coverage / support commitments".
- [ ] Update `docs/arc42/01-introduction-and-goals/overview.md` §2 stakeholders to add "production user (`as-is`)".
- [ ] Update `docs/arc42/04-solution-strategy/strategy.md` to insert ADR-0010 in the top-7.
- [ ] Update `docs/arc42/05-building-blocks/overview.md` to mention the active `KernelBackend` adapter set.
- [ ] Update the Phase 1 retrospective and Phase 2 plan with cross-references to ADR-0010.

**Exit**: the durable docs match the actual policy. No contradictions between README, ADR-0001, and ADR-0010.

## Cut lines (drop top first if behind schedule)

1. P2.5.M5 sections 4–5; ship the notebook through Section 3.
2. P2.5.M4 (benchmark); ship M3 Eigen adapter without numbers.
3. P2.5.M3 contraction support (matmul) on Eigen; keep element-wise + broadcast only and defer GEMM to a follow-up.

P2.5.M1, M2, M6 are non-negotiable for `0.0.3-alpha`.

## Risks and mitigations

| Risk                                                            | Likelihood | Impact | Mitigation                                                                  |
| --------------------------------------------------------------- | ---------- | ------ | --------------------------------------------------------------------------- |
| Eigen's API ergonomics clash with named-axis Domain types       | Medium     | Medium | Keep `Eigen::Map` use confined to the adapter; never leak Eigen types into Domain. |
| CI matrix doubles (`backend=reference` × `backend=eigen`)       | High       | Low    | Run both per PR; if too slow, reference-only on Debug and eigen-only on Release. |
| Production users file issues asking for op coverage             | High       | Medium | README + ADR-0010 disclaimer; CONTRIBUTING.md (TBD) sets expectations.    |
| Benchmark numbers become a target rather than an observation    | Medium     | Low    | Frame the comparison report as "snapshot, not goal"; no PR-gating.        |

## Follow-ups beyond Phase 2.5

- **Phase 3 (WebGPU, ADR-0006)** — `tensor::gpu` becomes the third `KernelBackend` adapter following the same template.
- **Kokkos adapter** (Phase 4+) — covers CPU + multi-GPU with a single adapter.
- **BLAS-only adapter** (Phase 4+) — useful for environments where Eigen is heavier than needed.
- **Phase 1.5 mop-up items** (M3 NTTP path, M4 evaluator bridge, mdspan polyfill, xeus-cling CI) — proceed in parallel.

## References

- [ADR-0010](../arc42/09-decisions/0010-refine-positioning-to-educational-first-production-capable.md)
- [ADR-0009](../arc42/09-decisions/0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md)
- [ADR-0006](../arc42/09-decisions/0006-adopt-webgpu-as-gpu-backend.md)
- [Eigen](https://eigen.tuxfamily.org/) — first non-reference adapter
- [tinygrad](https://github.com/tinygrad/tinygrad) — educational-first-production-capable precedent
- Phase 1 retrospective: [`../reports/2026-05-11_phase-1-retrospective.md`](../reports/2026-05-11_phase-1-retrospective.md)
- Phase 2 plan: [`./2026-05-11_phase-2-autograd.md`](./2026-05-11_phase-2-autograd.md)
