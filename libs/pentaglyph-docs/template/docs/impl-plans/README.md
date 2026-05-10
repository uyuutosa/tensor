---
status: Stable
owner: <placeholder>
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
