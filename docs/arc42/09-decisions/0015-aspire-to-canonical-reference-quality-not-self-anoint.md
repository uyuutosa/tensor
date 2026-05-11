---
status: Accepted
owner: tensor
last-reviewed: 2026-05-11
---

# ADR-0015: Reframe ADR-0013 from "is the canonical reference" to "aspires to canonical-reference quality"

| Metadata  | Value                                                                  |
| --------- | ---------------------------------------------------------------------- |
| Status    | **Accepted**                                                           |
| Type      | ADR (Type 5 / MADR v3.0 / arc42 §9)                                    |
| Date      | 2026-05-11                                                             |
| Deciders  | uyuutosa                                                               |
| Consulted | Claude (session of 2026-05-11)                                         |
| Informed  | future contributors                                                    |
| Ticket    | —                                                                      |
| Supersedes | [ADR-0013](0013-reframe-as-canonical-reference-for-named-tensor-computation.md). The disciplines ADR-0013 introduced (bibliography / ubiquitous language / reproducibility — collectively the G-8 citability discipline) **stay in force** under this ADR's framing; what changes is the *claim*. |

---

## Context and Problem Statement

[ADR-0013](0013-reframe-as-canonical-reference-for-named-tensor-computation.md) said: *"the project **is** the canonical reference for differentiable named-axis tensor computation in modern C++"*. In a 2026-05-11 conversation the maintainer pushed back on this claim, noting that ML papers regularly publish self-declared SOTA results that get "overwritten" within months — and asked whether self-declared *canonical reference* status carries the same embarrassment risk.

The maintainer's critique is sharp and partially correct. Distinguishing what's right from what's wrong:

- **Wrong about canonical-reference being SOTA-shaped**. *Canonical reference* and SOTA fail in different ways. SOTA is a dynamic "we are best" claim that gets overwritten when someone else is better — embarrassing precisely because the claim was time-bound. *Canonical reference* is a static "we are the citable definition" position; CPython remains canonical-Python even though PyPy is faster, CLRS remains canonical-algorithms-textbook 30+ years on, K&R remains canonical-C-textbook, RFC 791 remains canonical-IP — these works are not "overwritten" by newer / faster alternatives because their job is *being the cited substrate*, not *being best*.
- **Right about self-anointment**. CPython does not call itself "the canonical reference for Python" — that recognition is **external**: the community cites it, ports from it, extends it. The label is a **result**, not an opening claim. ADR-0013's wording — *"is the canonical reference"* — performs the opening claim, and that's the embarrassment-risk shape.

The fix is to keep the **discipline** (the three forcing-function behaviors ADR-0013 introduced: bibliography, ubiquitous language, reproducibility) while reframing the **claim** to honest epistemic posture.

---

## Decision Drivers

- **DD-1**: Avoid self-anointing. Canonical-reference status is bestowed by external adoption, not declared by the author.
- **DD-2**: Keep the disciplines. The three behaviors are good for the project regardless of whether the canonical-reference label ever materialises — they make the code more readable, the docs more citable, and the build more reproducible.
- **DD-3**: Match the project's actual state. A one-person project revived from 10-year dormancy on 2026-05-10 has not yet been cited / ported / extended by anyone external. Claiming canonical status before any external recognition is a category error.
- **DD-4**: Preserve the substantive reframing from ADR-0013. The audience-sharpening (future implementers / researchers / textbook authors) and the G-8 citability discipline are real wins; the only thing that needs to change is the verb tense and possessive of the claim itself.

---

## Considered Options

1. **Status quo** — keep ADR-0013's *"is the canonical reference"* wording. Rejected per the maintainer's critique.
2. **Drop the canonical-reference framing entirely** — revert to ADR-0010's *"educational-first, production-capable"*. Rejected because the three disciplines ADR-0013 introduced are genuinely worth keeping, and the language to refer to them shouldn't disappear.
3. **Reframe to "aspires to canonical-reference quality"** (chosen) — keep the disciplines, change the claim from declarative ("is") to aspirational ("aspires to" / "documented and designed to canonical-reference standards"). Honest about the project's pre-recognition state; the disciplines remain forcing functions that make the aspiration costly to walk away from.

---

## Decision Outcome

**Chosen option: 3 — aspirational framing.**

Concretely:

1. **The disciplines stay**. G-8 citability discipline (bibliography + ubiquitous language + reproducibility), the §12 glossary, the per-container detailed-design instances, the CFF citation metadata — all unchanged in substance. Only the *claim language* that names what we're aspiring to changes.

2. **Public claim language**. Replace declarative *"is the canonical reference"* with one of:
   - "Documented and designed to canonical-reference quality"
   - "Aspires to canonical-reference quality for ..."
   - "Built with canonical-reference disciplines (bibliography, ubiquitous language, reproducibility)"

   The first is the new opener for README, book/intro, CITATION.cff, arc42 §1 TL;DR.

3. **The disciplines become the demonstrable substance, not the canonical-reference label**. When asked "is this project a canonical reference?" the honest answer is "no project is a canonical reference until external work cites / ports / extends it. We're built so that — *if* it happens — the work is ready: every public name traces to a paper or ADR (bibliography), the codebase / docs / glossary use one name per concept (ubiquitous language), and clean clone reproduces build + bench + notebooks in under 30 minutes (reproducibility)."

4. **ADR-0013 status changes to `Superseded by ADR-0015`**. The body of ADR-0013 stays immutable per the kit's ADR rules; readers find this ADR via the status pointer.

5. **No code changes**. Header / kernel / test source is unaffected. The change is exclusively in user-facing language and documentation framing.

### Y-statement summary

> In the context of **the maintainer's 2026-05-11 critique that ADR-0013's *"is the canonical reference"* wording is shaped like self-declared SOTA and risks the same embarrassment**, facing **the choice between keeping the declarative claim, dropping the framing entirely, or reframing it as aspirational**, we decided for **the aspirational reframing — *aspires to canonical-reference quality* — while keeping the three disciplines (bibliography / ubiquitous language / reproducibility) in full force**, to achieve **epistemic honesty about the project's pre-recognition state without losing the forcing functions ADR-0013 introduced**, accepting **that the README / book / CITATION.cff / arc42 §1 / CONTRIBUTING / detailed-design docs need their opening language updated**.

---

## Pros and Cons of the Options

### Option 1: Status quo

- Pros: zero work; consistent with already-merged PRs.
- Cons: the embarrassment-risk the maintainer flagged; reads as self-anointment; CPython / CLRS / K&R / RFC counter-examples illustrate why the verb tense matters.

### Option 2: Drop the framing entirely

- Pros: simplest fallback.
- Cons: throws away substantive work (G-8 discipline, §12 glossary, detailed-design depth). Substituting bare educational-first wording also loses the depth-over-breadth contributor heuristic from CONTRIBUTING.md.

### Option 3: Aspirational reframing (chosen)

- Pros: honest about pre-recognition state; keeps the disciplines; preserves the substantive depth gained in PRs #39-#56; small mechanical update to user-facing language.
- Cons: more nuanced framing requires a paragraph rather than a slogan to land on a casual reader; some sentences in the existing docs need rewording rather than verbatim replacement.

---

## Consequences

### Positive

- Public claim matches actual state: the project is **built to** canonical-reference quality, not **claiming to be** the canonical reference. Closer to how CPython / CLRS / K&R earned the label (by behaving like one, then being recognised).
- The three disciplines stay as forcing functions, demonstrably costly to walk away from — the substance is preserved.
- Eliminates the SOTA-shaped embarrassment risk the maintainer identified.

### Negative

- Wording churn in many user-facing files: README, book/intro, CITATION.cff, CONTRIBUTING, tensor.hpp, CHANGELOG, arc42 §1 §2 §7 §10 §11 §12, detailed-design ×3, two impl-plans, three reports. One bundled PR per the doc-sync precedent (PRs #11, #29, #34, #45).
- New contributors reading the changelog may misread the supersession as "the project gave up on canonical-reference framing". The CHANGELOG entry must explicitly say *"disciplines stay; only the claim wording changes"*.

### Neutral

- ADR-0013 stays Accepted-in-body, Superseded-by-status; per the kit's MADR rule the body never changes.
- No code change; CI / tests / kernel sources unaffected.

### Follow-ups

- [ ] Mark ADR-0013 status frontmatter as `Superseded by ADR-0015`.
- [ ] Update README first paragraph + Citing section.
- [ ] Update book/intro.md first line + "How to cite this work" section.
- [ ] Update CITATION.cff title / abstract / keywords / preferred-citation / references entry naming ADR-0013.
- [ ] Update CONTRIBUTING.md opening paragraph.
- [ ] Update include/tensor/tensor.hpp umbrella header origin paragraph.
- [ ] Update CHANGELOG.md `[Unreleased]` block — supersession note + rewrite the prior "Canonical-reference reframing" entry.
- [ ] Update docs/arc42/01-introduction-and-goals/overview.md TL;DR, §1 system identity, §2 stakeholders, §3 G-8, §6 success criteria, revision history.
- [ ] Update docs/arc42/12-glossary/overview.md "Canonical reference" entry.
- [ ] Update docs/arc42/02-architecture-constraints/overview.md OC-5.
- [ ] Update docs/arc42/07-deployment/overview.md §5 citation discovery wording.
- [ ] Update docs/arc42/10-quality/overview.md QC-3 + QF-1.
- [ ] Update docs/arc42/11-risks/overview.md R-A3 (citability discipline rot).
- [ ] Update docs/detailed-design/{tensor-core,tensor-autograd,tensor-tex}.md TL;DR phrasing.
- [ ] Update docs/impl-plans/2026-05-11_phase-3-webgpu.md addendum mentioning canonical-reference.
- [ ] Update docs/reports/{phase-4-release-rehearsal,open-discussion-points}.md wherever the declarative claim shape appears.

---

## Compliance / Validation

- **Verification**: after the supersession PR lands, no user-facing string in `README.md`, `book/intro.md`, `CITATION.cff`, `CONTRIBUTING.md`, `include/tensor/tensor.hpp`, or `docs/arc42/` says *"is the canonical reference"* or *"the canonical reference for X"* in declarative form. The aspirational forms ("aspires to canonical-reference quality", "documented to canonical-reference standards") are acceptable.
- **Frequency**: per-PR grep at the half-yearly bibliography audit (ADR-0013 §Compliance; renamed in spirit to ADR-0015 §Compliance under this supersession). Initial audit due 2026-11-11.

---

## More Information

### Related ADRs

- Supersedes: [ADR-0013](0013-reframe-as-canonical-reference-for-named-tensor-computation.md)
- Refines (through ADR-0013): [ADR-0001](0001-pivot-to-educational-named-axis-dsl.md), [ADR-0010](0010-refine-positioning-to-educational-first-production-capable.md)
- Architectural foundation kept in force: [ADR-0009](0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md)
- Operational substrate strategy kept in force: [ADR-0014](0014-external-substrate-strategy.md)

### External references — works that earned canonical-reference status externally (not by self-declaration)

- [CPython](https://github.com/python/cpython) — slower than PyPy, but canonical-Python because the community cites and ports from it.
- [CLRS — *Introduction to Algorithms*](https://mitpress.mit.edu/9780262046305/introduction-to-algorithms/) — 30+ years on; still canonical algorithms textbook.
- K&R — *The C Programming Language* (Kernighan & Ritchie, 1978; 2nd ed. 1988) — canonical-C-textbook.
- [TeX](https://tug.org/whatis.html) — Knuth's typesetting system; canonical typesetting algorithm reference.
- [RFC 791 — Internet Protocol](https://www.rfc-editor.org/rfc/rfc791) — 1981; still canonical IP-spec.

In each case the work *behaved like* a canonical reference (clear vocabulary, citable, reproducible, durable design) — and was *then* recognised as one by external adoption. None of these works opens with "I am the canonical reference."

### Conversation that triggered this ADR

The maintainer's 2026-05-11 message: *"でもディープラーニング界隈の論文も過去 SOTA とかアブレーションスタディとか怪しい現象でしかない論文が散見されてたけど、、すぐ書き換わる SOTA も恥ずかしい気がする"* — "ML papers chase SOTA / ablation results that turn out to be dubious phenomena; SOTA that gets overwritten quickly is also embarrassing." The follow-on question — *"自称が hubris にならないための forcing function、これはどういうことを懸念しているの？わかりやすくいうと"* — surfaced the specific risk this ADR addresses.
