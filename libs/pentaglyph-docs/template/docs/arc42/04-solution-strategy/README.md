---
status: Draft
owner: <placeholder>
last-reviewed: <YYYY-MM-DD>
---

# arc42 §4 — Solution Strategy

> Authoritative source: <https://docs.arc42.org/section-4/>
>
> This section answers: *in 1–2 pages, what are the top design decisions that shape everything else?*

## What lives here

| Suggested file | Purpose                                                                                  |
| -------------- | ---------------------------------------------------------------------------------------- |
| `strategy.md`  | The 5–7 most important design decisions in summary form, each linking to its full ADR    |

## How to write here

1. Pick the **5–7 highest-leverage** decisions. Not 30. If you have more than 10, you are listing decisions, not strategy.
2. For each decision: one bullet stating *what was chosen* + a link to the full ADR under [`../09-decisions/`](../09-decisions/).
3. Do **not** restate the ADR's reasoning here. Strategy is a navigation index for decisions, not a replacement.
4. Update this file every time a top-level ADR is Accepted, Rejected, or Superseded.

For lifecycle / when to update, see [`../../WORKFLOW.md`](../../WORKFLOW.md).

## Cross-references

- §1 Introduction — goals these decisions serve
- §9 Decisions — full ADRs (the body these summaries link to)
- §5 Building Blocks — structural shape that follows from the strategy
