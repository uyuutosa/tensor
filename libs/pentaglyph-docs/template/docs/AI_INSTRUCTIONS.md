---
status: Stable
owner: <placeholder>
last-reviewed: 2026-05-04
audience: AI agents (Claude / Cursor / Copilot / generic LLMs)
---

# AI_INSTRUCTIONS — Read me first if you are an AI agent

> **You are an AI assistant working in a codebase that uses this `docs/` kit.**
> This file is your entry point. Read it once at the start of every session that touches `docs/`.

---

## 1. The two files you must internalise

| File | Purpose | When to consult |
|---|---|---|
| [`WORKFLOW.md`](./WORKFLOW.md) | Decision tree, lifecycle, code → doc mapping | Every time you create or update a doc |
| [`STRATEGY.md`](./STRATEGY.md) | Layer model, taxonomy, rationale | When you are unsure where a new concept belongs |

If those two files contradict anything you read elsewhere in this repo, **`WORKFLOW.md` wins**.

---

## 2. Decision protocol — every time you touch `docs/`

Run this protocol mentally before writing or editing a doc.

```
STEP 1: Classify the change
   What kind of change is this?
   ├─ Architectural decision?            → ADR (Template 5)
   ├─ Product requirement?               → PRD (Template 2)
   ├─ Module implementation spec?        → Module Detailed Design (Template 3)
   ├─ Actor / scenario?                  → Use Case (Template 4)
   ├─ Big-picture / cross-cutting?       → Architecture Overview (Template 1)
   ├─ Volatile (dated)?                  → Layer B directories
   └─ End-user docs?                     → user-manual/ (Diátaxis quadrant)

STEP 2: Find the destination directory
   Look up the destination in WORKFLOW.md §1 (decision tree) or §2 (mapping table).
   Never guess. If the table does not name your case, ask the user.

STEP 3: Pick the template
   Copy the named template from templates/ as the starting point.
   Do not invent a new front-matter format.

STEP 4: Set the lifecycle state
   New doc → `status: Draft` (or `Proposed` for ADRs).
   Editing existing → check current `status:` first; if `Done` or `Accepted`,
   STOP and ask whether to supersede instead of edit.

STEP 5: Cross-link
   - Link any new doc into the parent section's README.md index.
   - If you reference an external standard (arc42 / C4 / MADR / Diátaxis),
     link to the canonical URL, do not paraphrase.

STEP 6: Verify the PR contains both code AND doc changes
   If the PR changes code in src/ but does not change docs/, the PR is incomplete.
   Add the doc update before requesting review.
```

---

## 3. Per-directory cheat sheet

The detailed instructions live in each directory's `README.md`. This is the index — when you are about to write into directory X, open `X/README.md` first.

| Directory | What lives here | README.md says |
|---|---|---|
| `arc42/01-introduction-and-goals/` | System identity, top-level goals, stakeholders | Use Template 1 |
| `arc42/02-architecture-constraints/` | Fixed constraints (technical / organisational / regulatory) | One file per constraint family |
| `arc42/03-context-and-scope/` | System-context diagrams, actor list, use-cases, PRDs | C4 Level 1 + Template 4 + Template 2 |
| `arc42/04-solution-strategy/` | Top 5 design decisions in summary form | Each line links to its ADR |
| `arc42/05-building-blocks/` | Static decomposition (containers + components) | Index of `detailed-design/` files |
| `arc42/06-runtime/` | Sequence diagrams per business scenario | One file per scenario, Template 4 |
| `arc42/07-deployment/` | Environments, infra topology | One file per environment |
| `arc42/08-crosscutting/` | Security, observability, error handling, etc. | One file per concern |
| `arc42/09-decisions/` | ADRs (MADR v3.0) | Template 5; immutable once Accepted |
| `arc42/10-quality/` | SLOs, KPIs, quality scenarios | Measurable, testable |
| `arc42/11-risks/` | Risk register, technical debt | Honest enumeration |
| `arc42/12-glossary/` | Terms, abbreviations | Add on first use |
| `diagrams/c4/` | `workspace.dsl` (Structurizr DSL) | Single source of truth |
| `detailed-design/` | Per-module specs (HOW) | Template 3, linked from §5 |
| `design-guide/` | Conventions (naming, style, team agreements) | Template 0 |
| `api-contract/` | OpenAPI / MCP-tool schemas | One file per module group |
| `impl-plans/` | Dated implementation plans | Volatile, append-only |
| `task-list/` | Sprint task breakdowns | Volatile, append-only |
| `postmortems/` | Incident retrospectives | Volatile, append-only, Medium+ severity |
| `reports/` | Research / evaluation reports | Volatile, append-only |
| `cost-estimates/` | Cost projections | Volatile, latest-wins |
| `user-manual/tutorials/` | Learning-oriented (Diátaxis) | For newcomers |
| `user-manual/how-to/` | Problem-oriented (Diátaxis) | For competent users |
| `user-manual/reference/` | Information-oriented (Diátaxis) | Dry, lookup-able |
| `user-manual/explanation/` | Understanding-oriented (Diátaxis) | Background reading |

---

## 4. Things you must never do

1. **Never invent a new template.** The 6 templates in `templates/` cover every case.
2. **Never edit an Accepted ADR's body.** Supersede with a new ADR.
3. **Never duplicate the workflow rules.** If you find yourself restating `WORKFLOW.md` content in another file, link to `WORKFLOW.md` instead.
4. **Never re-explain arc42 / C4 / MADR / Diátaxis.** Link to the authoritative URLs.
5. **Never write doc-only PRs that change code-coupled docs.** Doc changes that describe non-existent code are bugs. If the code is being removed, supersede the doc in the same PR.
6. **Never put files into Layer C (`archive/_legacy/`)** unless explicitly instructed. Layer C is read-only history.
7. **Never write in a non-English language** in default-English directories. Per `STRATEGY.md`, only explicitly designated client-facing directories may be in another language.

---

## 5. When you are uncertain

If after running the [decision protocol](#2-decision-protocol--every-time-you-touch-docs) you still cannot place a doc:

1. **Ask the user.** Do not guess and create files in arbitrary locations.
2. **Search existing docs first.** A near-duplicate may already exist; update it instead of creating a new file.
3. **Default to `Template 0`** if no specialised template fits, but bias toward updating an existing file.

---

## 6. Self-check before requesting review

Before saying "doc update done", verify:

- [ ] The doc is placed under the directory pointed to by `WORKFLOW.md` §2.
- [ ] Front-matter has `status:`, `owner:`, `last-reviewed:` (durable docs) or a date prefix in the filename (volatile docs).
- [ ] The chosen template was used as the starting point.
- [ ] The parent directory's `README.md` index lists the new file.
- [ ] No external standard is paraphrased — only linked.
- [ ] If this is an ADR, the Y-statement section is filled and Status is `Proposed` (Accepted-state changes go through human review).
- [ ] The same PR also contains the code change that motivated the doc change.

---

## 7. References

This file deliberately does not re-explain why arc42 / C4 / MADR / Diátaxis exist. Read the originals once:

- arc42 — <https://arc42.org/overview/>
- C4 model — <https://c4model.com>
- MADR v3.0 — <https://adr.github.io/madr/>
- Diátaxis — <https://diataxis.fr>
