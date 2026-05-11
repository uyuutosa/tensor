---
status: Stable
owner: tensor
last-reviewed: 2026-05-11
---

# Phase 2 + Phase 2.5 retrospective — `tensor` autograd + backend port, 2026-05-11

| Metadata        | Value                                                          |
| --------------- | -------------------------------------------------------------- |
| Status          | Stable                                                         |
| Type            | Layer B — retrospective report (dated, append-only)            |
| Owner           | uyuutosa                                                       |
| Related plans   | [`../impl-plans/2026-05-11_phase-2-autograd.md`](../impl-plans/2026-05-11_phase-2-autograd.md) (Phase 2) and [`../impl-plans/2026-05-11_phase-2-5-backend-port-and-eigen.md`](../impl-plans/2026-05-11_phase-2-5-backend-port-and-eigen.md) (Phase 2.5) |
| Predecessor     | [`./2026-05-11_phase-1-retrospective.md`](./2026-05-11_phase-1-retrospective.md) |
| Successor (planned) | Phase 3 retrospective when WebGPU work closes              |

## Executive summary

Phase 2 (autograd) and Phase 2.5 (backend port) closed in the **same compressed window** as Phase 1. After 28 merged PRs the library has: tape-based reverse-mode autograd with broadcast-aware backward and contraction; a 15-method `KernelBackend` port with reference + Eigen adapters; LabelTag + TypedTensor giving the compile-time leg of the hybrid named-axis API; a `_tex` evaluator that bridges parse → AST → DynamicTensor end-to-end; four shipped Jupyter notebooks; a Jupyter Book scaffold; and a Phase 3 (WebGPU) impl-plan.

The architectural pivot recorded in ADR-0010 ("educational-first, **production-capable** via backend adapters") moved from aspiration to demonstrable reality with the Eigen adapter landing in PR #21.

## What shipped (Phase 2 — autograd build-out)

| Milestone | PR  | Deliverable                                                                                       |
| --------- | --- | ------------------------------------------------------------------------------------------------- |
| P2.M1     | #10 | Element-wise autograd MVP: `Variable<T, N>`, `Tape`, `sum_all`, `backward`, `gradient_check`.     |
| P2.M2     | #12 | Activations: `exp` / `log` / `relu` / `neg` with derivatives + FD parity tests.                  |
| P2.M3     | #13 | Broadcast-aware backward via `DynamicVariable<T>` + `unbroadcast` helper.                         |
| P2.M4     | #14 | Contraction: `dot(a, b)` via `tensor::core::contract`; backward through the same kernel.          |
| P2.M5     | #15 | `tutorials/05_autograd-from-scratch.ipynb` (6 sections).                                          |
| (prereq)  | #16 | DynamicVariable activations (unblock MLP chain).                                                  |
| P2.M6     | #17 | `tutorials/07_mlp-on-toy.ipynb` — Phase 2 capstone (training loop on y=2x+1).                    |

## What shipped (Phase 2.5 — backend port + Eigen)

| Milestone | PR  | Deliverable                                                                                       |
| --------- | --- | ------------------------------------------------------------------------------------------------- |
| (foundation) | #18 | ADR-0010 — refine ADR-0001 to "educational-first, production-capable" + Phase 2.5 plan.        |
| P2.5.M1   | #19 | ADR-0011 — `KernelBackend` port API design (15 methods); concept made concrete in `concepts.hpp`. |
| P2.5.M2   | #20 | `tensor::core::backend::reference::Backend` satisfies the concept; `unbroadcast` moved into core. |
| P2.5.M3   | #21 | `tensor::core::backend::eigen::Backend` (SIMD element-wise + GEMM matvec/matmul); CMake option `TENSOR_KERNEL_BACKEND={reference,eigen}`. |
| P2.5.M5   | #22 | `tutorials/08_swappable-backends.ipynb` — Hexagonal payoff demo.                                  |

(P2.5.M4 perf comparison report remains backlog — depends on a real machine with Eigen installed.)

## What shipped (Phase 1.5 mop-up, in parallel)

| Item                                              | PR  | Note                                                                                               |
| ------------------------------------------------- | --- | -------------------------------------------------------------------------------------------------- |
| M3 NTTP compile-time path primitives              | #23 | `FixedString<N>` + `LabelTag<S>` + `_ax` UDL.                                                       |
| M3 TypedTensor (compile-time-labelled wrapper)    | #24 | `TypedTensor<T, "i", "j">` with operator-level `static_assert` on label mismatch.                    |
| M4 `_tex` evaluator bridge                         | #25 | `tensor::tex::Evaluator<T>` — parse → AST → eval → DynamicTensor end-to-end.                       |
| `Variable::zero_grad()` + `sgd_update()`          | #26 | Training-loop ergonomics; toy regression test confirms convergence W=2.0007, b=0.9975.             |
| Phase 3 (WebGPU) impl-plan                        | #27 | 6 milestones; target window 2026-06-01 → 08-31; exit `0.0.4-alpha`.                                |
| Jupyter Book scaffold + GitHub Pages deploy       | #28 | `book/` directory + `.github/workflows/deploy-book.yml`. Live after Pages enable + next push.       |

## What was harder than planned

1. **CMake-vcpkg-Eigen interplay**. The vcpkg port name for Eigen3 is straightforward, but the rank-2 Eigen Map for row-major buffers required care: my GEMM code path uses `Eigen::Matrix<double, ..., ..., Eigen::RowMajor>` to match the project's row-major `DynamicTensor` layout, and transposes are inserted when the shared axis is on the "wrong" side. CI on PR #21 will validate. *(Pending green CI: the matrix transpose logic in `eigen::Backend::contract` was specified from first principles, not measured.)*
2. **Broadcast backward + `DynamicVariable`**. Mixing static-rank `Variable<T, N>` and runtime-rank `DynamicVariable<T>` in the same graph forced an asymmetry: the conversion from `Variable<T, N>` → `DynamicVariable<T>` allocates a *fresh* `DynamicGradAccum` rather than bridging to the static one. The conversion direction with grad tracking is deferred (a bridge closure is the proper fix; PR #13 documents this).
3. **`_tex` parser corner case**. Trailing-character tests were initially wrong: `parse("a_i garbage trailing")` doesn't throw because the parser's juxtaposition rule reads `garbage` and `trailing` as legal identifiers (implicit multiplication). The fix used a sentinel `?` character that the parser cannot absorb. Lesson: when a grammar has a fluent / consume-everything rule, *trailing* tests need explicit sentinels.

## What was easier than planned

1. **Hexagonal lite paying off immediately**. The reference adapter was a 5-method-wide refactor (PR #20); the Eigen adapter slot was the same shape minus the `if constexpr (std::is_same_v<T, double>)` fast paths (PR #21). Phase 3 (WebGPU) is now structurally a fill-in-the-blank exercise rather than an integration story.
2. **`contract` is self-dual under autograd**. `dL/da = contract(dL/dy, b)` and `dL/db = contract(a, dL/dy)` both reuse the same forward kernel. This is one of the algebraic gifts of named axes; the autograd `dot()` implementation is one helper for each side.
3. **NTTP class-type parameters in C++20** worked exactly as expected with deduction guides: `TypedTensor<double, "i", "j">` deduces both the FixedString sizes and the labels with no `<>` ceremony from the caller.

## Deferred / known-stale items

- mdspan polyfill restore (`Kokkos::Experimental::` namespace adapter).
- xeus-cling notebook CI (release-tag-time `nbconvert` smoke).
- LyX export module (ADR-0005's Phase 2 sub-deliverable — pending; covered by the `_tex` UDL for the in-source case).
- P2.5.M4 perf comparison report (requires a real machine).
- Phase 3 P3.M1 onward (planning shipped, implementation not started).
- 0.1.0 release tag and the public Jupyter Book go-live (Phase 4 close).

## Numbers (cumulative through 2026-05-11)

- **28 merged PRs** (Phase 1: 8, CI green-up: 1, Phase 2: 8, Phase 2.5: 5, Phase 1.5 mop-up: 4, Phase 3 plan: 1, Phase 4 prep: 1).
- **11 ADRs** in `Accepted` status (0001-0011).
- **5 dated impl-plans** under `docs/impl-plans/` (Phase 1, Phase 2, Phase 2.5, Phase 3) plus this retrospective and the Phase 1 retrospective under `docs/reports/`.
- **30+ C++ headers** across `tensor::core`, `tensor::autograd`, `tensor::tex` namespaces.
- **15 doctest TUs**.
- **4 Jupyter notebooks** shipped end-to-end + Jupyter Book scaffold.
- **2 concrete `KernelBackend` adapters** (`reference`, `eigen`); WebGPU planned.
- CI: 8 reference matrix jobs green + 1 Eigen job (Eigen CI iteration pending).

## Phase 3 entry conditions

- Phase 3 plan ready: [`../impl-plans/2026-05-11_phase-3-webgpu.md`](../impl-plans/2026-05-11_phase-3-webgpu.md).
- `KernelBackend` port API frozen at ADR-0011; WebGPU adapter is the third concrete implementation.
- Tutorial 08 (swappable backends) establishes the pedagogical frame for tutorial 06 (WebGPU acceleration).

## References

- Phase 1 retrospective (predecessor): [`./2026-05-11_phase-1-retrospective.md`](./2026-05-11_phase-1-retrospective.md)
- Research brief (Phase 0): [`./2026-05-10_tensor-revival-landscape.md`](./2026-05-10_tensor-revival-landscape.md)
- ADR set: [`../arc42/09-decisions/`](../arc42/09-decisions/)
