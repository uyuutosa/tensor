---
status: Proposed
owner: tensor
last-reviewed: 2026-05-14
---

# ADR-0021: Strategic narrowing — adopt specialist-reference positioning, decline production-scale ML chase

| Metadata  | Value                                                          |
| --------- | -------------------------------------------------------------- |
| Status    | **Proposed**                                                   |
| Type      | ADR (Type 5 / MADR v3.0 / arc42 §9)                            |
| Date      | 2026-05-14                                                     |
| Deciders  | uyuutosa (maintainer)                                          |
| Consulted | —                                                              |
| Informed  | All Phase 7+ contributors; Phase 7a impl-plan downstream       |

---

## Context and Problem Statement

The 2026-05-12 landscape recheck ([`../../reports/2026-05-12_landscape-recheck-and-adversarial-review.md`](../../reports/2026-05-12_landscape-recheck-and-adversarial-review.md) §A.5) reported "no first-class named-axis tensor exists in production Python ML" as the strategic wedge supporting Phase 6 / Phase 6.5 work. The follow-up 2026-05-14 landscape recheck ([`../../reports/2026-05-14_landscape-recheck-and-phase-7-roadmap.md`](../../reports/2026-05-14_landscape-recheck-and-phase-7-roadmap.md) §1) refined this finding:

- **Haliax** (Stanford CRFM → merged into Marin Nov 2025) **substantially fills the wedge in JAX** — production-scale, 70B-param LLM training on TPU v4-2048, active development.
- **Penzai** (Google DeepMind) brings major-vendor backing to the named-axes pattern in JAX, focused on notebook ergonomics + LLM tooling.
- **einx** (ICLR 2026 oral) legitimises Einstein-style notation academically; works across NumPy/PyTorch/JAX/TF/MLX as a notation translation layer.

The named-axis wedge in *Python ML* has therefore narrowed considerably between the May-12 and May-14 audits. This ADR records the strategic response: **decline the production-scale ML chase, narrow into the moats that remain uncontested** — C++20-native hybrid named-axis API, `_tex` UDL, Hexagonal-lite + KernelBackend port as teaching surface, canonical-reference-quality docs discipline.

This decision sits alongside [ADR-0010](./0010-refine-positioning-to-educational-first-production-capable.md) (educational-first, production-capable) and [ADR-0013](./0013-reframe-as-canonical-reference-for-named-tensor-computation.md) / [ADR-0015](./0015-aspire-to-canonical-reference-quality-not-self-anoint.md) (canonical-reference framing). Those prior ADRs set the framing; this ADR operationalises the response to a specific landscape shift.

---

## Decision Drivers

In priority order:

- **DD-1 — sustainability of solo-maintainer bandwidth**. Chasing Haliax's production-scale story would require FSDP analogue, sharded autograd, distributed checkpoint, scaling cards — roughly an order of magnitude more engineering than the project's solo maintainer can sustain. Narrowing preserves shipping velocity.
- **DD-2 — distinctive moats remain uncontested**. The C++ angle, `_tex` UDL, citation-grade docs, and educational-first hexagonal-lite framing have no head-to-head competitor. Investment in these compounds.
- **DD-3 — landscape audit cadence requires response**. [ADR-0017](./0017-clarify-reproducibility-envelope.md) commits to a half-yearly landscape recheck. Recording the strategic response to each recheck closes the audit→action loop.
- **DD-4 — `0.x` series budget**. The Phase 7+ roadmap recommends `0.4.0` (Phase 6.6 R-P6.5.5 lift) + the Phase 7a documentation-as-product release. Narrowing keeps scope inside the `0.x` budget; widening to production-scale would push into `1.0` semantics prematurely.

---

## Considered Options

1. **Status quo** — keep ADR-0013 / ADR-0015 framing exactly as-is. Treat the Haliax/Penzai/einx developments as background. Do not record a strategic response.
2. **Strategic narrowing** — explicitly decline production-scale ML competition; double-down on the four distinctive moats (C++-native hybrid axes, `_tex` UDL, KernelBackend as teaching surface, citation-grade docs). Operationalise via Phase 7a Documentation-as-Product.
3. **Widen to JAX integration** — add a JAX adapter alongside the existing reference/Eigen/WebGPU KernelBackend adapters, positioning `tensor` as "C++ bottom + JAX top" for users who want compile-time-checked axes feeding into JAX traces.
4. **Add a production-scale roadmap** — commit to FSDP analogue, tensor parallelism, distributed checkpoint over Phase 8-10. Match Haliax's production-scale story.

---

## Decision Outcome

**Chosen option: 2 — Strategic narrowing.**

Option 1 (status quo) misses an opportunity to record the rationale behind Phase 7a's documentation-as-product focus. Future contributors reading the impl-plan would not see *why* the project declined production-scale work — leaving room for scope creep on later phases.

Option 3 (JAX adapter) is an interesting widening, but the JAX runtime ABI is large and version-fragile; integrating `tensor::core` with JAX would be a substantial Phase-equivalent of work that competes with Phase 7c (`_tex` parser expansion) for solo-maintainer attention. Defer to a follow-up landscape audit; do not commit now.

Option 4 (production-scale) fails DD-1 outright. The project's solo-maintainer model cannot sustain a Haliax-equivalent production-scale story.

Option 2 cashes in on prior decisions (ADR-0010 / 0013 / 0015) by making the strategic-narrowing response explicit, and unlocks Phase 7a's Documentation-as-Product work as the immediate operationalisation. The four moats stay the project's distinctive offering; the production-scale gap stays the explicit non-goal.

### Y-statement summary

> In the context of **Haliax + Penzai + einx substantially filling the production-Python-ML named-axis wedge between 2026-05-12 and 2026-05-14**, facing **solo-maintainer bandwidth limits + the question of whether to chase production-scale parity or narrow into the remaining moats**, we decided for **strategic narrowing to specialist-reference positioning (C++-native hybrid axes + `_tex` UDL + KernelBackend teaching surface + citation-grade docs)** to achieve **sustained shipping velocity + uncontested moats compounded over Phase 7+**, accepting **the explicit non-goal of competing with Haliax/Penzai on production-scale ML**.

---

## Pros and Cons of the Options

### Option 1: Status quo

- Pros:
  - Zero documentation work.
  - Keeps ADR-0013 / ADR-0015 framing unchanged.
- Cons:
  - The audit→action loop from ADR-0017's half-yearly recheck stays implicit.
  - Future contributors lack the rationale chain for *why* Phase 7a is documentation-focused and not (say) JAX integration.

### Option 2: Strategic narrowing (chosen)

- Pros:
  - **Records the strategic response** to a specific landscape shift, closing the audit→action loop.
  - **Compounds the distinctive moats** — every Phase 7+ investment serves the specialist-reference positioning.
  - **Solo-maintainer-sustainable** — work fits inside the bandwidth budget.
  - Phase 7a Documentation-as-Product becomes the immediate operationalisation; downstream impl-plan rests on this ADR.
- Cons:
  - **Forecloses production-scale options** for the `0.x` series. If the landscape shifts back (Haliax/Penzai unmaintained, JAX ecosystem retracts), this ADR would need to be revisited.
  - **Audience-narrows** — the project's user persona becomes more sharply defined as textbook authors / future implementers / educators, with less appeal to production-scale practitioners.

### Option 3: Widen to JAX integration

- Pros:
  - Couples `tensor`'s compile-time-checked axes with JAX's production-scale runtime — bridges two ecosystems.
  - Could attract Haliax/Penzai users who want the C++ compile-time guarantees.
- Cons:
  - **JAX runtime ABI is large and version-fragile** — maintaining a JAX adapter takes ongoing attention.
  - **Competes with Phase 7c (`_tex` parser expansion)** for solo-maintainer bandwidth.
  - Risk of becoming a thin wrapper around JAX with little distinct value.

### Option 4: Production-scale roadmap

- Pros:
  - Direct competitive response to Haliax / Penzai.
  - Could capture production-scale named-axis users.
- Cons:
  - **Fails DD-1** — solo-maintainer cannot sustain FSDP / tensor parallelism / distributed checkpoint engineering.
  - Even if delivered, would arrive years after Haliax's existing production track record (Levanter → Marin 70B+).
  - Pushes `tensor` into `1.0` semantics prematurely.

---

## Consequences

### Positive

- **Phase 7a roadmap is unblocked**. Documentation-as-Product (citable PDF + NeurIPS workshop paper draft + landscape comparison report) becomes the immediate next phase, anchored to this ADR.
- **Future ADRs have a strategic-narrowing lens**. Phase 7+ decisions can ask "does this serve the specialist-reference positioning or chase production-scale?" — declining the latter automatically.
- **Audit→action loop is closed** for the 2026-05-14 landscape recheck. The next recheck due 2026-11-11 inherits this ADR as the prior strategic baseline.

### Negative

- **Production-scale user persona is explicitly out of scope** for the `0.x` series. Documentation should be honest about this: tell production-scale users to use Haliax / Penzai / Marin and explain why.
- **Six-monthly recheck must verify** that the wedges supporting strategic narrowing (Haliax production-scale serving the gap; `tensor`'s moats still uncontested) remain valid. If either retracts, this ADR needs supersession.

### Neutral

- **ADR-0010 / ADR-0013 / ADR-0015 are unchanged**. This ADR operationalises them; it does not supersede them.
- **`_tex` UDL + KernelBackend port surface + Hexagonal-lite framing** continue exactly as they have been. The change is in *how the project markets itself + sequences future work*, not in *what the project is*.

### Follow-ups

- [ ] Phase 7a impl-plan: [`../../impl-plans/2026-05-14_phase-7a-docs-as-product.md`](../../impl-plans/2026-05-14_phase-7a-docs-as-product.md) — milestones M1 (landscape comparison study) → M2 (Diátaxis-to-PDF packaging) → M3 (NeurIPS workshop paper draft) → M4 (`0.5.0` release ceremony).
- [ ] Landscape comparison study: [`../../reports/2026-05-14_named-tensor-library-comparison.md`](../../reports/2026-05-14_named-tensor-library-comparison.md) — M1 of Phase 7a.
- [ ] Six-monthly recheck (due 2026-11-11): verify Haliax/Penzai still serve the production-scale gap; verify `tensor`'s moats still uncontested. Trigger ADR-0021 supersession review if either condition fails.
- [ ] Update [`docs/STRATEGY.md`](../../../STRATEGY.md) to reference this ADR as the canonical strategic-narrowing source. (Only if `docs/STRATEGY.md` is the project's strategy entry point; if not, no-op.)

---

## Compliance / Validation

- **Verification**: every Phase 7+ impl-plan introduces a "Sequencing relative to other work" section that names this ADR as the strategic-narrowing baseline. A plan that violates the narrowing (e.g., adds an FSDP analogue) must either supersede ADR-0021 or document the deviation.
- **Frequency**: every Phase 7+ entry; every six-monthly landscape recheck.

---

## More Information

### Related ADRs

- Positioning chain: [ADR-0001](./0001-pivot-to-educational-named-axis-dsl.md) → [ADR-0010](./0010-refine-positioning-to-educational-first-production-capable.md) → [ADR-0013](./0013-reframe-as-canonical-reference-for-named-tensor-computation.md) → [ADR-0015](./0015-aspire-to-canonical-reference-quality-not-self-anoint.md) → **ADR-0021 (this)** (operationalises).
- Audit cadence: [ADR-0017](./0017-clarify-reproducibility-envelope.md) — clarifies reproducibility envelope including the half-yearly landscape recheck commitment.
- Related: [ADR-0019](./0019-phase-6-5-runtime-backend-selection-via-extras.md), [ADR-0020](./0020-multi-backend-runtime-dispatch-via-type-owner-separation.md) — current Phase 6.5 / 6.6 work that this ADR succeeds in the strategic sequence.

### References

- 2026-05-14 landscape recheck + Phase 7+ roadmap: [`../../reports/2026-05-14_landscape-recheck-and-phase-7-roadmap.md`](../../reports/2026-05-14_landscape-recheck-and-phase-7-roadmap.md).
- 2026-05-12 landscape recheck (predecessor): [`../../reports/2026-05-12_landscape-recheck-and-adversarial-review.md`](../../reports/2026-05-12_landscape-recheck-and-adversarial-review.md).
- 2026-05-10 original revival landscape: [`../../reports/2026-05-10_tensor-revival-landscape.md`](../../reports/2026-05-10_tensor-revival-landscape.md).
- Haliax (Marin merger Nov 2025): <https://github.com/marin-community/haliax>.
- Penzai documentation: <https://penzai.readthedocs.io/>.
- einx ICLR 2026 oral: <https://openreview.net/forum?id=QqvQ3iAdpC>.
