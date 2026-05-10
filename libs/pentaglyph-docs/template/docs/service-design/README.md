---
status: Stable
owner: <placeholder>
last-reviewed: 2026-05-09
---

# `service-design/` — TiSDD method bank home

This directory is the local home of **TiSDD** (*This Is Service Design Doing*,
Stickdorn et al., 2018) — the fifth canonical standard the pentaglyph kit
binds.

**Authoritative source**: <https://www.thisisservicedesigndoing.com/methods>.
The method bank's catalogue of personas, journey maps, service blueprints,
stakeholder maps, ecosystem maps, business model canvases, and the rest is
maintained at that URL and in the published book. **Do not re-author method
philosophy inside this repo** — link out instead. (This is the same rule
that applies to arc42, C4, MADR, and Diátaxis.)

## What this directory holds

- Per-service files documenting **specific service experiences** the project
  designs. One service per file (e.g. `online-checkout.md`, `appointment-booking.md`).
- Each file is composed from the artefact templates in
  [`../templates/`](../templates/):
  - [`6_persona.md`](../templates/6_persona.md) — TiSDD §"Personas" / Cooper "About Face" pattern.
  - [`7_journey-map.md`](../templates/7_journey-map.md) — TiSDD §"Journey Maps" / NN/g "User Journey Mapping".
  - [`8_service-blueprint.md`](../templates/8_service-blueprint.md) — TiSDD §"Service Blueprints" / Bitner-Ostrom-Morgan original.
- Cross-links into [`arc42/03-context-and-scope/`](../arc42/03-context-and-scope/)
  where the same actors / journeys appear at the architecture level.

## When to use this directory

- The service experience extends across multiple touchpoints (digital, physical, human-mediated) — a user-manual chapter alone cannot capture it.
- The decision being explored is about **how the service is experienced**, not **how the system is built**. (For "how is it built", use `arc42/`. For "how is it learned end-user-side", use `user-manual/`.)
- Stakeholder review needs a TiSDD-shaped artefact (journey map, blueprint) for non-technical reviewers — clinical leads, business operators, regulators.

## When NOT to use this directory

- Pure system-architecture concerns → `arc42/05-building-blocks/` or `detailed-design/`.
- Pure end-user learning content → `user-manual/` (Diátaxis quadrants).
- Customer support FAQ → `user-manual/explanation/` or `user-manual/how-to/`.

## File naming

`<service-or-flow>.md` (kebab-case). No date prefix — these are durable
records that supersede over delete.

## Lifecycle

Same as other Layer A durable docs (see [`../WORKFLOW.md` §4](../WORKFLOW.md)):
`Draft → Review → Done → Superseded`.

## Related

- [`../STRATEGY.md` §2](../STRATEGY.md) — the five-standard table.
- [`../templates/6_persona.md`](../templates/6_persona.md), [`7_journey-map.md`](../templates/7_journey-map.md), [`8_service-blueprint.md`](../templates/8_service-blueprint.md) — artefact templates.
- [`../arc42/03-context-and-scope/`](../arc42/03-context-and-scope/) — system-context counterpart of journey maps.
- TiSDD method bank: <https://www.thisisservicedesigndoing.com/methods>.
- TiSDD book (Stickdorn / Hormess / Lawrence / Schneider, O'Reilly, 2018).
