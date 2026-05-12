---
status: Draft
owner: <placeholder>
last-reviewed: <YYYY-MM-DD>
---

# arc42 §10 — Quality Requirements

> Authoritative source: <https://docs.arc42.org/section-10/>
>
> This section answers: *what quality attributes must the system satisfy, with measurable targets?*

## What lives here

| Suggested file         | Purpose                                                                  |
| ---------------------- | ------------------------------------------------------------------------ |
| `quality-tree.md`      | Hierarchical quality goals (ISO 25010 categories)                        |
| `quality-scenarios.md` | Concrete scenarios per quality attribute (Source-Stimulus-Environment-Response form) |
| `slos.md`              | Service-Level Objectives — measurable runtime targets + error budgets    |
| `kpis.md`              | Product / business KPIs (when distinct from SLOs)                        |

## How to write here

1. **Every quality requirement must be measurable.** "The system should be fast" is not a requirement — "p95 chat-response latency under 1.0s on 10 RPS" is.
2. Each scenario states: *Source* → *Stimulus* → *Environment* → *Response* → *Response Measure*.
3. SLOs link out to the dashboard / monitor that proves them in production.
4. Cross-link from each quality scenario to the [`../05-building-blocks/`](../05-building-blocks/) component(s) responsible for upholding it.

For lifecycle / when to update, see [`../../WORKFLOW.md`](../../WORKFLOW.md).

## Cross-references

- §1 Introduction — top-level Quality Goals (5–6 of them) that this section concretises
- §5 Building Blocks — components responsible for each quality attribute
- §6 Runtime — scenarios that exercise the quality attributes
- §11 Risks — risks of failing to meet a quality target
