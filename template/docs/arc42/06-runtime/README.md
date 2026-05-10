---
status: Draft
owner: <placeholder>
last-reviewed: <YYYY-MM-DD>
---

# arc42 §6 — Runtime View

> Authoritative source: <https://docs.arc42.org/section-6/>
>
> This section answers: *how do the building blocks interact during important business scenarios?*

## What lives here

One file per business scenario or critical runtime flow:

| Naming convention | Example | Purpose |
| ----------------- | ------- | ------- |
| `NN-<scenario>.md` | `01-user-onboarding.md` | One scenario, end-to-end sequence |

## How to write here

1. Use Template 4 (Use Case) — the Main Success Scenario + Sequence diagram sections cover most of §6.
2. Cover **at least the 5–10 most important scenarios** (happy path + a few worst-case / error paths).
3. Each scenario must reference building blocks from [`../05-building-blocks/`](../05-building-blocks/) by exact name. If a scenario invokes a building block that does not exist in §5, fix §5 first.
4. Sequence diagrams: Mermaid `sequenceDiagram` is the default. Embed inline; do not link to external image files.

For lifecycle / when to update, see [`../../WORKFLOW.md`](../../WORKFLOW.md).

## Cross-references

- §3 Context and Scope — actors that initiate these scenarios
- §5 Building Blocks — components that participate
- §10 Quality — performance scenarios that constrain runtime behaviour
