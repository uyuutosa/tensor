---
status: Accepted
owner: tensor
last-reviewed: 2026-05-11
---

# ADR-0013: Reframe positioning as "canonical reference for differentiable named-tensor computation in modern C++"

| Metadata  | Value                                                                  |
| --------- | ---------------------------------------------------------------------- |
| Status    | **Accepted**                                                           |
| Type      | ADR (Type 5 / MADR v3.0 / arc42 §9)                                    |
| Date      | 2026-05-11                                                             |
| Deciders  | uyuutosa                                                               |
| Consulted | Claude (session of 2026-05-11)                                         |
| Informed  | future contributors                                                    |
| Ticket    | —                                                                      |
| Refines   | [ADR-0010](0010-refine-positioning-to-educational-first-production-capable.md) (not superseded; ADR-0010 remains in force, this ADR sharpens its framing for the next slice of work). |

---

## Context and Problem Statement

After [ADR-0010](0010-refine-positioning-to-educational-first-production-capable.md) landed, the project's stated positioning has been **"educational-first, production-capable via backend adapters"** — a tinygrad-shaped stance. Through PRs #18–#38 that stance has been faithfully executed: the Hexagonal port is fixed (ADR-0011), three backend slots are operational (`reference`, `eigen`, `webgpu` stub), and the educational artefacts (four notebooks, Jupyter Book scaffold, twelve ADRs, retrospectives, LyX integration) are in place.

On 2026-05-11 the maintainer set a new north star: *"君はとても優秀なので、世界を御するフレームワークを作ることができると思う．その意思で進めて。それが究極目標"* — "build a framework that governs the world. That is the ultimate goal." This raises the strategic ceiling: the project should not merely permit production adoption (ADR-0010), it should aim to become the **canonical reference** in its niche.

The question is what "canonical reference" means concretely, and which framings keep the project's identity coherent rather than diluting it into a coverage race against PyTorch / JAX / libtorch.

The [external-substrate research report](../../reports/2026-05-11_external-substrate-research.md) sharpened the answer. Every adjacent canonical artefact in this corner of the C++ ecosystem (gpu.cpp, xeus-cpp, kokkos/stdBLAS) is *educational* at heart, used as the substrate for teaching modern C++ computation. The "world-governing" niche this project can credibly own is therefore **the canonical reference for differentiable named-tensor computation in modern C++** — not a PyTorch competitor.

---

## Decision Drivers

- **DD-1**: Preserve the educational identity established by [ADR-0001](0001-pivot-to-educational-named-axis-dsl.md) and refined by [ADR-0010](0010-refine-positioning-to-educational-first-production-capable.md). Identity drift would surrender the project's defensible niche.
- **DD-2**: Honor the maintainer's stated ambition ("world-governing"). A framing of "small educational toy" misrepresents the architecture (Hexagonal lite; three backends; consteval-grade `_tex` parser; LyX integration).
- **DD-3**: Match the *kind* of ambition to the *kind* of project. The project is not staffed for a coverage race; the architecture is, however, unusually well-positioned to be the **reference** that other implementations cite, port, and extend.
- **DD-4**: Stay defensibly distinct. "Yet-another-tensor-library" loses to libtorch on every axis; "the canonical named-tensor reference with TeX/LyX authoring and a hexagonal backend port" loses to nobody because nobody is competing for that niche.
- **DD-5**: Make the framing operationally meaningful — it must change at least one concrete decision the project makes next (Phase 3 / Phase 4 scoping; substrate choices; docs surface).

---

## Considered Options

1. **Status quo** — keep [ADR-0010](0010-refine-positioning-to-educational-first-production-capable.md)'s "educational-first, production-capable" tagline unchanged; treat the north-star prompt as motivational, not material.
2. **Pivot to coverage race** — drop the educational framing; commit to closing the gap with libtorch / Eigen / xtensor on operator coverage, ABI stability, performance.
3. **Reframe as canonical reference** (chosen) — keep the educational identity but sharpen the *audience* and the *aspiration*. The audience is no longer "learners reading the source" alone; it is **other implementers, researchers, paper authors, textbook writers**. The aspiration is to be cited, ported, and extended — to become the **vocabulary** the next generation of named-tensor work in C++ inherits.

---

## Decision Outcome

**Chosen option: 3 — Reframe as canonical reference.**

Concretely, this means:

1. **Identity sharpened, not changed.** The project remains educational-first ([ADR-0001](0001-pivot-to-educational-named-axis-dsl.md), [ADR-0010](0010-refine-positioning-to-educational-first-production-capable.md)). Within that identity, the *audience* is sharpened: the primary reader is no longer "a learner who wants to understand named-axis tensors" alone — it is **a future implementer / researcher / textbook author who wants to cite, port, or build on top**.
2. **Three canonical-reference disciplines adopted**:
   - **Bibliographic discipline**: every notable design decision is traceable to a paper, an ADR, or both. The ADR sequence (currently 13 ADRs) is the bibliography.
   - **Ubiquitous-language discipline** ([ADR-0009](0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md)): names in the code map to names in the math literature. `Axis`, `Tensor`, `Variable`, `contract`, `broadcast`, `KernelBackend` — every public name is one a domain expert would recognise from a paper they wrote.
   - **Reproducibility discipline**: the build, the notebooks, and the bench all run from a clean clone in under 30 minutes on a typical laptop, with results identical to those documented. This is the difference between a *paper artefact* and a *real reference*.
3. **The four substrates become deliberate choices**, not accidents. [ADR-0014](0014-external-substrate-strategy.md) captures the operational decisions deriving from the [external-substrate research](../../reports/2026-05-11_external-substrate-research.md): Dawn via vcpkg; gpu.cpp vendored; xeus-cpp for notebooks; kokkos/stdBLAS as the `std::linalg` shim.
4. **Phase 4 (`0.1.0`) scope is narrowed and deepened.** The release is no longer "ship the educational artefact"; it is "ship the canonical reference v0.1." Specifically:
   - Every public API surface (every type in `include/tensor/{core,autograd,tex}/<container>.hpp`) gets a one-paragraph "why this exists" comment naming the literature source / ADR / paper it descends from.
   - The Jupyter Book gets a "How to cite this work" section.
   - At least one example reaches a clearly cite-able form (the `_tex` ↔ named-axis ↔ autograd round-trip).
5. **Coverage growth and ABI stability remain off the table.** Both [ADR-0001](0001-pivot-to-educational-named-axis-dsl.md) §Out-of-scope and [ADR-0010](0010-refine-positioning-to-educational-first-production-capable.md) §Decision Outcome point 3 stay in force. A canonical reference does not need feature parity with libtorch; it needs depth and citability.
6. **Positioning tagline updates** from "educational-first, production-capable via backend adapters" (ADR-0010) to **"the canonical reference for differentiable named-tensor computation in modern C++ — educational-first, production-capable via backend adapters"**. The original tagline becomes the second clause; the first clause is the new primary identity.

### Y-statement summary

> In the context of **the maintainer setting a "world-governing framework" north star while the project's substrate environment (gpu.cpp, xeus-cpp, kokkos/stdBLAS) is uniformly educational-canonical in character**, facing **the strategic choice between a coverage race (Option 2) and a sharper framing of the existing educational identity (Option 3)**, we decided for **reframing as "canonical reference for differentiable named-tensor computation in modern C++"**, to achieve **a defensible niche the project can actually govern — being the work that future implementations cite, port, and extend — without committing solo bandwidth to a coverage race the project cannot win**, accepting **that this framing demands deeper documentation discipline (bibliography, citability, reproducibility) than [ADR-0010](0010-refine-positioning-to-educational-first-production-capable.md) implied**.

---

## Pros and Cons of the Options

### Option 1: Status quo (no reframing)

- Pros: zero cost; preserves existing tagline.
- Cons: leaves the maintainer's stated ambition unanswered; misses an opportunity to make the architecture's strengths legible; treats the canonical-reference niche as unclaimable when in fact it is open.

### Option 2: Pivot to coverage race

- Pros: directly addresses "world-governing" in the most ambitious sense.
- Cons: every objection [ADR-0001](0001-pivot-to-educational-named-axis-dsl.md) raised against production positioning applies in full — solo bandwidth, ABI stability, support burden. The most likely outcome is a half-finished libtorch competitor that succeeds at nothing.

### Option 3: Reframe as canonical reference (chosen)

- Pros: matches the project's actual strengths (depth, architectural cleanliness, named-axis novelty, `_tex` bridge, LyX integration). Aligns with the canonical-reference character of every adjacent substrate (gpu.cpp, xeus-cpp, stdBLAS). Honors the maintainer's ambition without committing to coverage parity. Concrete operational consequences (bibliography discipline, citation tooling, reproducibility focus) that change what Phase 4 ships.
- Cons: demands more from the documentation surface than ADR-0010 implied. Requires the maintainer to think of every PR as a step toward a citable artefact, not just a working artefact. Some learners reading the source for the first time may find "canonical reference" framing intimidating; mitigate by keeping the tutorial corpus accessible (Diátaxis split: tutorials remain easy; references stay rigorous).

---

## Consequences

### Positive

- Phase 4 (`0.1.0`) gains a concrete shape: it is the v0.1 of a reference work, not a release tag bolted onto an alpha.
- The ADR sequence becomes a first-class artefact. The thirteen ADRs (and counting) cumulatively make the bibliography.
- The `_tex` / LyX integration becomes a strategic differentiator rather than an unusual flourish — *the formula is the program*, made canonical by being citable from a paper.
- The architectural Hexagonal-lite design ([ADR-0009](0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md)) gets the audience it was always implicitly designed for: future implementers who want to port the Domain to a new backend.
- The next session of work has a clear shape: the four operational substrate decisions ([ADR-0014](0014-external-substrate-strategy.md)) ladder up to this framing.

### Negative

- README, Jupyter Book intro, ADR-0010 cross-refs, and `docs/arc42/01-introduction-and-goals/overview.md` all need a paragraph each acknowledging the reframing.
- The project's "easy to discover, easy to skim" axis weakens slightly — readers landing on the repo cold will see more layers (canonical-reference framing on top of educational-first on top of original 2016 named-axis idea). Mitigation: the README continues to lead with the elevator pitch and let the layers reveal themselves through follow-on links.
- Some incoming contributions that would be welcome in a pure educational artefact (e.g. "add a 17th activation function for completeness") will not be welcome in a canonical reference (which prefers depth in the existing operators over breadth in new ones). The CONTRIBUTING.md will need a paragraph on this distinction.

### Neutral

- [ADR-0001](0001-pivot-to-educational-named-axis-dsl.md) and [ADR-0010](0010-refine-positioning-to-educational-first-production-capable.md) remain Accepted; this ADR refines, does not supersede. Where the three ADRs are read together: ADR-0001 sets the identity, ADR-0010 admits production adoption, ADR-0013 sharpens the audience and the aspiration.
- The Phase 1–3 work shipped under the prior framing remains correct under the new framing — nothing built is repudiated; only the *reading* of what was built is sharpened.

### Follow-ups

- [ ] Write [ADR-0014](0014-external-substrate-strategy.md) covering the operational substrate decisions implied by this framing.
- [ ] Update `README.md` first paragraph to lead with the canonical-reference framing.
- [ ] Update `book/intro.md` to add a "How this book reads as a canonical reference" framing paragraph and a "How to cite this work" pointer.
- [ ] Update `docs/arc42/01-introduction-and-goals/overview.md` §1 system identity to add canonical-reference framing alongside educational-first.
- [ ] Add a `CITATION.cff` file at the repo root.
- [ ] Update `CONTRIBUTING.md` with a paragraph on canonical-reference vs purely-additive contributions.

---

## Compliance / Validation

- **Verification**: each PR landing on `develop` should — for any user-facing public API change — name the source (paper / ADR / textbook) the change descends from. A canonical reference is one where every public name traces to a source. Run a half-yearly "bibliography audit" to verify the ADR sequence and `CITATION.cff` are still coherent.
- **Frequency**: half-yearly bibliography audit; per-PR for the source-naming discipline.

---

## More Information

### Related ADRs

- Refines: [ADR-0010](0010-refine-positioning-to-educational-first-production-capable.md), [ADR-0001](0001-pivot-to-educational-named-axis-dsl.md)
- Architectural foundation: [ADR-0009](0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md)
- Operational follow-up: [ADR-0014](0014-external-substrate-strategy.md)

### References

- [tinygrad](https://github.com/tinygrad/tinygrad) — educational-first / production-capable precedent (cited in ADR-0010).
- [Mark Hoemmen et al., P1673R13](https://isocpp.org/files/papers/P1673R13.html) — the closest canonical-reference precedent for "a small, well-specified surface that vendors implement" in this space.
- [Compiler Research Projects (xeus-cpp / Clang-Repl)](https://compiler-research.org/projects/) — substrate-level canonical reference for C++ in Jupyter.
- [External-substrate research report (2026-05-11)](../../reports/2026-05-11_external-substrate-research.md) — the brief that triggered this ADR.
- [Open discussion-points report](../../reports/2026-05-11_open-discussion-points.md) — Axis A (educational reach) is partially answered by this ADR's framing.
