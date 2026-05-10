---
name: doc-status
description: >
  Read-only documentation coverage report. Invokes the completeness-auditor
  agent to inventory docs/ against the four-phase rubric and prints the current
  phase + coverage % + the single highest-priority next gap. Does not start a
  conversation. Use after manual edits, before commits, or as a quick health
  check.
disable-model-invocation: false
---

# /doc-status — coverage report

Use this skill when you want a **quick, conversation-free check** of
how complete the project's docs/ tree is.

## What it does

1. Invokes the **`completeness-auditor`** agent (read-only, no side effects)
2. The auditor walks `docs/` and checks every required artefact against the
   substance rubric
3. Returns a structured report: phase, coverage %, per-artefact status,
   inconsistencies, the single next gap

This skill **does not** trigger any writes or any conversation. It's safe to
run any time, in any state.

## When to use

- **Pre-commit gut check**: am I shipping with 60% coverage when policy is
  85%?
- **Resuming after a long pause**: what state is the project in?
- **After hand-edits**: I edited overview.md manually — does it still pass
  the rubric?
- **In CI**: pipe the output to a check (`grep "COVERAGE: 100%"` or
  `grep -v "INCONSISTENCIES: none"`)

## When NOT to use

- You want to start filling gaps in conversation → use `/doc-init` instead
- You want to fill one specific known gap → use `/doc-fill <area>` instead

## Output

```text
PHASE: 3
COVERAGE: 72%
SUBSTANTIVE-FILES: 16 / 22

ARTEFACTS:
  PHASE-1:
    ✓ docs/arc42/01-introduction-and-goals/overview.md: 7 goals, 4 stakeholders, 5 quality goals
    ✓ docs/arc42/03-context-and-scope/system-context.md: 2 actors, 3 external systems
    ✓ docs/arc42/03-context-and-scope/prds/auth-prd.md: 4 FRs, 3 NFRs
    ✓ docs/arc42/03-context-and-scope/use-cases/login.md: 3 ACs
  PHASE-2:
    ✓ docs/arc42/04-solution-strategy/strategy.md: 6 decisions, all linked
    ✓ docs/arc42/05-building-blocks/overview.md: 4 containers
    ✓ docs/diagrams/c4/workspace.dsl: matches building-blocks
    ✓ docs/arc42/09-decisions/: 5 ADRs (all with Y-statements)
  PHASE-3:
    ✓ docs/detailed-design/auth-service.md: 4200 chars, 4.2/4.3/5 filled
    ✗ docs/arc42/06-runtime/: no scenarios yet
    ⚠ docs/arc42/08-crosscutting/: 1 file, need ≥ 2
  PHASE-4:
    ✗ docs/arc42/07-deployment/deployment.md: missing
    ✗ docs/arc42/10-quality/slos.md: missing
    ✗ docs/arc42/11-risks/risk-register.md: missing
    ✗ docs/arc42/12-glossary/glossary.md: missing

INCONSISTENCIES:
  (none)

NEXT-GAP: write the primary runtime scenario (login flow)
NEXT-GAP-PATH: docs/arc42/06-runtime/01-user-login.md
NEXT-GAP-TEMPLATE: 4
NEXT-GAP-AGENT: spec-writer (with runtime-only scope)
NEXT-GAP-MISSING: scenario steps, sequence diagram, references to existing building blocks

SUMMARY: Phase 3 mostly complete; needs one runtime scenario and one more
crosscutting concern. After that, Phase 4 (operations) is all-greenfield.
```

## Implementation note

This skill is a thin pointer. The auditor's actual checklist lives in
`.claude/agents/completeness-auditor.md`. Edit the auditor's rubric to
change which artefacts are required; this skill file should not need
updates.

---

## See also

- `/doc-init` — start a conversation that fills the next gap
- `/doc-fill <area>` — fill one specific gap directly without orchestration
- `.claude/agents/README.md` — full rubric explanation
