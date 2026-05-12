---
status: Draft
owner: <placeholder>
last-reviewed: <YYYY-MM-DD>
---

# arc42 §11 — Risks and Technical Debt

> Authoritative source: <https://docs.arc42.org/section-11/>
>
> This section answers: *what could go wrong, and what shortcuts are we knowingly taking?*

## What lives here

| Suggested file         | Purpose                                                                       |
| ---------------------- | ----------------------------------------------------------------------------- |
| `risk-register.md`     | Active risks with likelihood × impact × owner × mitigation                    |
| `technical-debt.md`    | Known shortcuts, with the cost of paying them off and the trigger for doing so |
| `open-questions.md`    | Unresolved questions that block decisions                                     |

## How to write here

1. Be **honest**. Hidden risks become incidents. Known risks become managed risks.
2. Each risk: *what could happen* / *how likely* / *what would the impact be* / *who owns the mitigation* / *trigger condition for action*.
3. Each tech-debt entry: *what shortcut was taken* / *what the right answer would be* / *what the cost would be to fix* / *what would justify paying that cost*.
4. Move resolved items out of this section (don't accumulate forever) — link them in the relevant ADR's Consequences section instead.

For lifecycle / when to update, see [`../../WORKFLOW.md`](../../WORKFLOW.md).

## Cross-references

- §2 Constraints — constraints that create risks
- §9 Decisions — ADRs that introduce or accept risks (linked from each ADR's Consequences section)
- §10 Quality — quality targets at risk
