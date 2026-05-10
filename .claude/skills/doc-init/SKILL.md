---
name: doc-init
description: >
  Start (or resume) the conversational documentation build. Invokes the
  doc-orchestrator agent which audits the current docs/ state, identifies the
  current phase + next gap, and begins / resumes the discovery interview. Idem-
  potent — safe to call repeatedly. Use when a project has just been scaffolded
  with `pentaglyph init` or when picking up after a long pause.
disable-model-invocation: false
---

# /doc-init — start (or resume) conversational doc build

Use this skill when you want to **build (or continue building) the project's
docs/ tree through conversation** with the doc-orchestrator.

## What it does

1. Hands off to the **`doc-orchestrator`** agent
2. The orchestrator runs the `completeness-auditor` to see current state
3. The orchestrator decides the current phase (1 Discovery / 2 Architecture
   / 3 Detail / 4 Operations / complete) and shows you the coverage report
4. The orchestrator asks the next focused question (one at a time)
5. As answers accumulate, the orchestrator dispatches specialist agents
   (`discovery-agent`, `architect-agent`, `adr-writer`, `spec-writer`) to
   write each artefact
6. After every dispatch, the auditor re-runs and the orchestrator re-decides

The conversation continues until the auditor reports **100% coverage**.

## When to use

- **Fresh project**: just ran `pentaglyph init`, the docs/ scaffold has only
  README.md files. → `/doc-init` to start Phase 1 (Discovery)
- **Resuming**: a prior conversation got partway through. → `/doc-init` again;
  the orchestrator picks up exactly where you left off (state is computed
  from disk, not stored anywhere)
- **Recovering from a hand-edit**: you edited some docs by hand and want to
  know what's still left. → `/doc-init` will re-audit and tell you

## When NOT to use

- You only want a status check, no conversation → use `/doc-status` instead
- You know the specific gap you want to fill → use `/doc-fill <area>` instead
- You're documenting a single bug fix or one-shot change → just talk to
  Claude directly; no orchestration needed

## Output

The orchestrator's first turn always includes a coverage table like:

```text
PHASE: 2
COVERAGE: 35%
SUBSTANTIVE-FILES: 8 / 22

ARTEFACTS:
  PHASE-1:
    ✓ docs/arc42/01-introduction-and-goals/overview.md: 7 goals, 4 stakeholders, 5 quality goals
    ✓ docs/arc42/03-context-and-scope/system-context.md: 2 actors, 3 external systems
    ✓ docs/arc42/03-context-and-scope/prds/auth-prd.md: 4 FRs, 3 NFRs
    ✗ no use cases yet
  PHASE-2:
    ⚠ docs/arc42/04-solution-strategy/strategy.md: only 2 decisions, need ≥ 5
    ✗ docs/arc42/05-building-blocks/overview.md: missing
    ✗ docs/diagrams/c4/workspace.dsl: still in starter state
    ⚠ docs/arc42/09-decisions/: only 1 ADR, need ≥ 3

NEXT-GAP: write a use case for the primary auth flow
NEXT-GAP-PATH: docs/arc42/03-context-and-scope/use-cases/login.md
NEXT-GAP-TEMPLATE: 4 (Use Case)
NEXT-GAP-AGENT: discovery-agent
NEXT-GAP-MISSING: actor, main success scenario steps, ≥ 1 Given/When/Then AC
```

Followed by a single focused question targeting that gap.

## Implementation note

This skill is a thin pointer. The actual logic lives in
`.claude/agents/doc-orchestrator.md`. Edit the orchestrator's prompt to
change behaviour; this skill file should not need updates.

---

## See also

- `/doc-status` — coverage check without starting a conversation
- `/doc-fill <area>` — fill one specific gap directly
- [`docs/AGENTS_OVERVIEW.md`](../../../docs/AGENTS_OVERVIEW.md) and
  [`.claude/agents/README.md`](../../agents/README.md) — full agent system docs
