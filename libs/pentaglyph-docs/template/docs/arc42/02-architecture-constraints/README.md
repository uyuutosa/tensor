---
status: Draft
owner: <placeholder>
last-reviewed: <YYYY-MM-DD>
---

# arc42 §2 — Architecture Constraints

> Authoritative source: <https://docs.arc42.org/section-2/>
>
> This section answers: *what is fixed and cannot be negotiated by architecture?*

## What lives here

One file per constraint family. Suggested:

| Suggested file                | Purpose                                                              |
| ----------------------------- | -------------------------------------------------------------------- |
| `technical-constraints.md`    | Mandated runtimes, frameworks, languages, cloud providers            |
| `organisational-constraints.md` | Team size, skill mix, sprint cadence, on-call rotations            |
| `regulatory-constraints.md`   | Laws, certifications, data-residency, audit requirements             |

Add a `README.md` link to each file you create.

## How to write here

1. Record **only what is genuinely fixed** — things that the team cannot change. If something is up for debate, it is a Decision (§9), not a constraint.
2. Each constraint must include: *what it is*, *who imposes it*, *when it expires* (if ever).
3. Keep brief. One bullet per constraint. Cross-link to the source document (regulation, contract, vendor agreement).

For lifecycle / when to update, see [`../../WORKFLOW.md`](../../WORKFLOW.md).

## Cross-references

- §1 Introduction — goals that these constraints might block
- §4 Solution Strategy — strategy must respect these constraints
- §11 Risks — risks created by these constraints
