---
status: Stable
owner: tensor
last-reviewed: 2026-05-04
---

# reports — one-shot research and evaluation reports

> **Layer B (volatile, dated, append-only).** For findings that are timestamped to a moment of evaluation and not expected to remain current.

## File naming

`YYYY-MM-DD_<kebab-title>.md`

Examples:

- `2026-05-04_vendor-comparison-vector-databases.md`
- `2026-05-11_load-test-results-checkout-flow.md`

## What belongs here

- Vendor / library comparisons
- One-shot benchmark results
- Spike / proof-of-concept findings
- Customer interview synthesis
- Data analyses

## What does NOT belong here

- Anything that should remain current → put it in Layer A (e.g. `arc42/10-quality/` for SLO benchmarks).
- Architectural decisions → write an ADR under `arc42/09-decisions/` instead.
- Bug retrospectives → use [`../postmortems/`](../postmortems/).

For lifecycle, see [`../WORKFLOW.md`](../WORKFLOW.md).

## Index — reports in this project (12 as of 2026-05-14)

| Date       | File                                                                                                                   | One-line summary                                                                                          |
| ---------- | ---------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------- |
| 2026-05-10 | [`2026-05-10_tensor-revival-landscape.md`](./2026-05-10_tensor-revival-landscape.md)                                    | Landscape scan that justified the revival pivot (ADR-0001).                                                |
| 2026-05-11 | [`2026-05-11_phase-1-retrospective.md`](./2026-05-11_phase-1-retrospective.md)                                          | Phase 1 close — build system + core + `_tex` parser MVP.                                                   |
| 2026-05-11 | [`2026-05-11_phase-2-and-2-5-retrospective.md`](./2026-05-11_phase-2-and-2-5-retrospective.md)                          | Phase 2 + 2.5 close — autograd + `KernelBackend` port + Eigen adapter.                                    |
| 2026-05-11 | [`2026-05-11_phase-4-release-rehearsal.md`](./2026-05-11_phase-4-release-rehearsal.md)                                  | Pre-`0.1.0` rehearsal audit (release readiness GREEN).                                                     |
| 2026-05-11 | [`2026-05-11_backend-performance-comparison.md`](./2026-05-11_backend-performance-comparison.md)                        | Reference / Eigen / WebGPU perf on RTX 3090 (2026-05-12 measurements appended).                            |
| 2026-05-11 | [`2026-05-11_open-discussion-points.md`](./2026-05-11_open-discussion-points.md)                                        | 8 axes of "next decisions" with maintainer recommendations.                                                |
| 2026-05-11 | [`2026-05-11_external-substrate-research.md`](./2026-05-11_external-substrate-research.md)                              | Vendor / library scan for Dawn / xeus-cpp / kokkos-stdBLAS.                                                |
| 2026-05-12 | [`2026-05-12_gpu-cpp-dawn-abi-drift-and-raw-dawn-smoke.md`](./2026-05-12_gpu-cpp-dawn-abi-drift-and-raw-dawn-smoke.md) | RTX 3090 smoke that surfaced gpu.cpp's 14-month ABI lag → ADR-0016.                                       |
| 2026-05-12 | [`2026-05-12_landscape-recheck-and-adversarial-review.md`](./2026-05-12_landscape-recheck-and-adversarial-review.md)    | Strategy re-validation; §A.5 names the Python ML wedge that drives Phase 6.                               |
| 2026-05-13 | [`2026-05-13_phase-6-python-sdk-retrospective.md`](./2026-05-13_phase-6-python-sdk-retrospective.md)                    | Phase 6 close — Python SDK via nanobind in two days across 17 PRs; the four boundary papercuts documented. |
| 2026-05-13 | [`2026-05-13_documentation-pdca-cycles.md`](./2026-05-13_documentation-pdca-cycles.md)                                  | 20-cycle docs PDCA run — horizontal Phase 6/6.5 gap-fill across arc42 / DD / design-guide / user-manual / api-contract. |
| 2026-05-14 | [`2026-05-14_documentation-pdca-100-cycles.md`](./2026-05-14_documentation-pdca-100-cycles.md)                          | **This file**: 100-cycle docs PDCA run — vertical-depth fill (per-DD §6/§7/§8 + Diátaxis quadrant population + design-guide + ADR audit + glossary completeness). |
