---
status: Stable
owner: <placeholder>
last-reviewed: 2026-05-04
---

# design-guide — operational conventions

> **Use Template 0** ([`../templates/0_default.md`](../templates/0_default.md)) unless one of Templates 1–5 fits.

This directory holds operational and convention guidelines: code style, naming, branch / commit conventions, review checklists, sprint cadence — anything that is "how this team works" rather than "how this system is built".

**What does NOT belong here:** architecture-level cross-cutting concerns (security model, error strategy, observability stack). Those go under [`../arc42/08-crosscutting/`](../arc42/08-crosscutting/).

## Suggested files

| File                          | Purpose                                                                 |
| ----------------------------- | ----------------------------------------------------------------------- |
| `coding-style.md`             | Language-specific style guides + exceptions to upstream defaults        |
| `branch-and-commit.md`        | Branch strategy + commit message format                                 |
| `review-checklist.md`         | What reviewers must verify before approving a PR                        |
| `ai-augmented-pr.md`          | Rules for PR descriptions on AI-assisted changes (kit-shipped default)  |
| `code-tours.md`               | CodeTour-compatible guided reading paths (kit-shipped default)          |
| `sprint-cadence.md`           | Sprint length, ceremonies, definition of done                           |
| `release-process.md`          | Versioning scheme, release notes format, rollback procedure             |

For lifecycle / when to update, see [`../WORKFLOW.md`](../WORKFLOW.md).
