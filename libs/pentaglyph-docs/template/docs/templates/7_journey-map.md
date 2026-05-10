---
status: Draft
owner: <placeholder>
last-reviewed: <YYYY-MM-DD>
---

# Journey Map Template (Type 7)

> **Copy to use**: `cp templates/7_journey-map.md arc42/03-context-and-scope/journeys/<journey-id>.md`
> Recommended location: `arc42/03-context-and-scope/journeys/` (linked from arc42 §3 Context and §6 Runtime)
> References: [Kalbach "Mapping Experiences"](https://www.oreilly.com/library/view/mapping-experiences/9781491923528/) + [NN/g Customer Journey Maps 101](https://www.nngroup.com/articles/customer-journey-mapping/) + [Donna Lichaw "The User's Journey"](https://rosenfeldmedia.com/books/the-users-journey/)
> Length target: **1–4 pages per journey**
>
> A journey map covers **one persona × one scenario**. Don't combine multiple personas — split into separate files. The persona is referenced (not duplicated) from the persona file (Type 6).
>
> Use this for *experiential* mapping (thoughts, feelings, friction). For *system-level* mapping (which agent calls which tool), use a Use Case (Type 4). The two are complementary and often cross-link.
>
> Delete this `> ...` guidance block after copying.

---

# Journey: <Persona> — <Scenario name>

| Metadata             | Value                                                          |
| -------------------- | -------------------------------------------------------------- |
| Journey ID           | J-NN                                                           |
| Persona              | [<persona-id>](../personas/<file>.md)                          |
| Scenario type        | Current state / Future state / Comparative                     |
| Status               | Draft / Validated / Retired                                    |
| Owner                | <name>                                                         |
| Research basis       | <observation count, interview N, analytics dataset>            |
| Last Updated         | YYYY-MM-DD                                                     |

---

## 1. Scenario summary

One paragraph describing the trigger, the context, and the desired outcome from the persona's perspective.

> <Persona> is doing <scenario context> when <trigger>. They want <outcome> and care about <key value>. The scenario starts at <start point> and ends at <end point>.

---

## 2. Pre-conditions and assumptions

What must be true *before* the journey begins. List anything that, if false, invalidates this map.

- <pre-condition 1>
- <persona-side assumption>
- <system-side assumption>

---

## 3. Journey map (the table)

The core artefact. One row per stage. Stage count: typically 4–8. More than 10 means split the journey.

| #   | Stage              | Persona's goal       | Action                | Thinking ("…")           | Feeling          | Touchpoint              | Pain point              | Opportunity               |
| --- | ------------------ | -------------------- | --------------------- | ------------------------ | ---------------- | ----------------------- | ----------------------- | ------------------------- |
| 1   | <Awareness>        | <stage goal>         | <observable action>   | "<verbatim or inferred>" | 😐 Neutral       | <channel>               | <friction here>         | <design opportunity>     |
| 2   | <Onboarding>       | …                    | …                     | …                        | 🙂 Curious       | …                       | …                       | …                         |
| 3   | <First use>        | …                    | …                     | …                        | 😟 Confused      | …                       | …                       | …                         |
| 4   | <Repeated use>     | …                    | …                     | …                        | 😊 Confident     | …                       | …                       | …                         |
| 5   | <Outcome>          | …                    | …                     | …                        | 🤩 Delighted     | …                       | …                       | …                         |

**Feeling encoding.** Pick one of: 😩 Frustrated / 😟 Anxious / 😐 Neutral / 🙂 Positive / 😊 Confident / 🤩 Delighted. Or use a -2..+2 scale if your team prefers numbers. Be consistent within the file.

> Tip: a sparkline of feeling values across the row reveals where investment matters most. Look for the deepest dip — that is usually the highest-leverage redesign target.

---

## 4. Moments of truth

The 1–3 stages that disproportionately determine whether the journey succeeds or fails for this persona. Investment per dollar is highest here.

| #   | Moment                                | Why it's pivotal                               | Current state (👍 / 👎) |
| --- | ------------------------------------- | ---------------------------------------------- | ----------------------- |
| MoT-1 | <stage / interaction>                | <why this is the fulcrum>                      | 👎 Currently fails      |
| MoT-2 | <stage / interaction>                | …                                              | 👍                      |

Reference: [Carlzon "Moments of Truth"](https://www.harpercollins.com/products/moments-of-truth-jan-carlzon).

---

## 5. Key insights

Synthesised insights that emerge from the rows above. 3–6 bullets, each one sentence.

- **Insight 1:** <observation about the persona-system interaction>
- **Insight 2:** <often-surprising friction>
- **Insight 3:** <gap between persona's mental model and system reality>

---

## 6. Service implications

Translate insights into design / engineering implications. Connect to product artefacts.

| Implication                                  | Affected artefact                                        |
| -------------------------------------------- | -------------------------------------------------------- |
| <design implication>                         | PRD `FR-<CAT>-NNN` / [<feature>](../prds/<file>.md)      |
| <engineering implication>                    | [<module>](../../../detailed-design/<file>.md)           |
| <decision needed — record as ADR>            | New ADR under `arc42/09-decisions/`                       |

---

## 7. Variants and exception flows

Short list of journey variants worth flagging without authoring full alternative maps. If a variant is materially different, split it into its own file.

- **Variant A: <condition>** — diverges at stage <#>, returns at <#>. Implication: <one line>.
- **Exception: <error or refusal>** — what the persona experiences. Implication: <one line>.

---

## 8. Linked artefacts

| Type                                         | File                                                    |
| -------------------------------------------- | ------------------------------------------------------- |
| Persona                                      | [<persona-id>](../personas/<file>.md)                   |
| Use case (system-level)                      | [<use-case-id>](../use-cases/<file>.md)                 |
| Service blueprint (frontstage / backstage)   | [<blueprint-id>](../blueprints/<file>.md)               |
| Related PRDs                                 | [<prd>](../prds/<file>.md)                              |
| Related ADRs                                 | [<adr>](../../09-decisions/<file>.md)                   |

---

## 9. Open questions / Next research

What this map *does not yet* answer, with planned method.

- [ ] <question> — method: <usability test / diary study / analytics dive> — owner: <name> — by: <date>

---

## References

- [Kalbach — "Mapping Experiences"](https://www.oreilly.com/library/view/mapping-experiences/9781491923528/) (the canonical methodology reference)
- [NN/g — Customer Journey Maps 101](https://www.nngroup.com/articles/customer-journey-mapping/)
- [Donna Lichaw — "The User's Journey"](https://rosenfeldmedia.com/books/the-users-journey/)
- [Carlzon — "Moments of Truth"](https://www.harpercollins.com/products/moments-of-truth-jan-carlzon)
- [Service Design Tools — Journey Map](https://servicedesigntools.org/tools/customer-journey-map)
