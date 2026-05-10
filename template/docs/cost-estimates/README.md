---
status: Stable
owner: <placeholder>
last-reviewed: 2026-05-04
---

# cost-estimates — dated cost projections

> **Layer B (volatile, dated, append-only).** Latest-wins.

## File naming

`YYYY-MM_<kebab-title>.md` (monthly cadence is enough for most projects).

Examples:

- `2026-05_baseline-monthly-cost.md`
- `2026-05_scaling-to-100k-users.md`

## What each file should contain

1. **Assumptions** — workload, traffic shape, retention, region.
2. **Per-service line items** — compute / storage / network / managed-service fees.
3. **Sensitivity analysis** — what changes the bill the most (the top 3 levers).
4. **Source** — the pricing-page URLs / contract terms used. Cost pages drift; cite a date.

For lifecycle, see [`../WORKFLOW.md`](../WORKFLOW.md).
