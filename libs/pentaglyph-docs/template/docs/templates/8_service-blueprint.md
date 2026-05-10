---
status: Draft
owner: <placeholder>
last-reviewed: <YYYY-MM-DD>
---

# Service Blueprint Template (Type 8)

> **Copy to use**: `cp templates/8_service-blueprint.md arc42/03-context-and-scope/blueprints/<service-id>.md`
> Recommended location: `arc42/03-context-and-scope/blueprints/` (linked from arc42 §3 Context and §6 Runtime)
> References: [Bitner, Ostrom, Morgan "Service Blueprinting"](https://digitalcommons.usu.edu/cgi/viewcontent.cgi?article=1041&context=marketing_facpub) + [NN/g Service Blueprints 101](https://www.nngroup.com/articles/service-blueprints-definition/) + [This Is Service Design Doing](https://www.thisisservicedesigndoing.com/)
> Length target: **2–4 pages per blueprint**
>
> A service blueprint extends a journey map by exposing **what the organisation does behind the scenes** to make the customer experience happen. Use it when:
> - the service is *cross-functional* (humans + systems + back-office)
> - failures cluster at organisational seams (frontstage ↔ backstage handoffs)
> - SLA / response-time / staffing decisions need a shared picture
>
> If the journey is purely customer-facing with no human backstage, a Journey Map (Type 7) is enough — skip this template.
>
> Delete this `> ...` guidance block after copying.

---

# Service Blueprint: <Service name>

| Metadata             | Value                                                          |
| -------------------- | -------------------------------------------------------------- |
| Blueprint ID         | SB-NN                                                          |
| Persona(s)           | [<persona-id>](../personas/<file>.md)                          |
| Related Journey      | [<journey-id>](../journeys/<file>.md)                          |
| Status               | Draft / Validated / Retired                                    |
| Owner                | <name>                                                         |
| Last Updated         | YYYY-MM-DD                                                     |

---

## 1. Service summary

One paragraph: what this service does, who delivers it, who receives it, and what success looks like.

> The <service> delivers <value> to <persona> via <channels>, with <organisational units> behind the scenes. Success means <outcome metric>.

---

## 2. Scope

| Aspect              | In scope                                | Out of scope                            |
| ------------------- | --------------------------------------- | --------------------------------------- |
| Personas            | <list>                                  | <list>                                  |
| Channels            | <e.g. mobile app, on-site iPad, HCP UI> | <e.g. paper forms>                      |
| Time horizon        | <e.g. single visit, 7-day session>      | <e.g. multi-year cohort>                |
| Organisational units| <departments, vendors involved>         | <out-of-scope ops>                      |

---

## 3. The blueprint (the table)

The blueprint is a table in **time order** (columns = stages) with **swim-lanes** (rows = layers). Stages mirror the journey map; layers are the Bitner standard four plus support processes.

> **Lines** between layers are conceptual:
> - **Line of Interaction** — between Customer Actions and Frontstage. Crossing it means human-to-organisation contact.
> - **Line of Visibility** — between Frontstage and Backstage. Below this line, the customer cannot see what's happening.
> - **Line of Internal Interaction** — between Backstage and Support. Below this line, contact is between organisational units, not with the customer at all.

| Layer ↓ \ Stage →                       | Stage 1: <name>          | Stage 2: <name>          | Stage 3: <name>          | Stage 4: <name>          |
| --------------------------------------- | ------------------------ | ------------------------ | ------------------------ | ------------------------ |
| **Physical evidence**                   | <signage, device, UI>    | <touchpoint artefact>    | <ticket, receipt, badge> | <follow-up artefact>     |
| **Customer actions**                    | <what persona does>      | …                        | …                        | …                        |
| ── *Line of Interaction* ──             |                          |                          |                          |                          |
| **Frontstage actions** (visible)        | <staff or AI agent action shown to customer> | … | … | … |
| ── *Line of Visibility* ──              |                          |                          |                          |                          |
| **Backstage actions** (invisible)       | <ops staff / system internal> | …                   | …                        | …                        |
| ── *Line of Internal Interaction* ──    |                          |                          |                          |                          |
| **Support processes**                   | <upstream/downstream system, vendor, regulator> | … | … | … |

---

## 4. Failure points and recovery

Where does this service fail? What are the recovery paths?

| #   | Failure point                            | Layer                | Detection signal                | Recovery                          | SLA / target          |
| --- | ---------------------------------------- | -------------------- | ------------------------------- | --------------------------------- | --------------------- |
| F-1 | <e.g. AI generates non-compliant text>   | Frontstage           | QC Gate refusal                 | Fallback template + log + retrain | < 1 % of sessions     |
| F-2 | <e.g. HCP no-response>                   | Backstage / Support  | 30-min timer expiry             | Escalate to facility manager      | 30 min                |
| F-3 | <e.g. consent ledger unreachable>        | Support              | Health check                    | Block escalations + retry queue   | < 5 min outage        |

---

## 5. Cross-functional dependencies

Layers are owned by different groups. Make ownership explicit so accountability survives reorganisation.

| Layer                  | Owner (role / team)              | Escalation contact         |
| ---------------------- | -------------------------------- | -------------------------- |
| Physical evidence      | <design lead>                    | <contact>                  |
| Frontstage actions     | <product / on-site staff lead>   | <contact>                  |
| Backstage actions      | <ops lead / system owner>        | <contact>                  |
| Support processes      | <vendor manager / SRE>           | <contact>                  |

---

## 6. Time and capacity dimensions

For services with throughput or response-time constraints, document the per-stage budget so engineering can allocate compute / staffing correctly.

| Stage     | Customer-perceived target | System target | Backstage human target |
| --------- | ------------------------- | ------------- | ---------------------- |
| <stage 1> | < 3 s response            | < 1 s p95     | N/A                    |
| <stage 4> | < 30 min HCP reply        | < 1 s alert   | < 30 min               |

---

## 7. Variants

Short notes on alternative blueprint flows. If materially different, split into its own file.

- **Variant A: <condition>** — diverges at stage <#> with <change>. Capacity implication: <one line>.

---

## 8. Linked artefacts

| Type                                         | File                                                    |
| -------------------------------------------- | ------------------------------------------------------- |
| Persona(s)                                   | [<persona-id>](../personas/<file>.md)                   |
| Journey map                                  | [<journey-id>](../journeys/<file>.md)                   |
| Use case(s) the blueprint operationalises    | [<use-case-id>](../use-cases/<file>.md)                 |
| Module specs of involved systems             | [<module>](../../../detailed-design/<file>.md)          |
| Quality scenarios / SLOs                     | [SLOs](../../10-quality/slos.md)                        |
| Related ADRs                                 | [<adr>](../../09-decisions/<file>.md)                   |

---

## 9. Open questions / Next research

- [ ] <question about a layer or handoff> — method: <observation / process audit> — owner: <name> — by: <date>

---

## References

- [Bitner, Ostrom, Morgan — "Service Blueprinting" (2008)](https://digitalcommons.usu.edu/cgi/viewcontent.cgi?article=1041&context=marketing_facpub) (the canonical methodology paper)
- [NN/g — Service Blueprints 101](https://www.nngroup.com/articles/service-blueprints-definition/)
- [This Is Service Design Doing](https://www.thisisservicedesigndoing.com/)
- [Service Design Tools — Service Blueprint](https://servicedesigntools.org/tools/service-blueprint)
- [Lynn Shostack — "Designing Services That Deliver" (HBR 1984)](https://hbr.org/1984/01/designing-services-that-deliver) (the original)
