---
status: Stable
owner: <placeholder>
last-reviewed: 2026-05-04
---

# arc42 §9 — Architecture Decision Records (ADRs)

> Authoritative source for the section: <https://docs.arc42.org/section-9/>
> Authoritative source for the format: **MADR v3.0** — <https://adr.github.io/madr/>

## File naming

`NNNN-<kebab-title>.md` where `NNNN` is a zero-padded 4-digit sequence, globally unique across the project.

Examples:

- `0001-adopt-postgres-as-primary-store.md`
- `0002-use-jwt-for-service-to-service-auth.md`

## Template

Always start from [`../../templates/5_adr.md`](../../templates/5_adr.md). Do not invent a homemade ADR format.

## Status legend

| Status                | Meaning                                                                   |
| --------------------- | ------------------------------------------------------------------------- |
| **Proposed**          | Draft; under team review. Body may still be edited.                       |
| **Accepted**          | Ratified. **Body is immutable** — supersede with a new ADR, do not edit.  |
| **Rejected**          | Considered and explicitly rejected. Kept for traceability.                |
| **Superseded by NNNN**| Replaced by the cited ADR. Read the superseding ADR for the current decision. |
| **Deprecated**        | No longer relevant; no active replacement.                                |

## Authoring rules

1. **One file per decision.** No grouped ADRs.
2. **Status field** must be one of the values in the legend above.
3. **All external rationale** (regulatory documents, design specs, business analysis) must be absorbed into the ADR body or linked. Do not link to ephemeral chat / ticket comments as the only source.
4. **Date in ISO 8601** (`YYYY-MM-DD`).
5. **Once `Accepted`, body is immutable.** If the decision changes, write a new ADR with `Supersedes: NNNN`, then update the old ADR's status to `Superseded by MMMM`.
6. **Y-statement** required in the Decision Outcome section (Olaf Zimmermann form).
7. **Minimums**: 3 Decision Drivers, 2 Considered Options, Consequences in Positive / Negative / Neutral form, Compliance / Validation section.

## Index (your project keeps an Index here)

Maintain a section like the example below. Group ADRs by theme.

```markdown
### Runtime / SDK Foundation

| #     | File                                                | Title                                       | Status   | Date       |
| ----- | --------------------------------------------------- | ------------------------------------------- | -------- | ---------- |
| 0001  | [0001-...](./0001-...md)                            | <decision title>                            | Proposed | YYYY-MM-DD |
```

## References

- MADR v3.0 — <https://adr.github.io/madr/>
- Michael Nygard original (2011) — <https://www.cognitect.com/blog/2011/11/15/documenting-architecture-decisions>
- Y-statements (Olaf Zimmermann) — <https://medium.com/olzzio/y-statements-10eb07b5a177>
- arc42 §9 — <https://docs.arc42.org/section-9/>
