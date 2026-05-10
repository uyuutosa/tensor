---
status: Accepted
owner: tensor
last-reviewed: 2026-05-11
---

# ADR-0010: Refine ADR-0001 to "educational-first, production-capable via backend adapters"

| Metadata  | Value                                                                  |
| --------- | ---------------------------------------------------------------------- |
| Status    | **Accepted**                                                           |
| Type      | ADR (Type 5 / MADR v3.0 / arc42 §9)                                    |
| Date      | 2026-05-11                                                             |
| Deciders  | uyuutosa                                                               |
| Consulted | Claude (session of 2026-05-11)                                         |
| Informed  | future contributors                                                    |
| Ticket    | —                                                                      |
| Refines   | [ADR-0001](0001-pivot-to-educational-named-axis-dsl.md) (not superseded; ADR-0001 remains in force, this ADR narrows one of its claims) |

---

## Context and Problem Statement

[ADR-0001](0001-pivot-to-educational-named-axis-dsl.md) positioned the library as *educational, not production*. The rationale rested on three claims:

1. **Speed**: a one-person rewrite cannot match Eigen / xtensor / libtorch on raw performance.
2. **Coverage**: a one-person rewrite cannot match the thousand-plus operator surface of mature production libraries.
3. **Commitments**: production users expect ABI stability, support, and long-term roadmap commitments that solo bandwidth cannot sustain.

The Hexagonal "lite" architecture established in [ADR-0009](0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md) introduces a `KernelBackend` port that explicitly admits alternative backend adapters. After the Phase 2 build-out (PRs #10–#17), the *speed* claim is materially weakened: a CPU adapter built on [Eigen](https://eigen.tuxfamily.org/) or BLAS, and the Phase 3 [WebGPU](https://www.w3.org/TR/webgpu/) adapter from ADR-0006, can plug into the same Domain and bring production-grade performance on the operators the library does support.

The other two claims (coverage and commitments) still apply with full force. The question is therefore *not* whether to pivot to a production library wholesale, but whether to **refine** ADR-0001's narrative so that **production adoption becomes a permitted side effect rather than an actively discouraged one**.

[tinygrad](https://github.com/tinygrad/tinygrad) is the canonical example: explicitly minimal and educational by design, yet adopted in production for inference of large language models. tinygrad did not pivot — it shipped a strong educational artifact and let production users find it.

---

## Decision Drivers

- **DD-1**: Honor the Domain narrative. ADR-0001's *clarity > correctness > portability > performance* priority order must not flip.
- **DD-2**: Honor the Hexagonal design. If the architecture admits production-grade adapters, the positioning should not pretend otherwise.
- **DD-3**: Avoid pulling in commitments the solo maintainer cannot keep (ABI stability, semver guarantees, full operator coverage parity, formal support).
- **DD-4**: Preserve the educational pedagogy as the project's identity even if production adoption increases.

---

## Considered Options

1. **Status quo** — keep ADR-0001 strict; treat production use as out-of-scope and discouraged.
2. **Explicit production pivot** — rescind ADR-0001's educational positioning; commit to ABI stability, coverage growth, formal support.
3. **Refine ADR-0001** (chosen) — keep the educational identity, but acknowledge that:
   - production-grade backend adapters (Eigen / BLAS / Kokkos / WebGPU / …) are **first-class** behind the `KernelBackend` port,
   - production *adoption* is **permitted** and not actively discouraged,
   - the library still makes no ABI / coverage / support / SLA commitments — production users adopt on "as-is" terms.

---

## Decision Outcome

**Chosen option: 3 — Refine, not pivot.**

Concretely:

1. **Identity unchanged.** The library remains an *educational* artifact whose Domain is the readable, named-axis tensor algebra implementation. ADR-0001 stays in force in spirit and in the priority order *clarity > correctness > portability > performance*.
2. **Backends become first-class.** The `KernelBackend` port (declared anticipatorily in [`include/tensor/core/concepts.hpp`](../../../include/tensor/core/concepts.hpp), refined by the next ADR landing alongside Phase 2.5) admits multiple concrete adapters. The first non-reference adapter is **Eigen** in Phase 2.5; **WebGPU** in Phase 3 ([ADR-0006](0006-adopt-webgpu-as-gpu-backend.md)).
3. **Coverage and commitments unchanged.** No promise of operator-by-operator parity with Eigen / xtensor / libtorch. No ABI stability. No formal support. The README's production-disclaimer language softens from "*not a production library*" to "*production-capable on the supported operator set, but ships without ABI / coverage / support commitments — adopt as-is*".
4. **Positioning tagline** updates from "educational, not production" to **"educational-first, production-capable"**, paralleling tinygrad's stance.
5. **Tutorial implication.** A new chapter (planned: `tutorials/08_swappable-backends.ipynb`) explicitly demonstrates the architectural payoff: swap the reference adapter for Eigen, observe a speedup, confirm gradients still match. The architecture itself becomes more visible as a teaching artifact.

### Y-statement summary

> In the context of **completing Phase 2 and observing that the Hexagonal architecture (ADR-0009) genuinely admits production-grade backend adapters**, facing **the question of whether ADR-0001's hard refusal of production positioning still fits**, we decided for **refining (not superseding) ADR-0001 to "educational-first, production-capable via backend adapters"**, to achieve **a defensible niche that lets production adoption emerge as a side effect without committing the solo maintainer to ABI / coverage / support obligations**, accepting **that production users may adopt the library "as-is" with no compatibility or coverage guarantees**.

---

## Pros and Cons of the Options

### Option 1: Status quo (strict educational)

- Pros: simplest; no risk of production-driven scope creep.
- Cons: surrenders a genuine architectural payoff; misaligns the documentation with what the library can already do; closes the door on the tinygrad-shaped niche.

### Option 2: Explicit production pivot

- Pros: clears the air for production users; permits aggressive coverage growth.
- Cons: commits solo bandwidth to coverage parity, ABI stability, support, performance regression CI — the three barriers ADR-0001 originally flagged. Educational pedagogy gets dominated by production hygiene.

### Option 3: Refine (chosen)

- Pros: preserves educational identity; admits the architectural payoff honestly; matches tinygrad-style precedent; production adoption permitted but not invited; no new commitments.
- Cons: "educational-first, production-capable" is a more nuanced pitch than "educational, not production" — some readers will conflate the two. Mitigation: README and ADR-0001 disclaimers updated to spell out exactly what production-capable means and does not mean.

---

## Consequences

### Positive

- The `KernelBackend` port gets formal API design (in the ADR-0011 / Phase 2.5 work) rather than remaining an anticipatory marker.
- Eigen + Apple Accelerate / BLAS / Kokkos / WebGPU adapters are uniformly first-class. The build system option `TENSOR_KERNEL_BACKEND={reference, eigen, kokkos, webgpu, ...}` (or similar) selects at configure time.
- Performance-comparison demos become a legitimate teaching topic.
- Production users can adopt the library for niche workloads (named-axis algebra, _tex-driven kernels) without the maintainer needing to bless them.

### Negative

- README and project description must be updated; "not a production library" language is too strong.
- Some incoming issues will request features the project still won't add (coverage parity, ABI stability) — they must be politely declined with a link to this ADR.
- The educational pitch must work harder to stay visible above the backend-comparison material; risk of the project's identity drifting toward "C++ tensor benchmark" if not actively managed.

### Neutral

- ADR-0001 remains Accepted and in force; this ADR refines one claim within it, not the whole.
- The `0.1.0` milestone scope is unchanged; the production-capable framing kicks in *now* (Phase 2.5) and is mature by `0.1.0`.

### Follow-ups

- [ ] Write ADR-0011 (`KernelBackend` port API design) alongside Phase 2.5 work.
- [ ] Update `README.md` first paragraph and disclaimer block to reflect the refined positioning.
- [ ] Update [`docs/arc42/01-introduction-and-goals/overview.md`](../01-introduction-and-goals/overview.md) §1 system identity + §3 stakeholders to add a "production user (`as-is`)" entry.
- [ ] Update [`docs/arc42/04-solution-strategy/strategy.md`](../04-solution-strategy/strategy.md) top-7 navigation to insert ADR-0010 alongside ADR-0001.
- [ ] Add new Phase 2.5 impl-plan: `2026-05-11_phase-2-5-backend-port-and-eigen.md`.
- [ ] Plan tutorial `tutorials/08_swappable-backends.ipynb` for Phase 2.5 close.

---

## Compliance / Validation

- **Verification**: README and ADR-0001's §Stakeholders / §Goals / §Y-statement summary must remain consistent with this ADR's refinement. Any contradiction is a bug in the docs, not in the policy.
- **Frequency**: each time a Phase plan closes, re-audit the README and §1 overview for staleness.

---

## More Information

### Related ADRs

- Refines: [ADR-0001](0001-pivot-to-educational-named-axis-dsl.md)
- Architectural foundation: [ADR-0009](0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md)
- Specific backend: [ADR-0006](0006-adopt-webgpu-as-gpu-backend.md) — WebGPU is the second concrete `KernelBackend` adapter (after reference + Eigen).
- Future: ADR-0011 (`KernelBackend` port API design, alongside Phase 2.5).

### References

- [tinygrad](https://github.com/tinygrad/tinygrad) — the educational-first / production-capable precedent.
- [tinygrad's 2025 LLaMA / Stable Diffusion deployments](https://www.blog.brightcoding.dev/2025/09/08/tinygrad-the-ultra-minimal-deep-learning-library-that-runs-llama-and-stable-diffusion/) — empirical evidence the stance is defensible.
- [Eigen](https://eigen.tuxfamily.org/) — the planned first non-reference `KernelBackend` adapter.
- [Kokkos](https://github.com/kokkos/kokkos) — a potential multi-platform backend (CPU + GPU) bypassing the need for WebGPU as a separate concern; deferred for now per ADR-0006's per-platform reasoning.
- Original Phase 1 research brief: [`../../reports/2026-05-10_tensor-revival-landscape.md`](../../reports/2026-05-10_tensor-revival-landscape.md)
- Phase 1 retrospective: [`../../reports/2026-05-11_phase-1-retrospective.md`](../../reports/2026-05-11_phase-1-retrospective.md)
