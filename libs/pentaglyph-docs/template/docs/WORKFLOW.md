---
status: Stable
owner: <placeholder>
last-reviewed: 2026-05-04
---

# WORKFLOW — When to write what, where to put it, what state it goes through

> **Read this file first.** Every other doc in this repo links back here.
>
> This is the **single source of truth** for the documentation workflow. Do not duplicate the rules below into other files; link to this document instead.

---

## TL;DR

1. **Code change → doc change.** They land in the same PR. No exceptions beyond trivial typos.
2. **Pick one template** from `templates/` based on the [decision tree](#1-which-template-do-i-use) below.
3. **Place the file** in the directory the decision tree points to.
4. **Set the front-matter** (`status: Draft / Review / Done / Superseded`, `owner`, `last-reviewed`).
5. **Move through the lifecycle**: Draft → Review → Done. Append-only for volatile docs.
6. **Supersede, do not edit** for ADRs and Done durable docs.

---

## 1. Which template do I use?

```
START: I am about to write a doc.
│
├─ Is it a single architectural decision?
│   YES → Template 5 (ADR / MADR v3.0)
│         Place in: arc42/09-decisions/NNNN-<kebab-title>.md
│
├─ Is it a product requirement (WHAT/WHY, user-facing)?
│   YES → Template 2 (PRD)
│         Place in: arc42/03-context-and-scope/prds/<feature>.md
│         (or wherever your PRDs live; arc42 §3 is the canonical home)
│
├─ Is it an actor / scenario behaviour spec?
│   YES → Template 4 (Use Case)
│         Place in: arc42/03-context-and-scope/use-cases/<name>.md
│         (linked from arc42 §6 Runtime View)
│
├─ Is it a per-module implementation spec (HOW)?
│   YES → Template 3 (Module Detailed Design)
│         Place in: detailed-design/<module>.md
│         (and link from arc42/05-building-blocks/)
│
├─ Is it the "big picture" of the whole system / a cross-cutting concern?
│   YES → Template 1 (Architecture Overview)
│         Place in: arc42/01-introduction-and-goals/overview.md
│         (or arc42/08-crosscutting/<concern>.md)
│
├─ Is it volatile (dated, append-only)?
│   ┌─ Implementation plan? → impl-plans/YYYY-MM-DD_<title>.md
│   ├─ Sprint task list?    → task-list/YYYY-MM-DD_<title>.md
│   ├─ Postmortem?          → postmortems/YYYY-MM-DD_<title>.md
│   ├─ Research report?     → reports/YYYY-MM-DD_<title>.md
│   └─ Cost estimate?       → cost-estimates/YYYY-MM_<title>.md
│
├─ Is it end-user documentation?
│   YES → user-manual/{tutorials|how-to|reference|explanation}/
│         (Diátaxis quadrant — see §3 below)
│
├─ Is it UX research output (optional, for product / design teams)?
│   ┌─ Persona definition?            → Template 6 → arc42/03-context-and-scope/personas/<id>.md
│   ├─ Customer / user journey map?   → Template 7 → arc42/03-context-and-scope/journeys/<id>.md
│   └─ Service blueprint (cross-functional service)? → Template 8 → arc42/03-context-and-scope/blueprints/<id>.md
│
└─ None of the above? → Template 0 (Default) in the closest matching directory
```

---

## 2. Code change → doc update mapping

When you change code, the **same PR** must update the corresponding doc. Use this table to know which file.

| Code change | Doc to update | Template |
|---|---|---|
| New / changed module or subsystem | `detailed-design/<module>.md` + arc42 §5 building-block index | Template 3 |
| New / changed public API | `api-contract/<module>.md` + the corresponding `detailed-design/<module>.md` API section | Template 3 |
| New architectural decision | New ADR in `arc42/09-decisions/NNNN-<title>.md` | Template 5 |
| Crosscutting concern (security, error model, observability) | `arc42/08-crosscutting/<concern>.md` | Template 1 |
| New runtime scenario / sequence | `arc42/06-runtime/<scenario>.md` | Template 4 |
| Quality / SLO / KPI change | `arc42/10-quality/` | Template 1 |
| Risk / technical debt surfaced | `arc42/11-risks/` | Template 0 |
| Glossary term added | `arc42/12-glossary/` | Template 0 |
| Diagram update | `diagrams/c4/workspace.dsl` (Structurizr DSL — single source of truth) | (no template) |
| New use-case | `arc42/03-context-and-scope/use-cases/<name>.md` | Template 4 |
| Operational / convention guideline | `design-guide/<topic>.md` | Template 0 |
| Implementation plan (dated) | `impl-plans/YYYY-MM-DD_<title>.md` | Template 0 |
| Sprint task breakdown | `task-list/YYYY-MM-DD_<title>.md` | Template 0 |
| Bug / incident retrospective (Medium+) | `postmortems/YYYY-MM-DD_<title>.md` | Template 0 |
| End-user manual chapter | `user-manual/{tutorials,how-to,reference,explanation}/` | (Diátaxis) |
| Cost change | `cost-estimates/YYYY-MM_<title>.md` | Template 0 |
| New persona (UX research) | `arc42/03-context-and-scope/personas/<id>.md` | Template 6 |
| New customer journey map | `arc42/03-context-and-scope/journeys/<id>.md` | Template 7 |
| New service blueprint | `arc42/03-context-and-scope/blueprints/<id>.md` | Template 8 |

PRs that change code without the corresponding doc update **must be sent back at review**.

---

## 3. Diátaxis quadrants for `user-manual/`

Pick the quadrant by what the user is trying to do.

| Quadrant | Reader's goal | Example | Local home |
|---|---|---|---|
| **Tutorial** | Learning by doing (newcomer) | "Get from zero to a working hello-world" | `user-manual/tutorials/` |
| **How-to** | Solving a specific problem (already competent) | "How do I rotate API keys?" | `user-manual/how-to/` |
| **Reference** | Looking something up (precise, dry) | "Full CLI flag list" | `user-manual/reference/` |
| **Explanation** | Understanding (background reading) | "Why we chose token-based auth" | `user-manual/explanation/` |

Authoritative source: <https://diataxis.fr>. Do not invent fifth quadrants.

---

## 4. Lifecycle

### Durable docs (`arc42/`, `detailed-design/`, `design-guide/`, `api-contract/`, `user-manual/`)

```
   ┌──────────┐  add to PR    ┌──────────┐  reviewer       ┌──────────┐
   │  Draft   │──────────────►│  Review  │────────────────►│   Done   │
   └──────────┘  (open Q ok)  └──────────┘  approves       └─────┬────┘
                                                                 │
                                                       new info ▼
                                                          ┌──────────────┐
                                                          │  Superseded  │
                                                          │ (link to     │
                                                          │  successor;  │
                                                          │  keep file)  │
                                                          └──────────────┘
```

**Rules:**

1. **Front-matter required.** Every durable doc starts with `status:`, `owner:`, `last-reviewed:`.
2. **`Done` is not "frozen forever"** — it means "approved as a citable record". Routine updates that do not change intent are fine; mark them with a revision-history bump.
3. **ADRs are immutable once `Accepted`.** If the decision changes, write a new ADR with `Supersedes: NNNN` and update the old one's front-matter to `Superseded by NNNN`. Never edit an Accepted ADR's body.
4. **Supersede over delete.** Old Done docs stay on disk (with `status: Superseded` + a link to the successor) so traceability survives.

### Volatile docs (`impl-plans/`, `task-list/`, `postmortems/`, `reports/`, `cost-estimates/`)

```
   ┌──────────┐  next entry        ┌──────────────┐
   │  Active  │───────────────────►│  Superseded  │
   └──────────┘  appends a new     │  (file stays │
                 dated file        │  on disk)    │
                                   └──────────────┘
```

Volatile docs **skip Review** — they are append-only by nature and superseded by the next dated entry. Never edit a closed volatile doc; write a new one.

---

## 5. Layer model (where things go)

| Layer | Directories | Change rate | Owner sensitivity |
|---|---|---|---|
| **A — Durable design** | `arc42/`, `diagrams/c4/`, `detailed-design/`, `design-guide/`, `api-contract/`, `user-manual/` | Slow | High (PR review required) |
| **B — Volatile working material** | `impl-plans/`, `task-list/`, `postmortems/`, `reports/`, `cost-estimates/` | Fast (dated) | Low (append-only) |
| **C — Reference & archive** | `archive/_legacy/`, third-party RAW data | Frozen | Read-only |

When in doubt, **Layer A is the default for anything that describes "how the system is built"**, Layer B for "what we did when".

---

## 5.1 Three-track completion model (the auditor reports each independently)

The four-phase rubric in [§4 Lifecycle](#4-lifecycle) covers the **architecture
completion track** only. Two additional tracks are reported separately by the
auditor and may legitimately be `N/A` for a given project state:

| Track | Phases | Applicability |
|-------|--------|---------------|
| **Architecture** (mandatory) | Phase 1 Discovery → Phase 2 Architecture → Phase 3 Detail → Phase 4 Operations | Always applicable. 100% target before any 1.0 release. |
| **Implementation companion** (conditional) | Phase 5 — `api-contract/` schemas + optional `design-guide/` conventions | Applicable when any PRD describes an HTTP/RPC interface, OR `api-contract/` / `design-guide/` has any content. Otherwise `N/A`. |
| **End-user docs / Diátaxis** (conditional) | Phase 6 — ≥ 1 file per Diátaxis quadrant under `user-manual/` | Applicable once UI ships (auditor heuristic: at least one non-README file under `user-manual/`). Before that: `N/A — UI not yet shipped`. |

Hitting **100% on Architecture alone is publish-ready as an architecture
package**. It is *not* the same as 100% project documentation. A typical
v1.0 deliverable is **Architecture 100% + Implementation 100% + End-user
≥ partial**. Track each separately so you know whether you are missing
"the architecture is unclear" vs "we never wrote the user manual".

---

## 6. Hard rules (never violate)

1. **One canonical location per topic.** If a concept appears in two files, one must be a link to the other.
2. **Front-matter on all durable docs** (`arc42/`, `detailed-design/`, `design-guide/`, `api-contract/`, `user-manual/`).
3. **Date prefix on all volatile docs** (`YYYY-MM-DD_<kebab-title>.md`).
4. **MADR for ADRs.** No homemade ADR formats.
5. **C4 single source of truth = `diagrams/c4/workspace.dsl`.** Image renders are gitignored.
6. **English by default.** Other languages are reserved for explicitly client-facing locations declared in `STRATEGY.md`.
7. **Repo-root-relative links.** Use `docs/<path>` form so links survive reorganization.
8. **No re-explaining external standards.** Link to <https://arc42.org> / <https://c4model.com> / <https://adr.github.io/madr/> / <https://diataxis.fr> instead of paraphrasing them locally.

---

## 7. The day in the life

| Action | Trigger | Doc(s) touched |
|---|---|---|
| Start a feature | New PRD requested | New file under §3 PRD area, status `Draft` |
| Decide tech stack | PRD review reveals an open question | New ADR under §9, status `Proposed` |
| Implement | PRD + ADR are `Accepted` | New `detailed-design/<module>.md`, update arc42 §5 building-block index |
| Add a public API | Detailed design names new endpoints | New / updated `api-contract/<module>.md` |
| Hit a runtime bug in prod | On-call paged | New `postmortems/YYYY-MM-DD_<title>.md` (volatile) |
| Reverse a previous decision | New evidence contradicts ADR-NNNN | New ADR with `Supersedes: NNNN`; old ADR's status flips to `Superseded by MMMM` |
| User-facing feature lands | Manual section is now stale | Update / add files under `user-manual/{quadrant}/` |
| Sprint kicks off | Planning meeting | New `task-list/YYYY-MM-DD_sprint-N.md` (volatile) |
| Quarterly cost review | Finance check-in | New `cost-estimates/YYYY-MM_<title>.md` (volatile) |

---

## 8. References

- arc42 — <https://arc42.org/overview/>
- C4 model — <https://c4model.com>
- MADR v3.0 — <https://adr.github.io/madr/>
- Diátaxis — <https://diataxis.fr>
- Google Design Doc — <https://www.industrialempathy.com/posts/design-docs-at-google/>
- Pragmatic Engineer RFC — <https://blog.pragmaticengineer.com/rfcs-and-architecture-decision-records/>
- Y-statements (Olaf Zimmermann) — <https://medium.com/olzzio/y-statements-10eb07b5a177>
