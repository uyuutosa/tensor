---
paths:
  - "**"
---

# Narrative Consultative Dialogue Rule (auto-loaded globally)

This rule governs how the AI **talks to the user** during open-ended discussion, design exploration, and requirements gathering. It does not apply to direct execution requests ("fix this bug", "rename this function"); for those, just do the work.

## When to engage narrative mode

Engage when the user signals exploration rather than execution:

- "What do you think about ...?"
- "Let's discuss ..."
- "Help me decide ..."
- "Is this the right time to ...?"
- "Can we talk through ...?"
- The user describes a problem space without prescribing the solution
- The user asks a question whose best answer is "it depends, on these things"

Do **not** engage narrative mode when:

- The user gives a direct command ("write the report", "run the tests")
- The user is mid-execution and just needs a status update
- The task is trivially scoped

## The 4-beat rhythm

Every consultative turn follows this shape:

1. **Acknowledge / synthesise (1–3 sentences).** Restate what you understood so the user can correct misalignment cheaply, before you sink effort into the wrong direction.
2. **Set context (1–3 sentences).** Frame why the question matters now. The user should be able to answer cold without re-reading earlier messages.
3. **Ask one focused question.** One concrete decision point per turn. If two are coupled, name the coupling and still pick one to lead with.
4. **Scaffold the answer space.** Offer 2–4 candidate answers (A / B / C) with the trade-off in one line each. The user can pick, modify, or reject the framing entirely. Never present scaffolding as exhaustive — close with "or something I'm not seeing".

## Why "one question at a time"

Bundling questions feels efficient but is the most common cause of misalignment in design conversations. The user picks the one they have an opinion on and the rest get blurred. Then you build on a half-answered question and the gap compounds.

If you have N questions, ask the most blocking one. The answer often eliminates or reshapes the others.

## Worked examples

### Bad (question dump)

> Should we use Tailwind or Material-UI? Should the iPad be landscape or portrait? Is the medical console a separate app? Should HITL be modal or inline?

The user has to context-switch four times. They will answer one and ignore the rest.

### Good (one question, scaffolded)

> Tailwind か Material-UI かは、その下の component library を A2UI と並走させる前提だと結構効いてきます。3 案あって：
>
> - A. Tailwind + Headless UI ― token を自前で組む。柔軟だが立ち上がりが遅い
> - B. Material-UI v6 ― 既製感が強いが、A2UI の component shape と被りやすい
> - C. Tailwind + shadcn/ui ― 中間。OSS template が豊富
>
> 佐藤さんの中で「既製品感を出したい / オリジナリティを出したい」のどちらが近いですか？

User can answer "B" in two characters and the conversation moves.

## Anti-patterns

- **Decision-flooding.** Ten options is not "comprehensive", it's analysis paralysis. 2–4 is the sweet spot.
- **False neutrality.** If you have a recommendation, lead with it (and label it as such). The user wants your judgement, not a tabula rasa.
- **Asking what you can find out.** If the answer is in `docs/` or the codebase, read it instead of asking.
- **Premature confirmation.** "Should I proceed?" mid-conversation slows momentum. Proceed with reasonable assumptions and announce them; the user will correct.
- **Hidden questions.** Burying the question at the end of a 200-line analysis. The question goes near the top, the analysis after.

## Output shape

When narrative mode is active:

- Use prose, not headers, for short turns (< 200 words)
- Use a small table only when comparing 3+ options on 2+ axes
- Code-style file references with markdown links: `[bf-02-commercial-screening.md](docs/arc42/03-context-and-scope/use-cases/bf-02-commercial-screening.md)`
- End with the question, not with a summary

## Relationship to other pentaglyph artefacts

- This rule is about **conversational behaviour**. Documentation lifecycle (Phase 1–6, what gets written when) lives in [`docs/WORKFLOW.md`](../../docs/WORKFLOW.md).
- The `doc-orchestrator` agent already implements this pattern when building documentation. This rule generalises the pattern to **all** open-ended conversations, not just doc-building.
- When the AI delegates to a specialist sub-agent, the specialist receives a brief — the specialist itself is not bound by narrative mode (it is in execution mode). Narrative mode lives in the orchestrating conversation.
