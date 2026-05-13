---
status: Stable
owner: tensor
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

## Index — incidents in this project

**As of 2026-05-14: zero incidents recorded.** No production incident has reached the severity threshold (Medium and above) because the project has no production users yet ([ADR-0010](../arc42/09-decisions/0010-refine-positioning-to-educational-first-production-capable.md) — as-is positioning).

When the first incident lands, the file naming convention is `YYYY-MM-DD_<kebab-title>.md` and the structure follows the seven sections above. Anticipated near-term incident categories (preregistered in [`../arc42/11-risks/overview.md`](../arc42/11-risks/overview.md) §5):

- PyPI / OIDC trusted-publishing failure during a release tag push.
- HuggingFace Space build timeout / network failure.
- A repeat of the four lessons-learned bug classes (R-P6 plotly MathJax, R-P7 un-executed notebook, R-P8 Sphinx source-tree) IF they reintroduce *with user impact* — the current arc42 §11 entries cover them as preregistered risk-class names rather than postmortems.

Expected fields when filing a postmortem:

- **Severity** (Medium / High / Critical) — per the threshold above.
- **Discovery** — how the incident was found (user report / CI / monitoring / chance).
- **Cross-link to the §11 risk row** if the incident matches a preregistered risk class. Update the row's "Active mitigations" column with the post-incident hardening.
