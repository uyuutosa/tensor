---
status: Stable
owner: <placeholder>
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
