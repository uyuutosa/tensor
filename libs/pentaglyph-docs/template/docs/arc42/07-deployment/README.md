---
status: Draft
owner: <placeholder>
last-reviewed: <YYYY-MM-DD>
---

# arc42 §7 — Deployment View

> Authoritative source: <https://docs.arc42.org/section-7/>
>
> This section answers: *where does the software run, and how does it get there?*

## What lives here

| Suggested file   | Purpose                                                               |
| ---------------- | --------------------------------------------------------------------- |
| `deployment.md`  | Environment topology (dev / staging / prod), regions, infra diagram   |
| `tenant-model.md` | Multi-tenancy strategy (single-tenant / pooled / per-tenant infra)   |

Add one file per distinct deployment target if the system spans multiple (e.g. `edge.md`, `mobile.md`).

## How to write here

1. Use Template 1 (architecture-overview) trimmed to the §7 portion, or Template 0 if simpler.
2. Show the **infrastructure**, not the code: cloud regions, networking boundaries, data flows across environments.
3. Link out to the IaC source (Terraform / Pulumi / CDK) — do not duplicate the IaC content.
4. Include rollback / blue-green / canary strategy if relevant.

For lifecycle / when to update, see [`../../WORKFLOW.md`](../../WORKFLOW.md).

## Cross-references

- §5 Building Blocks — the building blocks being deployed
- §8 Crosscutting — observability / security cross-cutting that lives in deployment
- §10 Quality — availability / DR / latency targets that drive deployment choices
