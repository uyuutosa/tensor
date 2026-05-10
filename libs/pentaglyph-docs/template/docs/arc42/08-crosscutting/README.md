---
status: Draft
owner: <placeholder>
last-reviewed: <YYYY-MM-DD>
---

# arc42 §8 — Crosscutting Concepts

> Authoritative source: <https://docs.arc42.org/section-8/>
>
> This section answers: *what design rules apply across multiple building blocks?*

## What lives here

One file per concern. Common concerns:

| Suggested file              | Purpose                                                                    |
| --------------------------- | -------------------------------------------------------------------------- |
| `error-strategy.md`         | Error model, retry policy, circuit breakers, user-facing error contracts   |
| `auth-and-pii.md`           | Authentication, authorization, PII handling, data minimisation             |
| `observability.md`          | Logging, metrics, tracing, alerting conventions                            |
| `performance.md`            | Performance budgets, hot-path conventions, caching strategy                |
| `state-management.md`       | Where state lives (client / server / both), idempotency rules              |
| `internationalisation.md`   | i18n / l10n conventions                                                    |
| `accessibility.md`          | A11y baseline (WCAG level, screen-reader contract)                         |
| `frontend-architecture.md`  | Frontend (React/Next.js): Server vs Client State, FSD layout, light DDD    |

Add files only for concerns that **actually span ≥ 2 building blocks**. A concern relevant to a single building block belongs in that building block's detailed-design file.

## How to write here

1. Use Template 0 (default) or Template 1 trimmed.
2. State the **rule** (the convention every building block must follow), then the **rationale** (why this rule, not alternatives).
3. Link out to library / framework docs rather than re-explaining how the underlying tool works.

For lifecycle / when to update, see [`../../WORKFLOW.md`](../../WORKFLOW.md).

## Cross-references

- §5 Building Blocks — each building block's detailed-design file declares which §8 concerns apply to it
- §9 Decisions — ADRs that justify a §8 rule (e.g. "Adopt OpenTelemetry as the tracing standard")
- §10 Quality — quality scenarios that motivate §8 rules
