---
name: doc-fill
description: >
  Fill one specific documentation gap directly, bypassing the orchestrator.
  Takes an `<area>` argument (e.g. `prd:auth`, `adr:adopt-postgres`,
  `module:billing-service`, `runtime:checkout-flow`, `slos`, `glossary`) and
  dispatches the appropriate specialist agent. Useful when you know exactly
  what you want next. Does not run the auditor first.
argument-hint: "<area>  e.g. prd:auth | adr:replace-mysql | module:billing-service | runtime:login | slos | risks | glossary"
disable-model-invocation: false
---

# /doc-fill <area> — fill one specific gap

Use this skill when you **already know what artefact you want to write**
and don't need the orchestrator's full audit + interview loop.

## Argument syntax

```text
/doc-fill <type>:<id>
/doc-fill <bare-type>          (for types that are singletons)
```

| `<area>` | Dispatches | Writes to |
|----------|-----------|-----------|
| `intro` | `discovery-agent` | `docs/arc42/01-introduction-and-goals/overview.md` |
| `context` | `discovery-agent` | `docs/arc42/03-context-and-scope/{business,system}-context.md` |
| `prd:<feature>` | `discovery-agent` | `docs/arc42/03-context-and-scope/prds/<feature>.md` |
| `usecase:<name>` | `discovery-agent` | `docs/arc42/03-context-and-scope/use-cases/<name>.md` |
| `strategy` | `architect-agent` | `docs/arc42/04-solution-strategy/strategy.md` |
| `building-blocks` | `architect-agent` | `docs/arc42/05-building-blocks/overview.md` + `docs/diagrams/c4/workspace.dsl` |
| `adr:<title-kebab>` | `adr-writer` | `docs/arc42/09-decisions/NNNN-<title>.md` |
| `module:<name>` | `spec-writer` | `docs/detailed-design/<name>.md` |
| `runtime:<scenario>` | `spec-writer` | `docs/arc42/06-runtime/NN-<scenario>.md` |
| `crosscutting:<concern>` | `architect-agent` | `docs/arc42/08-crosscutting/<concern>.md` |
| `deployment` | `architect-agent` | `docs/arc42/07-deployment/deployment.md` |
| `slos` | `architect-agent` | `docs/arc42/10-quality/slos.md` |
| `risks` | `architect-agent` | `docs/arc42/11-risks/risk-register.md` |
| `glossary` | `architect-agent` | `docs/arc42/12-glossary/glossary.md` |

## When to use

- **You have a specific gap in mind**: "write the auth PRD"
- **You're filling in after the orchestrator stalled**: e.g. orchestrator
  said `INSUFFICIENT BRIEF` for a particular file and you have the answers
  now
- **You're maintaining**: a new feature lands → you want one new ADR + one
  new module spec, no ceremony
- **You want fast turnaround**: skip the audit overhead

## When NOT to use

- You don't know what's missing → use `/doc-status` first
- You want guided discovery from zero → use `/doc-init` instead
- You want the orchestrator to keep state between dispatches → use
  `/doc-init` (this skill is a one-shot)

## Behaviour

1. The skill parses `<area>` and identifies the target agent
2. The user is prompted (one focused turn) to provide the minimum brief
   the specialist needs (e.g. for `adr:adopt-postgres`: drivers, options,
   chosen rationale)
3. The skill dispatches the specialist with that brief
4. The specialist writes the file and returns its standard structured
   summary
5. The skill suggests running `/doc-status` to verify the gap closed

## Examples

```text
/doc-fill prd:authentication
  → discovery-agent asks: "What is the user story for auth, in one sentence?
    What 3 functional requirements are MVP? What is the availability NFR?"
  → writes docs/arc42/03-context-and-scope/prds/authentication.md
  → suggests: "/doc-status to verify; or /doc-fill usecase:login next"

/doc-fill adr:choose-event-broker
  → adr-writer asks: "What are the 3–5 drivers? What 2–3 options were
    considered? Which won and why? Y-statement?"
  → writes docs/arc42/09-decisions/0007-choose-event-broker.md
  → suggests: "/doc-status to verify; remember to update strategy.md"

/doc-fill module:notification-service
  → spec-writer asks: "What does this module expose (API methods)?
    What data does it own? What are its 2 worst failure modes? What's
    one rejected alternative design?"
  → writes docs/detailed-design/notification-service.md
  → suggests: "/doc-fill runtime:notification-delivery if you want a
    sequence diagram for the main flow"
```

## Implementation note

This skill is a router. It dispatches to existing specialist agents
defined in `.claude/agents/`. Edit those agents to change writing
behaviour; this skill only handles dispatch routing.

---

## See also

- `/doc-init` — full orchestrated conversation
- `/doc-status` — coverage check
- `.claude/agents/README.md` — agent inventory + rubric
