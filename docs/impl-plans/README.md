---
status: Stable
owner: tensor
last-reviewed: 2026-05-04
---

# impl-plans — dated implementation plans

> **Layer B (volatile, dated, append-only).** Use Template 0 ([`../templates/0_default.md`](../templates/0_default.md)) or write freely.

This directory holds plans for *how* a feature will be implemented over the next N weeks: phasing, milestones, dependencies, sequencing.

## File naming

`YYYY-MM-DD_<kebab-title>.md` — date-prefixed so the directory sorts chronologically.

Examples:

- `2026-05-04_billing-migration.md`
- `2026-05-11_search-rewrite-phase-1.md`

## Lifecycle

Active → Superseded by next dated file. **Never edit a closed file** — write a new one and link from it back to the predecessor.

For details, see [`../WORKFLOW.md`](../WORKFLOW.md).

## Index — plans in this project (7 as of 2026-05-14)

| Date       | File                                                                                                       | Status                                                  |
| ---------- | ---------------------------------------------------------------------------------------------------------- | ------------------------------------------------------- |
| 2026-05-10 | [`2026-05-10_revival-phase-1.md`](./2026-05-10_revival-phase-1.md)                                          | Closed (Phase 1 shipped — see `phase-1-retrospective`).  |
| 2026-05-11 | [`2026-05-11_phase-2-autograd.md`](./2026-05-11_phase-2-autograd.md)                                        | Closed (Phase 2 shipped).                                |
| 2026-05-11 | [`2026-05-11_phase-2-5-backend-port-and-eigen.md`](./2026-05-11_phase-2-5-backend-port-and-eigen.md)        | Closed (Phase 2.5 shipped — reference + Eigen).         |
| 2026-05-11 | [`2026-05-11_phase-3-webgpu.md`](./2026-05-11_phase-3-webgpu.md)                                            | Closed (Phase 3 shipped — 12 of 15 methods on RTX 3090). |
| 2026-05-12 | [`2026-05-12_post-investigation-tasks.md`](./2026-05-12_post-investigation-tasks.md)                        | Closed (carry-over tasks from the 2026-05-12 strategic review). |
| 2026-05-12 | [`2026-05-12_phase-6-python-sdk.md`](./2026-05-12_phase-6-python-sdk.md)                                    | Closed (Phase 6 shipped 2026-05-13 — see `phase-6-python-sdk-retrospective`). |
| 2026-05-13 | [`2026-05-13_phase-6-5-set-backend.md`](./2026-05-13_phase-6-5-set-backend.md)                              | **Active** — Phase 6.5 (M1–M4 forward; exit `0.3.0`).  |
