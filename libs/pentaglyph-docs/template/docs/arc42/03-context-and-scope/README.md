---
status: Draft
owner: <placeholder>
last-reviewed: <YYYY-MM-DD>
---

# arc42 §3 — Context and Scope

> Authoritative source: <https://docs.arc42.org/section-3/>
>
> This section answers: *what is the system's boundary and who interacts across it?*

## What lives here

| Suggested file / dir   | Purpose                                                                               |
| ---------------------- | ------------------------------------------------------------------------------------- |
| `business-context.md`  | Business actors, business external systems, value flows                               |
| `system-context.md`    | C4 Level 1 system-context diagram + technical interfaces                              |
| `use-cases/`           | One file per use case (Template 4)                                                    |
| `prds/`                | One file per PRD (Template 2)                                                         |

The C4 L1 diagram lives in [`../../diagrams/c4/workspace.dsl`](../../diagrams/c4/workspace.dsl). This section embeds an export or links to it.

## How to write here

1. Use Template 1 for `business-context.md` and `system-context.md` (or trim its arc42 §3 portion).
2. Use Template 4 for each file in `use-cases/`.
3. Use Template 2 for each file in `prds/`.
4. Every external system must appear here exactly once with a stable identifier; downstream sections reference by that identifier.

For lifecycle / when to update, see [`../../WORKFLOW.md`](../../WORKFLOW.md).

## Cross-references

- §1 Introduction — goals served by these external interactions
- §5 Building Blocks — internal decomposition that satisfies these external interfaces
- §6 Runtime — sequences that span these external boundaries
- [`../../diagrams/c4/`](../../diagrams/c4/) — C4 L1 single source of truth
