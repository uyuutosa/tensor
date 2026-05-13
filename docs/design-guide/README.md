---
status: Stable
owner: tensor
last-reviewed: 2026-05-14
---

# design-guide — operational conventions

> "How this team works" rather than "how this system is built". The pentaglyph-docs scaffold ships some defaults (`ai-augmented-pr.md`, `code-tours.md`, `version-control.md`); the project-specific ones are added in cycles 13-14 + 67-71 of the docs PDCA runs.

**What does NOT belong here:** architecture-level cross-cutting concerns (security model, error strategy, observability stack). Those go under [`../arc42/08-crosscutting/`](../arc42/08-crosscutting/).

## Index — files in this project (11 as of 2026-05-14)

### Architecture + convention discipline

- [`architectural-discipline.md`](./architectural-discipline.md) — the Hexagonal-lite dependency rule + the grep-based enforcement examples.
- [`cpp-style-guide.md`](./cpp-style-guide.md) — project-specific C++ idioms on top of C++ Core Guidelines.
- [`python-binding-style.md`](./python-binding-style.md) — nanobind binding conventions (the four boundary patterns + arg naming + type conversion).
- [`python-notebook-authoring.md`](./python-notebook-authoring.md) — execute-before-commit, plotly MathJax v2 trap, math syntax, Sphinx source-tree limit, common-mistakes table.

### Workflow + release

- [`version-control.md`](./version-control.md) — the project's Git Flow specialisation + 2026-05-14 lessons-learned section.
- [`release-ceremony.md`](./release-ceremony.md) — the 8-step release sequence as actually executed at `0.1.0` + `0.2.0`, with hard + soft rules.
- [`ai-augmented-pr.md`](./ai-augmented-pr.md) — PR description rules + worked exemplars from PRs #109, #119, #120, #121.
- [`code-tours.md`](./code-tours.md) — `.tours/` convention + planned-tour priority list.

### Citability discipline

- [`glossary-maintenance.md`](./glossary-maintenance.md) — when to add a §12 glossary entry, half-yearly audit cadence.
- [`cross-reference-discipline.md`](./cross-reference-discipline.md) — link style + audit script + common breakage patterns.

For lifecycle / when to update, see [`../WORKFLOW.md`](../WORKFLOW.md).

## Authoring rules

1. **One topic per file.** A file that covers C++ style AND notebook authoring is two files.
2. **Cross-link to the relevant ADR / arc42 / detailed-design.** Every design-guide file is the *contributor-facing* face of decisions captured elsewhere; the upstream ratification stays in arc42 / ADRs.
3. **Show, don't just tell.** Code examples + WRONG/RIGHT pairs are required for technical guides (style guides, binding conventions, etc.).
4. **Half-yearly review.** Each `last-reviewed` frontmatter bumps at the bibliography audit (next: 2026-11-11).

## Cross-references

- [`../arc42/`](../arc42/) — the architecture that these conventions serve.
- [`../api-contract/`](../api-contract/) — public surface contracts these conventions help maintain.
- [`../user-manual/`](../user-manual/) — Diátaxis-typed user-facing docs the conventions enable.
- [`../templates/`](../templates/) — file templates for new design-guide entries.
