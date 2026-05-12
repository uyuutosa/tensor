---
status: Draft
owner: tensor
last-reviewed: 2026-05-12
---

# `tensor` — Introduction and Goals

| Metadata      | Value                                                          |
| ------------- | -------------------------------------------------------------- |
| Status        | Draft                                                          |
| Type          | arc42 §1 (Introduction and Goals)                              |
| Owner         | uyuutosa                                                       |
| Related       | ADR-0001 — pivot decision; ADR-0013 (superseded by ADR-0015) + ADR-0015 — canonical-reference *quality* aspiration; ADR-0014 + ADR-0016 — external-substrate strategy + refinement; ADR-0017 — reproducibility envelope clarification (refines ADR-0015 §Compliance bullet 3); `docs/reports/2026-05-10_tensor-revival-landscape.md`; `docs/reports/2026-05-11_external-substrate-research.md`; `docs/reports/2026-05-12_gpu-cpp-dawn-abi-drift-and-raw-dawn-smoke.md`; `docs/reports/2026-05-12_landscape-recheck-and-adversarial-review.md` |
| Last Updated  | 2026-05-12                                                     |

## Revision history

| Version | Date       | Summary                                                        |
| ------- | ---------- | -------------------------------------------------------------- |
| 0.1.0   | 2026-05-10 | Initial draft after revival pivot (ADR-0001).                  |
| 0.2.0   | 2026-05-11 | Reframed as canonical reference (ADR-0013). Stakeholder table expanded with future-implementer audience; goals' phrasing updated to name the bibliography / ubiquitous-language / reproducibility disciplines. Success criteria refreshed against PR #1–#44 state. |
| 0.2.1   | 2026-05-11 | ADR-0015 supersedes ADR-0013: the three disciplines stay, but the *claim* moves from declarative ("is the canonical reference") to aspirational ("aspires to canonical-reference quality"). Per the maintainer's critique that self-declared canonical-reference status carries the same shape as self-declared SOTA — embarrassment risk if the community never adopts the work. TL;DR / §1 / §6 wording updated accordingly. |
| 0.2.2   | 2026-05-12 | ADR-0016 refines ADR-0014 §Decision Outcome point 2 (drop gpu.cpp wrapper, talk to Dawn directly via `webgpu_cpp.h`) after the local RTX 3090 Stage 2 smoke surfaced gpu.cpp@0.2.0's 14-month lag behind Dawn's async-callback API. Phase 3 P3.M3.2 + P3.M4.2 + P3.M5 dispatch wiring shipped on RTX 3090: 12 of 15 `KernelBackend` methods (4 binary + 4 unary + 1 contract + 3 broadcast) now execute real Dawn compute on `float`, cross-validated against reference within `1e-5` / `1e-3`. §1 "WebGPU in flight" wording replaced with the Phase 3 functionally-complete state; §6 success criteria refreshed against PR #1–#74 state; §7 cross-references list extended to ADR-0016 and enumerates the seven detailed-design instances. |
| 0.2.3   | 2026-05-12 | Strategic investigation cycle (PRs #78–#87) — landscape re-check + adversarial self-review under ADR-0015 §Compliance. **Strategy unchanged**: the four-prong wedge (named-axis + differentiable + header-only C++20 + canonical-reference framing) is uncontested in the 8-month window 2025-10 → 2026-05. ADR-0017 refines ADR-0015 §Compliance bullet 3 by splitting the 30-min reproducibility envelope into (a) build+test+bench and (b) separate notebook-audit step (after the audit caught that notebook execution alone is 20–50 min). Tutorial corpus grows from 5 to 6 notebooks — new `01_formula-is-the-program.ipynb` surfaces the headline `_tex` UDL feature that was buried in `00_intro` §5. Glossary gains six new entries (`FixedString` / `LabelTag` / `BroadcastPlan` / `ContractPlan` / `AxisLike` / `ShapeLike`) closing the largest gap in the ubiquitous-language discipline; `KernelBackend` + `Dawn` entries refreshed for post-#60 reality; new Mojo MAX `NamedMapping` vocabulary disambiguation entry. §7 cross-references list extended to ADR-0017. |

---

## TL;DR

`tensor` aspires to **canonical-reference-quality** documentation and design for differentiable named-axis tensor algebra in modern C++ ([ADR-0015](../09-decisions/0015-aspire-to-canonical-reference-quality-not-self-anoint.md), superseding [ADR-0013](../09-decisions/0013-reframe-as-canonical-reference-for-named-tensor-computation.md); refining [ADR-0001](../09-decisions/0001-pivot-to-educational-named-axis-dsl.md) + [ADR-0010](../09-decisions/0010-refine-positioning-to-educational-first-production-capable.md)). It is a **header-only C++20/23 library and bundled Jupyter book** that develops named-axis tensor algebra — Einstein-notation operations, function tensors, reference (recursive) tensors, and convolutions reformulated as tensor inner products — through automatic differentiation and onto WebGPU acceleration. Authoring is bidirectional with TeX / LyX so that, in the project's own slogan, *the formula is the program*. The Domain is intentionally small and readable; speed for production-shaped workloads comes from swappable `KernelBackend` adapters (reference, Eigen, WebGPU). Production adoption is permitted on **as-is terms** per [ADR-0010](../09-decisions/0010-refine-positioning-to-educational-first-production-capable.md) — no ABI / coverage / support commitments. The three forcing-function disciplines (bibliography / ubiquitous language / reproducibility) keep the work behaving like a canonical reference *should* — whether external adoption ever bestows the label is determined by the community, not by self-declaration. See [`CITATION.cff`](../../../CITATION.cff) for citation metadata.

---

## 1. System identity

The `tensor` repository began in ~2016 as a personal C++ template library built around an unusual idea: tensors that carry **axis labels** (`a_i`, `b_j`) and operate on each other through Einstein-style broadcasting (`a_i + b_j → c_{ij}`). The library shipped function-tensors (tensor elements that are functions), reference-tensors (tensor elements that recursively reference the previous element along an axis), and a [proof-of-concept reformulation of N-D convolution as a tensor inner product](http://qiita.com/uyuutosa/items/12e87f4695bd151b1d74).

It went dormant after a single commit run. In May 2026 the project was revived under explicit new positioning ([ADR-0001](../09-decisions/0001-pivot-to-educational-named-axis-dsl.md), refined by [ADR-0010](../09-decisions/0010-refine-positioning-to-educational-first-production-capable.md), reframed by [ADR-0013](../09-decisions/0013-reframe-as-canonical-reference-for-named-tensor-computation.md) and then again by [ADR-0015](../09-decisions/0015-aspire-to-canonical-reference-quality-not-self-anoint.md)): **aspires to canonical-reference-quality** documentation and design — educational-first, production-capable via backend adapters. The Domain stays small and readable; production-grade speed is available through swappable adapters; the project's documentation, naming, and reproducibility disciplines aim at a single goal: *to be ready when external implementations decide to cite, port, or extend the work*. ADR-0015 explicitly distinguishes this aspirational framing from SOTA-shaped self-declaration: canonical-reference status is bestowed by external adoption, not announced by the author. The library does not commit to operator-coverage parity with Eigen / xtensor / libtorch / Kokkos / `std::linalg` and offers no ABI / support guarantee; production users adopt as-is.

The 2016 codebase has been retired to `archive/legacy-2016/`. The rewrite targets C++20 + `mdspan` interop ([ADR-0002](../09-decisions/0002-rewrite-on-cpp20-baseline-with-mdspan-interop.md)), CMake + vcpkg ([ADR-0003](../09-decisions/0003-replace-eclipse-cdt-with-cmake-and-vcpkg.md)), and a hybrid runtime / NTTP named-axis API ([ADR-0004](../09-decisions/0004-adopt-hybrid-named-axis-api.md)). External-substrate decisions (Dawn via vcpkg, gpu.cpp vendored, xeus-cpp for notebooks, kokkos/stdBLAS shim) are bundled in [ADR-0014](../09-decisions/0014-external-substrate-strategy.md).

## 2. Stakeholders

| Stakeholder                                                | Interest                                                                                  |
| ---------------------------------------------------------- | ----------------------------------------------------------------------------------------- |
| **Future implementers / researchers / textbook authors**   | Want to **cite, port, or build on top of** this work. ADR sequence is the bibliography; public names trace to sources. Primary audience per [ADR-0013](../09-decisions/0013-reframe-as-canonical-reference-for-named-tensor-computation.md) / [ADR-0015](../09-decisions/0015-aspire-to-canonical-reference-quality-not-self-anoint.md). |
| Self-taught C++ learners exploring tensor algebra          | Want a small, hackable, idiomatic library + readable notebooks. Co-primary audience; the Diátaxis split (tutorials easy, references rigorous) preserves accessibility. |
| University / bootcamp instructors                          | Want assignable teaching material for "build your own ML framework" or "modern C++" courses; want to cite the ADR sequence as a worked example of MADR + Hexagonal-lite discipline. |
| Researchers in tensor-DSL design                           | Want a real working artifact to reference for named-axis API ergonomics in C++, the `_tex` ↔ AST ↔ DynamicTensor round-trip, and the LyX integration story. |
| Production users (`as-is`)                                 | Adopt the library for niche workloads (named-axis algebra, `_tex`-driven kernels) on top of a fast `KernelBackend` adapter (reference + Eigen + WebGPU all shipped as of 2026-05-12; WebGPU dispatches real Dawn compute on 12 of 15 methods for `float`). **No ABI / coverage / support commitments** — see [ADR-0010](../09-decisions/0010-refine-positioning-to-educational-first-production-capable.md). |
| The maintainer (uyuutosa)                                  | Wants to revive a long-dormant idea and produce a public artifact future implementations can cite. |

## 3. Goals (G-1 … G-7)

Each goal is phrased so success is observable.

- **G-1 — Named-axis algebra is the headline feature.** Every public operation respects axis labels. Mismatched axes are caught at compile time when the user opts in via `_ax` literals, at runtime otherwise. (See [ADR-0004](../09-decisions/0004-adopt-hybrid-named-axis-api.md).)
- **G-2 — Modern C++ idioms throughout.** No raw `new` / owning references. Concepts, NTTPs, `consteval`, ranges, `mdspan` interop. (See [ADR-0002](../09-decisions/0002-rewrite-on-cpp20-baseline-with-mdspan-interop.md).)
- **G-3 — "The formula is the program".** A LaTeX subset (Einstein notation, `\sum`, indexed arithmetic) parses at compile time via `_tex` UDL into the same expression graph as direct C++ tensor expressions. LyX export is a follow-on deliverable. (See [ADR-0005](../09-decisions/0005-adopt-tex-lyx-as-authoring-surface.md).)
- **G-4 — End-to-end teaching arc.** The bundled tutorials walk a learner from `a_i + b_j` through autograd to a small MLP on MNIST, with optional WebGPU acceleration. (See [ADR-0007](../09-decisions/0007-adopt-autograd-as-first-class-subsystem.md), [ADR-0008](../09-decisions/0008-distribute-as-header-only-with-jupyter-tutorials.md).)
- **G-5 — Zero-friction installation for learners.** `git clone && cmake --preset=default && jupyter lab tutorials/` works on Linux, macOS, Windows. No proprietary GPU toolchain required. (See [ADR-0003](../09-decisions/0003-replace-eclipse-cdt-with-cmake-and-vcpkg.md), [ADR-0006](../09-decisions/0006-adopt-webgpu-as-gpu-backend.md).)
- **G-6 — Living book, not abandoned tutorial.** A Jupyter Book site is published from the `tutorials/` directory at every release; CI executes every notebook end-to-end. (See [ADR-0008](../09-decisions/0008-distribute-as-header-only-with-jupyter-tutorials.md).)
- **G-7 — Honest scope disclaimers.** README, repo description, and Jupyter Book front-page state plainly that `tensor` is educational-first; production users are pointed at Eigen / xtensor / libtorch / Kokkos / `std::linalg`. No benchmark-shopping.
- **G-8 — Citability discipline ([ADR-0015](../09-decisions/0015-aspire-to-canonical-reference-quality-not-self-anoint.md), superseding [ADR-0013](../09-decisions/0013-reframe-as-canonical-reference-for-named-tensor-computation.md)).** Three sub-disciplines (forcing functions, not claims about external recognition): (i) **bibliography** — every notable design decision is traceable to a paper, an ADR, or both; the ADR sequence is the bibliography. (ii) **ubiquitous language** — public names map to names in the math literature ([ADR-0009](../09-decisions/0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md)). (iii) **reproducibility** — clean clone → build + bench + notebook execution in under 30 minutes on a typical laptop, with results matching the documentation. A `CITATION.cff` at the repo root makes the work citable by name. *Whether the work earns canonical-reference status from external adoption is a separate question; G-8 makes the work ready, not the announcement.*

## 4. Quality goals

In priority order (top three):

| Priority | Quality       | Why                                                                                                  |
| -------- | ------------- | ---------------------------------------------------------------------------------------------------- |
| 1        | **Clarity**   | Every public API surface, every notebook, every diagnostic message is optimized for *being read*.    |
| 2        | **Correctness** | Mathematical correctness is non-negotiable for a teaching artifact. All ops have property tests; gradients are gradient-checked against finite differences. |
| 3        | **Portability** | Must build on GCC 11+, Clang 13+, MSVC 19.30+, and run notebook tutorials on Linux/macOS/Windows. |

Performance is explicitly ranked **below** clarity and correctness. The library is allowed to be slower than Eigen / libtorch by an order of magnitude as long as the implementation reads like the math and the diagnostics are clear. (See [ADR-0001](../09-decisions/0001-pivot-to-educational-named-axis-dsl.md).)

## 5. Out of scope

Captured here so future reviewers do not litigate them as gaps.

- **Production deployment.** No ABI stability, no semver-after-1.0 commitments, no LTS branches.
- **Coverage parity with Eigen / xtensor / libtorch.** We will skip operations that do not teach.
- **Distributed training.** Single-node, period.
- **Sparse tensors as a first-class citizen.** Possible later; not in the first release.
- **CUDA-direct GPU.** Disqualified in [ADR-0006](../09-decisions/0006-adopt-webgpu-as-gpu-backend.md) on educational-cost-of-entry grounds.
- **Forward-mode and source-to-source autograd.** Reachable as future complementary backends ([ADR-0007](../09-decisions/0007-adopt-autograd-as-first-class-subsystem.md)).

## 6. Success criteria (concrete, measurable)

The first release is `0.1.0`. Success at `0.1.0` means:

- [x] All ADR-0001 follow-ups are closed. *(achieved 2026-05-12: Phase 1 + 1.5 + 2 + 2.5 + 3 all shipped per `book/roadmap.md`; the "educational-first, production-capable via backend adapters" north star refined by ADR-0010 / ADR-0015 is fully realised in shipped code — three `KernelBackend` adapters (reference + Eigen + WebGPU) with 12 of 15 methods on real GPU per PRs #60–#62, autograd Phase 2 + `zero_grad` + `sgd_update`, `_tex` UDL + Evaluator + LyX export, 17 ADRs, 7 detailed-design instances, 6 notebooks.)*
- [x] CMake build green on the {GCC 11, Clang 13, MSVC 19.30} × {Debug, Release} matrix in CI. *(achieved 2026-05-11 via PR #9; matrix grew to 10 jobs via PRs #21, #38, #41.)*
- [x] `tutorials/00_intro.ipynb` reproduces the 2016 blog post's convolutions-as-inner-products in the rewritten library. *(shipped 2026-05-10 via PR #8; notebook-CI execute path now targets xeus-cpp 0.10+ per [ADR-0014](../09-decisions/0014-external-substrate-strategy.md), with xeus-cling smoke retained.)*
- [x] `tutorials/05_autograd-from-scratch.ipynb` walks the autograd implementation primitive-by-primitive. *(shipped 2026-05-11 via PR #15.)*
- [x] `tutorials/07_mlp-on-toy.ipynb` trains a small MLP on a toy dataset, converging to W≈2, b≈1. *(shipped 2026-05-11 via PR #17.)*
- [x] `tutorials/08_swappable-backends.ipynb` demonstrates the Hexagonal-lite payoff (same Domain code on reference, Eigen). *(shipped 2026-05-11 via PR #22.)*
- [x] `tutorials/06_webgpu-acceleration.ipynb` exists and walks the reader through Phase 3 end-to-end. *(Shipped in Option 3 "design walkthrough" form per Phase 4 release rehearsal #48; P3.M3.2 + P3.M4.2 dispatch wiring shipped subsequently — the 8 element-wise WGSL kernels and the tiled GEMM kernel all run on real GPU and are cross-validated against the reference backend within `1e-5` / `1e-3` for `float`. The notebook will gain executable cells in a follow-up PR; the design walkthrough remains current.)*
- [ ] Jupyter Book site is reachable at the repo's GitHub Pages URL. *(Phase 4 — scaffold + deploy workflow shipped via PR #28; site goes live when Pages is enabled.)*
- [x] README's first paragraph leads with the canonical-reference-quality aspiration framing and disclaims production use. *(canonical-reference framing landed via PR #39, reframed to aspirational by ADR-0015.)*
- [x] `CITATION.cff` at the repo root names the work, the maintainer, and the bibliographic ADRs. *(shipped 2026-05-11 via PR #40.)*
- [x] `third_party/` content is vendored under `VENDORED_FROM` discipline with CI enforcement. *(shipped 2026-05-11 via PR #41.)*

For interim phase-by-phase deliverables, see the dated impl-plans under [`../../impl-plans/`](../../impl-plans/).

## 7. Cross-references

- §2 Architecture Constraints: [`../02-architecture-constraints/overview.md`](../02-architecture-constraints/overview.md) — TC-1..TC-9 + OC-1..OC-5.
- §3 Context and Scope: [`../03-context-and-scope/system-context.md`](../03-context-and-scope/system-context.md).
- §4 Solution Strategy: [`../04-solution-strategy/strategy.md`](../04-solution-strategy/strategy.md).
- §5 Building Blocks: [`../05-building-blocks/overview.md`](../05-building-blocks/overview.md).
- §9 Decisions: [ADR-0001](../09-decisions/0001-pivot-to-educational-named-axis-dsl.md) … [ADR-0017](../09-decisions/0017-clarify-reproducibility-envelope.md). The supersession + refinement chain in force as of 2026-05-12: ADR-0013 superseded by ADR-0015; ADR-0014 §Decision Outcome point 2 refined by ADR-0016; ADR-0015 §Compliance bullet 3 refined by ADR-0017.
- `CITATION.cff` at the repo root (the citable metadata for this work).
- Detailed designs ([`../../detailed-design/`](../../detailed-design/)): seven Template-3 instances covering the Domain quad (`tensor-core.md`, `tensor-autograd.md`, `tensor-tex.md`) + the WebGPU adapter trio (`webgpu-element-wise-kernels.md`, `webgpu-gemm-kernel.md`, `webgpu-broadcast-kernels.md`) + the port surface (`kernel-backend-port.md`).
- §10 Quality: [`../10-quality/overview.md`](../10-quality/overview.md) — quality tree + measurable scenarios (QC-1..QS-2).
