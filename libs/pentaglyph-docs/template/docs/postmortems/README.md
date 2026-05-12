---
status: Stable
owner: <placeholder>
last-reviewed: 2026-05-04
---

# postmortems — incident retrospectives

> **Layer B (volatile, dated, append-only).** Severity threshold: **Medium and above**. Trivial bugs do not get a postmortem.

## File naming

`YYYY-MM-DD_<kebab-title>.md`

Example: `2026-05-04_payment-webhook-deadletter.md`

## Recommended structure (per file)

1. **Summary** — one paragraph: what happened, when, who was affected.
2. **Timeline** — UTC timestamps from first symptom to full resolution.
3. **Root cause** — five whys. Stop only when you reach a system-design or process root, not a person.
4. **Impact** — quantified (# users affected, $ revenue lost, SLO budget consumed).
5. **What went well** — honest positives. Detection time, communication, rollback.
6. **What went poorly** — honest negatives. Detection gaps, paging gaps, knowledge gaps.
7. **Action items** — concrete, owned, dated. Each with a ticket link.

## Cultural rules

- **Blameless.** Name systems and processes, never individuals.
- **Action items must have owners and due dates** — otherwise they will not happen.
- **Link from the relevant ADR's Consequences section** if the incident invalidates a previous decision.

For lifecycle, see [`../WORKFLOW.md`](../WORKFLOW.md).
