---
paths:
  - "docs/**"
---

# Documentation Rule (auto-loaded for `docs/**`)

This project uses the **pentaglyph-docs** scaffold: arc42 + C4 + MADR + Diátaxis bound by one workflow.

## Two files you must read first

| File                                               | Purpose                                                      |
| -------------------------------------------------- | ------------------------------------------------------------ |
| [`docs/AI_INSTRUCTIONS.md`](../../docs/AI_INSTRUCTIONS.md) | Entry point for AI agents — decision protocol per touch     |
| [`docs/WORKFLOW.md`](../../docs/WORKFLOW.md)       | When to write what, where to put it, what state it goes through |

If the rule below contradicts those files, **the files in `docs/` win** — they are the source of truth, this rule is just a pointer.

## One-line summary

**Code change implies doc change in the same PR.** Use the table in `docs/WORKFLOW.md §2` to find which doc to update.

## Hard rules (verbatim from `docs/WORKFLOW.md §6`)

1. **One canonical location per topic.** If a concept appears in two files, one must be a link to the other.
2. **Front-matter on all durable docs** (`arc42/`, `detailed-design/`, `design-guide/`, `api-contract/`, `user-manual/`).
3. **Date prefix on all volatile docs** (`YYYY-MM-DD_<kebab-title>.md`).
4. **MADR for ADRs.** No homemade ADR formats. Use `docs/templates/5_adr.md`.
5. **C4 single source of truth = `docs/diagrams/c4/workspace.dsl`.** SVG renders under `docs/diagrams/c4/exports/` are **committed** so repo web UIs (CodeCommit, GitHub Enterprise, Bitbucket) can display them without local tooling. Regenerate via the [`/diagram-render`](../skills/diagram-render/SKILL.md) skill; CI may enforce `/diagram-render --check` to block drift.
6. **English by default.** Other languages reserved for explicitly designated client-facing locations declared in `docs/STRATEGY.md`.
7. **Repo-root-relative links.** Use `docs/<path>` form so links survive reorganization.
8. **No re-explaining external standards.** Link to <https://arc42.org> / <https://c4model.com> / <https://adr.github.io/madr/> / <https://diataxis.fr> instead of paraphrasing.

## Forbidden

- Editing anything under `docs/archive/` (read-only history).
- Editing the body of an `Accepted` ADR (supersede with a new ADR instead).
- Inventing a new template (the nine in `docs/templates/` cover every case — six core + three UX research).
- Duplicating workflow rules into other files (link to `docs/WORKFLOW.md`).
- Writing in a non-English language in default-English directories.

## When in doubt

1. Run the [decision protocol](../../docs/AI_INSTRUCTIONS.md#2-decision-protocol--every-time-you-touch-docs) in `AI_INSTRUCTIONS.md §2`.
2. If still unclear, ask the user. Do not guess directories.
