---
status: Stable
owner: <placeholder>
last-reviewed: 2026-05-04
---

# arc42 — Architecture Description

This directory follows **arc42** (12 sections). Authoritative source: <https://arc42.org/overview/>.

> For *what to write when*, see [`../WORKFLOW.md`](../WORKFLOW.md). For *why this layout*, see [`../STRATEGY.md`](../STRATEGY.md). This README is a section index only.

| §   | Section                  | Local                                                       |
| --- | ------------------------ | ----------------------------------------------------------- |
| 1   | Introduction and Goals   | [`01-introduction-and-goals/`](./01-introduction-and-goals/)|
| 2   | Architecture Constraints | [`02-architecture-constraints/`](./02-architecture-constraints/) |
| 3   | Context and Scope        | [`03-context-and-scope/`](./03-context-and-scope/)          |
| 4   | Solution Strategy        | [`04-solution-strategy/`](./04-solution-strategy/)          |
| 5   | Building Block View      | [`05-building-blocks/`](./05-building-blocks/)              |
| 6   | Runtime View             | [`06-runtime/`](./06-runtime/)                              |
| 7   | Deployment View          | [`07-deployment/`](./07-deployment/)                        |
| 8   | Crosscutting Concepts    | [`08-crosscutting/`](./08-crosscutting/)                    |
| 9   | Architecture Decisions   | [`09-decisions/`](./09-decisions/) (MADR)                   |
| 10  | Quality Requirements     | [`10-quality/`](./10-quality/)                              |
| 11  | Risks and Technical Debt | [`11-risks/`](./11-risks/)                                  |
| 12  | Glossary                 | [`12-glossary/`](./12-glossary/)                            |

The 12 sections are non-negotiable. Do not rename, renumber, or merge them. If a section is empty for your project, leave its `README.md` in place with a one-line "intentionally empty" note.

For the meaning and intent of each section, refer to <https://docs.arc42.org/>. Do not re-explain it locally.

## Default templates

| Template | Use for |
| -------- | ------- |
| [`../templates/1_architecture-overview.md`](../templates/1_architecture-overview.md) | §1 / §3 / §4 / §5 / §8 / §10 / §11 / §12 — most arc42 sections |
| [`../templates/3_module-detailed-design.md`](../templates/3_module-detailed-design.md) | Per-module detailed designs under [`../detailed-design/`](../detailed-design/) (linked from §5) |
| [`../templates/4_use-case.md`](../templates/4_use-case.md) | Use-case files under [`03-context-and-scope/use-cases/`](./03-context-and-scope/use-cases/) |
| [`../templates/5_adr.md`](../templates/5_adr.md) | ADRs under [`09-decisions/`](./09-decisions/) (MADR v3.0) |
