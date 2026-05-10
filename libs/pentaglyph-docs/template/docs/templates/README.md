---
status: Stable
owner: <placeholder>
last-reviewed: 2026-05-04
---

# Template Index

Nine templates cover the full authoring surface (six core + three UX research). Pick one before writing.

> For *when to write what and where to put it*, see [`../WORKFLOW.md`](../WORKFLOW.md). This README is a template index only.

The template set is synthesised from industry references: [arc42](https://arc42.org/), [C4 Model](https://c4model.com/), [MADR v3.0](https://adr.github.io/madr/), [Google Design Doc](https://www.industrialempathy.com/posts/design-docs-at-google/), [Lenny PRD](https://www.lennysnewsletter.com/p/how-the-most-successful-pms-write), [Cockburn Use Cases](https://www.craiglarman.com/wiki/downloads/cockburn-use-case-fundamentals.pdf), [Cooper Personas](https://www.cooper.com/journal/2008/05/the_origin_of_personas), [Kalbach Mapping Experiences](https://www.oreilly.com/library/view/mapping-experiences/9781491923528/), [Bitner Service Blueprints](https://digitalcommons.usu.edu/cgi/viewcontent.cgi?article=1041&context=marketing_facpub), and [Diátaxis](https://diataxis.fr/).

---

## Template list

| #   | Template                                                       | Type                            | arc42 mapping            | Question it answers                                |
| --- | -------------------------------------------------------------- | ------------------------------- | ------------------------ | -------------------------------------------------- |
| 0   | [Default](0_default.md)                                        | Generic detailed design         | (any)                    | Fallback when none of 1–5 fit                      |
| 1   | [Architecture Overview](1_architecture-overview.md)            | System-wide / cross-cutting     | §1 + §3 + §4 + §5 + §8   | "How does the whole thing work?"                   |
| 2   | [PRD](2_prd.md)                                                | Product requirement spec        | (under §3 PRD area)      | "Who, for what purpose, is asking for what?"       |
| 3   | [Module Detailed Design](3_module-detailed-design.md)          | Implementation spec             | §5 Building Block        | "How is it implemented / what does the API look like?" |
| 4   | [Use Case](4_use-case.md)                                      | Scenario                        | §6 Runtime View          | "Which actor performs which operations?"           |
| 5   | [ADR (MADR v3.0)](5_adr.md)                                    | Design decision                 | §9 Decisions             | "Why was this chosen / what was rejected?"         |
| 6   | [Persona](6_persona.md)                                        | UX research — goal-directed persona | §3 Context (personas/) | "Who is this for, and what do they want?"          |
| 7   | [Journey Map](7_journey-map.md)                                | UX research — experience over time  | §3 Context (journeys/) / §6 Runtime | "How does this persona experience the scenario, stage by stage?" |
| 8   | [Service Blueprint](8_service-blueprint.md)                    | UX research — frontstage / backstage | §3 Context (blueprints/) / §6 Runtime | "What do we do behind the scenes to make the journey happen?"    |

## How to choose a template

```text
Before writing a document
    │
    ├─ Does it cover the "big picture" or a cross-cutting concern across modules?
    │   YES → Type 1: Architecture Overview
    │
    ├─ Is it centered on "what users want / what we will build"? (WHAT/WHY rather than HOW)
    │   YES → Type 2: PRD
    │
    ├─ Does it describe behaviour for a specific actor / scenario?
    │   YES → Type 4: Use Case
    │
    ├─ Does a single important design decision need to be recorded independently?
    │   YES → Type 5: ADR (standalone file under `arc42/09-decisions/`)
    │   NO  → Append a lightweight ADR section inside the relevant Module Detailed Design
    │
    └─ Otherwise (HOW / implementation focused)
        → Type 3: Module Detailed Design
```

---

## Section × mandatory/optional matrix

Cross-reference table consolidated from the industry survey. Single source of truth when modifying templates.

| Section                                | Type 1<br>Arch Overview | Type 2<br>PRD | Type 3<br>Tech Design | Type 4<br>Use Case | Type 5<br>ADR |
| -------------------------------------- | :---------------------: | :-----------: | :-------------------: | :----------------: | :-----------: |
| Title / Status / Owner / Date          | M                       | M             | M                     | M                  | M             |
| Background / Context                   | M                       | M             | M                     | M                  | M             |
| Problem statement                      | O                       | M             | O                     | M                  | M             |
| Goals                                  | M                       | M             | M                     | —                  | —             |
| **Non-Goals** (mandated by Google)     | O                       | **M**         | **M**                 | —                  | —             |
| Target users / Personas                | O                       | M             | O                     | M (Actors)         | —             |
| Decision drivers                       | O                       | —             | O                     | —                  | M             |
| Considered options / Alternatives      | O                       | O             | M                     | —                  | M             |
| Decision outcome                       | —                       | —             | —                     | —                  | M             |
| Consequences / Trade-offs              | O                       | O             | M                     | —                  | M             |
| Proposed design / Solution             | M                       | —             | M                     | —                  | —             |
| System context diagram (C4 L1)         | M                       | —             | M                     | —                  | —             |
| Container diagram (C4 L2)              | M                       | —             | O                     | —                  | —             |
| Data model                             | O                       | —             | M                     | —                  | —             |
| API contract                           | O                       | —             | M                     | —                  | —             |
| Main flow / Scenario                   | —                       | —             | —                     | M                  | —             |
| Alternative / Exception flows          | —                       | —             | —                     | M                  | —             |
| Acceptance criteria (Given/When/Then)  | —                       | M             | O                     | M                  | —             |
| Success metrics / KPI                  | O                       | M             | O                     | O                  | —             |
| Out of scope                           | O                       | M             | M                     | O                  | —             |
| Quality attributes / NFR               | M                       | O             | O                     | —                  | —             |
| Cross-cutting (security, observability)| M                       | —             | M                     | —                  | —             |
| Risks / Open questions                 | M                       | M             | M                     | —                  | O             |
| Rollout / Migration plan               | —                       | O             | O                     | —                  | —             |
| References / Links                     | M                       | M             | M                     | O                  | M             |

Legend: M = Mandatory / O = Optional (recommended) / — = N/A

---

## Common rules

1. **Front-matter required** — every template begins with YAML front-matter declaring `status` (Draft/Review/Done/Superseded), `owner`, and `last-reviewed`.
2. **Non-Goals are not optional** (Types 2 and 3) — Google reports this as the most frequent reason design docs are rejected.
3. **ADRs are immutable** — once Accepted, do not edit; supersede with a new ADR instead. See [MADR](https://adr.github.io/madr/).
4. **C4 diagrams in Mermaid** (`C4Context` / `C4Container`) — render inline in Markdown. See [C4 Model](https://c4model.com/) and [Mermaid C4 plugin](https://mermaid.js.org/syntax/c4.html).
5. **Language** — English by default per [`../STRATEGY.md`](../STRATEGY.md). Other languages reserved for explicitly designated client-facing directories.
6. **Ticket linkage** — when a related ticket exists, place its identifier in the `Related` field (`#NNN` for GitHub Issues, `KEY-NNN` for Jira, `#AB#NNNN` for Azure DevOps Boards).

## How to update a template

1. Update the section matrix in this README.
2. Update the relevant template file.
3. Grep existing documents for references that may need updating.
4. Record the change in an implementation note under `../impl-plans/` or in an ADR under `../arc42/09-decisions/`.

## References

- [arc42](https://arc42.org/overview) / [§8 Crosscutting Concepts](https://docs.arc42.org/section-8/)
- [C4 Model](https://c4model.com/) / [Mermaid C4 plugin](https://mermaid.js.org/syntax/c4.html)
- [MADR v3.0](https://adr.github.io/madr/) / [Y-statements (Olaf Zimmermann)](https://medium.com/olzzio/y-statements-10eb07b5a177)
- [Google Design Doc (Industrial Empathy)](https://www.industrialempathy.com/posts/design-docs-at-google/)
- [Pragmatic Engineer RFC](https://blog.pragmaticengineer.com/rfcs-and-architecture-decision-records/)
- [Lenny Rachitsky PRD](https://www.lennysnewsletter.com/p/how-the-most-successful-pms-write)
- [Cockburn Use Cases](https://www.amazon.com/Writing-Effective-Cases-Alistair-Cockburn/dp/0201702258)
- [Mike Cohn User Stories (INVEST)](https://www.mountaingoatsoftware.com/agile/user-stories)
- [Diátaxis](https://diataxis.fr/)
- [ADR community collection (Joel Parker Henderson)](https://github.com/joelparkerhenderson/architecture-decision-record)
