---
status: Draft
owner: <placeholder>
last-reviewed: <YYYY-MM-DD>
---

# ADR Template (Type 5 / MADR v3.0)

> **Copy to use**: `cp templates/5_adr.md arc42/09-decisions/NNNN-<title-kebab-case>.md`
> Recommended location: `arc42/09-decisions/` (cross-cutting ADRs); inside §7 of a Module Detailed Design (local ADRs, lightweight form)
> Reference: [MADR v3.0](https://adr.github.io/madr/) + [Y-statements (Olaf Zimmermann)](https://medium.com/olzzio/y-statements-10eb07b5a177) + [Michael Nygard original (2011)](https://cognitect.com/blog/2011/11/15/documenting-architecture-decisions)
> Length target: **1–3 pages**
>
> **Immutable rule**: once Accepted, do not edit. If the decision changes, create a new ADR and link via `Superseded by ADR-XXXX`.
> **Numbering**: `0001-`, `0002-` zero-padded to 4 digits, globally unique across the project.
> **Title**: start with a verb (e.g. "Adopt PostgreSQL as primary store").
>
> Delete this `> ...` guidance block after copying.

---

# ADR-NNNN: <Decision title (verb-led)>

| Metadata  | Value                                                          |
| --------- | -------------------------------------------------------------- |
| Status    | **Proposed** / Accepted / Deprecated / Superseded by ADR-XXXX  |
| Type      | ADR (Type 5 / MADR v3.0 / arc42 §9)                            |
| Date      | YYYY-MM-DD                                                     |
| Deciders  | <PO / Tech Lead / owner>                                       |
| Consulted | <people consulted>                                             |
| Informed  | <people notified>                                              |
| Ticket    | <placeholder>                                                  |

---

## Context and Problem Statement

<!-- The situation in which a decision is needed. 1–3 paragraphs that frame the *problem*. -->

<e.g. The system requires a primary datastore. Among options such as PostgreSQL, MySQL, and DynamoDB, we must select one for adoption.>

---

## Decision Drivers

<!-- ★ MADR-required trade-off drivers.
3–5 factors that drive the decision, in priority order. -->

- **DD-1**: <e.g. operational maturity in our cloud provider>
- **DD-2**: <e.g. cost per GB-month at expected scale>
- **DD-3**: <e.g. team's existing expertise>
- **DD-4**: <e.g. long-term maintainability>

---

## Considered Options

<!-- 2–5 candidates. Title + one-line summary; details below in Pros/Cons. -->

1. **<Option A>**: <one-liner>
2. **<Option B>**: <one-liner>
3. **<Option C>**: <one-liner>

---

## Decision Outcome

**Chosen option: <Option X>**

<!-- Why X — link the rationale to the Decision Drivers in 2–3 paragraphs. -->

<Reasoning, anchored to Decision Drivers.>

### Y-statement summary (Olaf Zimmermann form, one sentence)

> In the context of **<use case>**, facing **<concern>**, we decided for **<option X>** to achieve **<quality>**, accepting **<downside>**.

---

## Pros and Cons of the Options

### Option A: <name>

- Pros:
  - <pro 1>
  - <pro 2>
- Cons:
  - <con 1>
  - <con 2>

### Option B: <name> (chosen)

- Pros:
  - <pro 1>
  - <pro 2>
- Cons:
  - <con 1>

### Option C: <name>

- Pros:
  - …
- Cons:
  - …

---

## Consequences

### Positive

- <positive impact>

### Negative

- <negative impact / cost we accept>

### Neutral

- <neutral impact>

### Follow-ups

- [ ] <follow-up task 1> (<placeholder ticket id>)
- [ ] <follow-up task 2>

---

## Compliance / Validation (optional)

<!-- How we will verify this decision is being upheld in the future. -->

- Verification: <e.g. static analysis confirms no `import legacy_db` remains>
- Frequency: <e.g. pre-commit hook>

---

## More Information

### Related ADRs

- Prerequisite ADR: [ADR-XXXX](XXXX-<placeholder>.md)
- Complementary ADR: [ADR-YYYY](YYYY-<placeholder>.md)
- Dissent / rejected alternative: [ADR-ZZZZ](ZZZZ-<placeholder>.md) (rejected)

### References

- <official documentation / academic paper / blog post / GitHub issue>

---

## Lightweight form (for use inside a Module Detailed Design §7)

> **This template is the full MADR v3.0 form for cross-cutting ADRs.**
> Local ADRs that live inside a Module Detailed Design §7 should use the lightweight form below:

```markdown
### ADR-NN: <title> (YYYY-MM-DD)
**Context**: <1–2 sentences>
**Decision**: <what was chosen>
**Alternatives considered and rejected**: A → reason X; B → reason Y
**Consequences**: <expected impact>
```

Heuristic for using the lightweight form: "Will my future self in six months ask 'why is this the way it is?'" If YES, write it down.
