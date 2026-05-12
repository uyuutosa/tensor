---
status: Draft
owner: <placeholder>
last-reviewed: <YYYY-MM-DD>
---

# Persona Template (Type 6)

> **Copy to use**: `cp templates/6_persona.md arc42/03-context-and-scope/personas/<persona-id>.md`
> Recommended location: `arc42/03-context-and-scope/personas/` (linked from arc42 §3 Context)
> References: [Alan Cooper goal-directed personas](https://www.cooper.com/journal/2008/05/the_origin_of_personas) + [Pruitt & Adlin "The Persona Lifecycle"](https://www.elsevier.com/books/the-persona-lifecycle/pruitt/978-0-12-566251-2) + [Indi Young "Mental Models"](https://rosenfeldmedia.com/books/mental-models/)
> Length target: **1–3 pages per persona**
>
> **Not used**: marketing personas (demographics-first, no goals), ad-hoc "user types". Cooper-style **goal-directed personas** are the only kind pentaglyph opinionates on, because they survive contact with engineering decisions.
>
> A persona is a *reusable* artefact — one persona is referenced from many use-cases, journey maps, and PRDs. Keep it standalone; do not inline scenarios here (those go in journey maps, Type 7).
>
> Delete this `> ...` guidance block after copying.

---

# Persona: <Persona name / archetype>

| Metadata             | Value                                                          |
| -------------------- | -------------------------------------------------------------- |
| Persona ID           | EU-NN / PU-NN / OP-NN (whatever scheme the project uses)       |
| Type                 | Primary / Secondary / Anti                                     |
| Status               | Draft / Validated / Retired                                    |
| Owner                | <name>                                                         |
| Research basis       | <interview count, observation hours, survey N, secondary data> |
| Last Updated         | YYYY-MM-DD                                                     |

---

## 1. One-line summary

> A <demographic> who is trying to <primary goal> while constrained by <key constraint>, and who notices <distinguishing trait>.

One sentence. If you can't write it in one sentence, the persona isn't sharp enough yet.

---

## 2. Identity

| Field                  | Value                                                       |
| ---------------------- | ----------------------------------------------------------- |
| Name (fictional)       | <given name to humanise — never a real person>              |
| Age range              | <e.g. 30–39>                                                |
| Role / Occupation      | <work role or life-stage descriptor>                        |
| Context of use         | <where / when they encounter our product>                   |
| Tech fluency           | Low / Medium / High (with one example anchor)               |
| Quote (verbatim)       | "<one verbatim quote from research that captures voice>"    |

> Photo placeholder: stock or illustrative. Do not use a real person's photo without explicit consent.

---

## 3. Goals (Cooper hierarchy)

Goals drive design decisions. List 3–6 across the three Cooper levels.

| Level         | Goal                                       | Priority |
| ------------- | ------------------------------------------ | -------- |
| **Life**      | <long-term aspiration>                     | High     |
| **End**       | <what they want to *accomplish* with us>   | High     |
| **End**       | <secondary task-level goal>                | Medium   |
| **Experience**| <how they want to *feel* during the task>  | High     |

References: [Cooper "The Inmates Are Running the Asylum"](https://www.cooper.com/about/books/inmates) §9.

---

## 4. Jobs to be Done (JTBD, optional but recommended)

When this persona "hires" the product, what *job* are they getting done?

> When **<situation>**, I want to **<motivation / verb>**, so I can **<expected outcome>**.

Example:
> When I'm scrolling through my company app on my commute, I want to feel I'm progressing toward something good for my body, so I can return to the office without feeling guilty about a sedentary day.

References: [Christensen "Competing Against Luck"](https://hbr.org/2016/09/know-your-customers-jobs-to-be-done).

---

## 5. Behaviours and mental model

3–7 bullets describing observable behaviours and the mental model the persona brings.

- <behaviour 1 — observed, not assumed>
- <mental model 1 — what they believe is true about how the world works>
- <coping strategy when system disagrees with their model>

---

## 6. Pain points

What is currently bad about how this persona accomplishes the goal *without* our product, or with workarounds?

| Pain point                          | Severity | Frequency | Source     |
| ----------------------------------- | -------- | --------- | ---------- |
| <pain>                              | High     | Daily     | Interview #3 |
| <pain>                              | Med      | Weekly    | Observation |

Severity = how much it hurts. Frequency = how often. The product's value is roughly Severity × Frequency × Coverage.

---

## 7. Gains sought

What "better" looks like for this persona — described from their perspective, not ours.

- <gain 1>
- <gain 2>
- <delight gain — would surprise and delight, not strictly required>

References: [Strategyzer Value Proposition Canvas](https://www.strategyzer.com/library/the-value-proposition-canvas).

---

## 8. Anti-patterns (what this persona is NOT)

Equally important. Lists who looks similar but isn't this persona, so designers don't drift.

- NOT <similar-looking persona> — differs in <key dimension>
- NOT <stereotype> — based on <evidence>

---

## 9. Effective vocabulary / Avoidance vocabulary

The words this persona responds to vs. recoils from. Particularly important for AI / LLM-generated content where one wrong word triggers refusal.

| Effective ("approach")                | Avoidance ("retreat")                  |
| ------------------------------------- | -------------------------------------- |
| <word / phrase that opens engagement> | <word that closes engagement>          |

---

## 10. Touchpoints / Channels

Where and how does this persona interact with us?

| Touchpoint     | Frequency  | Purpose at this touchpoint      |
| -------------- | ---------- | ------------------------------- |
| <e.g. mobile app> | Daily   | <primary task>                  |
| <e.g. event venue iPad> | Annual | <one-shot intake>            |

---

## 11. Constraints / Accessibility considerations

- Sensory: <vision / hearing / motor constraints relevant to design>
- Cognitive: <literacy level, language(s), domain knowledge>
- Situational: <one-handed use? noisy environment? short attention window?>
- Regulatory: <consent capacity, data residency, anything special>

---

## 12. Linked artefacts

| Type                                         | File                                                    |
| -------------------------------------------- | ------------------------------------------------------- |
| Journey map(s) where this persona is primary | [`<journey-id>`](../journeys/<file>.md)                |
| Use cases this persona acts in               | [`<use-case-id>`](../use-cases/<file>.md)              |
| PRDs that target this persona                | [`<prd>`](../prds/<file>.md)                           |
| Related research notes                       | <file or research repository link>                      |

---

## 13. Open questions / Next research

What we still don't know about this persona, with the planned method.

- [ ] <question> — method: <interview / observation / survey> — owner: <name> — by: <date>

---

## References

- [Cooper Goal-Directed Personas](https://www.cooper.com/journal/2008/05/the_origin_of_personas)
- [Pruitt & Adlin — "The Persona Lifecycle"](https://www.elsevier.com/books/the-persona-lifecycle/pruitt/978-0-12-566251-2)
- [Indi Young — "Mental Models"](https://rosenfeldmedia.com/books/mental-models/)
- [Strategyzer — Value Proposition Canvas](https://www.strategyzer.com/library/the-value-proposition-canvas)
- [Christensen — Jobs to be Done](https://hbr.org/2016/09/know-your-customers-jobs-to-be-done)
