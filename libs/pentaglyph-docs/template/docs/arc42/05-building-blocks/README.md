---
status: Draft
owner: <placeholder>
last-reviewed: <YYYY-MM-DD>
---

# arc42 §5 — Building Block View

> Authoritative source: <https://docs.arc42.org/section-5/>
>
> This section answers: *what is the static decomposition of the system into building blocks?*

## What lives here

| Suggested file          | Purpose                                                                                   |
| ----------------------- | ----------------------------------------------------------------------------------------- |
| `overview.md`           | Top-level decomposition, key topology, naming conventions                                 |
| `level-2-containers.md` | Per-container fact sheet: responsibility, interfaces, scaling notes                       |
| `level-3-components.md` | Per-component zoom-in for structurally complex containers                                 |

**Implementation HOW does not live here.** Per-module implementation specs live under [`../../detailed-design/`](../../detailed-design/) (Template 3). This section is a **navigation index** that links to those files.

## How to write here

1. Use Template 1 (architecture-overview) trimmed to the §5 portion.
2. Names of containers and components must match exactly across §5, [`../../diagrams/c4/workspace.dsl`](../../diagrams/c4/workspace.dsl), and [`../../detailed-design/`](../../detailed-design/) — no synonyms.
3. Show **why each building block exists** by linking back to the runtime scenarios in [`../06-runtime/`](../06-runtime/) it serves. If no runtime scenario needs a building block, it should not exist.
4. Diagrams: render from `workspace.dsl` and embed; never hand-draw a diagram that diverges from the DSL.

For lifecycle / when to update, see [`../../WORKFLOW.md`](../../WORKFLOW.md).

## Cross-references

- §3 Context and Scope — C4 L1 zoom-out
- §6 Runtime — these building blocks in motion
- §9 Decisions — ADRs justifying the structural choices
- [`../../diagrams/c4/workspace.dsl`](../../diagrams/c4/workspace.dsl) — single source of truth for structural diagrams
- [`../../detailed-design/`](../../detailed-design/) — implementation specs (HOW)
