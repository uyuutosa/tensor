---
status: Stable
owner: <placeholder>
last-reviewed: 2026-05-04
---

# task-list — sprint-scoped task breakdowns

> **Layer B (volatile, dated, append-only).** The ticket system (GitHub Issues / Jira / ADO Boards) is the source of truth for individual tasks; this directory holds *snapshots* of sprint plans for reference.

## File naming

`YYYY-MM-DD_<sprint-or-iteration>.md`

Examples:

- `2026-05-04_sprint-23.md`
- `2026-05-11_release-1.4-cut.md`

## What goes here vs. in the ticket system

| Lives here                                                  | Lives in ticket system                                   |
| ----------------------------------------------------------- | -------------------------------------------------------- |
| Plan-time snapshot (what we agreed at sprint planning)      | Live state (Active / Done / Blocked) of each ticket      |
| Cross-ticket dependency narrative                           | Per-ticket detail (assignee, estimate, comments)         |
| "Why this scope, not the alternatives" reasoning            | Daily progress comments                                  |

For lifecycle, see [`../WORKFLOW.md`](../WORKFLOW.md).
