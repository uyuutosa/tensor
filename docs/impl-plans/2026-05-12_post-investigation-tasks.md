---
status: Active
owner: tensor
date: 2026-05-12
type: Layer B implementation plan
---

# Post-investigation tasks (2026-05-12)

> Implementation plan derived from [`docs/reports/2026-05-12_landscape-recheck-and-adversarial-review.md`](../reports/2026-05-12_landscape-recheck-and-adversarial-review.md) §4 findings table. Eight findings → nine tasks (F-3 splits into 3a / 3b / 3c). Each task is single-PR, single-purpose, and traceable to a finding ID.

| Metadata     | Value                                                                            |
| ------------ | -------------------------------------------------------------------------------- |
| Layer        | B (volatile working material — dated, append-only)                              |
| Predecessor  | [`2026-05-12_landscape-recheck-and-adversarial-review.md`](../reports/2026-05-12_landscape-recheck-and-adversarial-review.md) |
| Target window| 2026-05-12 → `0.1.0` cut (maintainer-only ceremony)                              |
| Out of scope | The maintainer's `0.1.0` release ceremony (release branch / tag / Pages enable)  |

---

## Task summary

| ID  | Finding | Title                                                                          | Effort | Priority   | Workflow target |
| --- | ------- | ------------------------------------------------------------------------------ | ------ | ---------- | --------------- |
| T-1 | F-3a    | Refresh glossary `KernelBackend` entry — drop "stub, dispatch wiring deferred" | XS     | **P0** (pre-cut, 🔴 High) | arc42 §12 update |
| T-2 | F-2     | Update `vcpkg.json:5` description to ADR-0015 aspirational wording             | XS     | **P0** (pre-cut)          | metadata only    |
| T-3 | F-3b    | Refresh glossary `Dawn` entry — Phase 3 shipped, not "planned"                | XS     | **P0** (pre-cut)          | arc42 §12 update |
| T-4 | F-3c    | Add glossary entries for `FixedString`, `LabelTag`, `BroadcastPlan`, `ContractPlan`, `AxisLike` (+ optional `DynamicTensor`) | S | **P0** (pre-cut) | arc42 §12 update |
| T-5 | F-1     | Add glossary entry disambiguating "named-axis" (this project) vs Mojo MAX `NamedMapping` (mesh sharding) | XS | **P0** (pre-cut) | arc42 §12 update |
| T-6 | F-7     | Add prerequisites preamble cell to tutorial 06                                  | XS     | **P0** (pre-cut)          | `tutorials/06_*.ipynb` |
| T-7 | F-8     | Add learning-arc dependency map to `tutorials/README.md`                        | XS     | **P0** (pre-cut)          | `tutorials/README.md` |
| T-8 | F-4     | ADR-0017 — clarify reproducibility envelope (build + test + bench; notebook separate) | M | **P1** (pre or post cut) | new ADR (`docs/templates/5_adr.md`) |
| T-9 | F-6     | New `tutorials/01_formula-is-the-program.ipynb`                                 | M      | **P1** (pre or post cut)  | `tutorials/` |

**P0** = recommended before `0.1.0` cut (low-effort, high-clarity-yield).
**P1** = strategically meaningful but does not block release; lands either side of the cut.

Effort scale: XS ≤ 30 min · S ≤ 2 hr · M ≤ 1 day · L > 1 day.

---

## T-1 — Refresh glossary `KernelBackend` entry (F-3a, 🔴 High)

**Why P0**: the only 🔴 High finding. An external reader checking the project's own glossary against the changelog will catch this immediately — a glossary that says "stub, dispatch wiring deferred" when the changelog shows 12 of 15 methods on real GPU is the cleanest possible self-contradiction.

**Scope**: one-paragraph edit at [`docs/arc42/12-glossary/overview.md:140`](../arc42/12-glossary/overview.md).

**Concrete change**: replace the parenthetical "(stub, dispatch wiring deferred)" with "12 of 15 methods dispatch real Dawn compute on `float` on RTX 3090; the rest delegate to reference, matching the Eigen adapter's scope."

**Branch / PR title**: `feature/glossary-refresh-kernelbackend-dawn-entries` (bundle with T-3).
`docs(glossary): refresh KernelBackend + Dawn entries for Phase 3 shipped state`

**Dependency**: none.

## T-2 — Update `vcpkg.json:5` description to ADR-0015 wording (F-2)

**Why P0**: it's the *first* line a vcpkg consumer reads about the project. If GitHub's Code Search ever surfaces the `vcpkg.json` description, it should match the canonical aspirational framing, not the superseded declarative one.

**Scope**: one-line edit at [`vcpkg.json:5`](../../vcpkg.json).

**Concrete change**:

- Before: `"Header-only C++20/23 library — the canonical reference for differentiable named-axis tensor computation in modern C++ (ADR-0013)."`
- After:  `"Header-only C++20/23 named-axis differentiable tensor library aspiring to canonical-reference quality (ADR-0015, superseding ADR-0013)."`

**Branch / PR title**: `feature/vcpkg-description-adr-0015`
`chore(vcpkg): update description to ADR-0015 aspirational wording`

**Dependency**: none.

## T-3 — Refresh glossary `Dawn` entry (F-3b)

**Why P0**: the same self-contradiction class as T-1, one tier softer.

**Scope**: paragraph edit at [`docs/arc42/12-glossary/overview.md:161`](../arc42/12-glossary/overview.md).

**Concrete change**: rewrite the entry to describe the actual shipped state — `vcpkg install 'dawn[core,vulkan]'` plus `find_package(Dawn CONFIG REQUIRED)` is what PRs #60 / #61 / #62 used on RTX 3090. The "vcpkg baseline bump" path is documented as a future option for users who want Dawn without a separate `vcpkg install` step.

**Branch / PR title**: bundled with T-1 above.

**Dependency**: same PR as T-1.

## T-4 — Add glossary entries for missing public symbols (F-3c)

**Why P0**: closes the largest gap in the ubiquitous-language discipline (5 missing entries out of ~27 public symbols ≈ 20% gap).

**Scope**: five new entries (one optional sixth) in [`docs/arc42/12-glossary/overview.md`](../arc42/12-glossary/overview.md):

| New entry        | Section                | One-line summary                                                                  |
| ---------------- | ---------------------- | --------------------------------------------------------------------------------- |
| `FixedString<N>` | Domain (after `Axis`)  | NTTP-friendly fixed-length string type backing the `_ax` UDL (ADR-0004).          |
| `LabelTag<S>`    | Domain (after `Axis`)  | Compile-time form of `Axis`; runtime-`Axis` and `LabelTag<S>` are the hybrid pair.|
| `BroadcastPlan`  | Domain (after `Broadcast`) | Pre-computed index map for an Einstein-style broadcast; decouples algebra from byte-level dispatch. |
| `ContractPlan`   | Domain (after `Contraction`) | Pre-computed index map for an Einstein-style contraction; same role as `BroadcastPlan`. |
| `AxisLike`       | Architecture           | C++20 concept; compile-time constraint on axis-template arguments.                |
| `DynamicTensor<T>` (optional) | Domain (after `Tensor`) | Runtime-rank twin of `Tensor<T, N>`. Currently covered inline in `Tensor` entry. |

**Branch / PR title**: `feature/glossary-add-missing-public-symbol-entries`
`docs(glossary): add entries for FixedString / LabelTag / BroadcastPlan / ContractPlan / AxisLike`

**Dependency**: prefer landing after T-1 + T-3 so the file does not have stale + missing entries simultaneously, but technically parallel-able.

## T-5 — Add Mojo MAX `NamedMapping` disambiguation entry (F-1)

**Why P0**: a reader who searches "Mojo named tensor" in 2026-05 will find Modular 26.3's mesh-sharding `NamedMapping`. The two uses of "named" have nothing to do with each other and the glossary is exactly where the disambiguation belongs.

**Scope**: one entry under "Architecture (cross-cutting)" in [`docs/arc42/12-glossary/overview.md`](../arc42/12-glossary/overview.md):

> **Named-axis (this project) vs `NamedMapping` (Modular MAX)** — the project's "named-axis" terminology refers to per-tensor semantic axis labels (`a_i + b_j → c_{ij}`, ADR-0004). [Modular MAX](https://docs.modular.com/max/changelog/)'s `NamedMapping` (introduced 2026-04 with Mojo 1.0 beta) is the JAX-`PartitionSpec`-equivalent for naming **mesh axes for distributed sharding**, not for naming tensor semantic axes. The two terms address different problems and are not interoperable. If a reader arriving from a Mojo MAX search needs distributed sharding, this project is not it; if they need named-tensor algebra in C++, the Mojo MAX equivalent does not exist.

**Branch / PR title**: `feature/glossary-mojo-namedmapping-disambiguation`
`docs(glossary): disambiguate named-axis from Mojo MAX NamedMapping`

**Dependency**: prefer landing after T-4 (so the new entries land in one batch and this one comes on top). Standalone-OK.

## T-6 — Tutorial 06 prerequisites preamble (F-7)

**Why P0**: every reader landing on tutorial 06 from a "WebGPU C++" search should immediately know whether they need autograd background first. Today they don't.

**Scope**: one new markdown cell at the top of [`tutorials/06_webgpu-acceleration.ipynb`](../../tutorials/06_webgpu-acceleration.ipynb):

> **Prerequisites.** Notebook `00_intro.ipynb` (named-axis fundamentals). No autograd knowledge required — this notebook covers forward execution on GPU only; the autograd backward pass is in `05_autograd-from-scratch.ipynb` and runs independently of the kernel backend.

**Branch / PR title**: `feature/tutorial-06-prerequisites-preamble`
`docs(tutorials): add prerequisites preamble to tutorial 06`

**Dependency**: none.

## T-7 — Learning-arc dependency map in `tutorials/README.md` (F-8)

**Why P0**: the corpus is numbered 00 / 05 / 06 / 07 / 08 (deliberately — see [PR #63](https://github.com/uyuutosa/tensor/pull/63)). A reader who doesn't know the deliberate gaps will fall through. A one-paragraph map fixes it.

**Scope**: a new section in [`tutorials/README.md`](../../tutorials/README.md) titled "Suggested reading order" with the dependency graph:

```
00 (named-axis fundamentals)
 ├─► 05 (autograd from scratch)
 │     └─► 07 (MLP on toy data, requires 00 + 05)
 ├─► 06 (WebGPU acceleration — design walkthrough, parallel to 05)
 └─► 08 (Hexagonal payoff — requires 00 + a peek at docs/arc42/05-building-blocks/)
```

**Branch / PR title**: `feature/tutorials-readme-learning-arc-map`
`docs(tutorials): add suggested reading order + dependency map`

**Dependency**: none.

## T-8 — ADR-0017 reproducibility envelope clarification (F-4)

**Why P1**: ADR-0015 §Compliance says "clean clone → build + bench + notebook in under 30 minutes." Notebook execution alone takes 15–40 minutes depending on xeus-cpp availability, so the literal claim is unrealistic. Since [ADR-0015's body is immutable](../arc42/09-decisions/0015-aspire-to-canonical-reference-quality-not-self-anoint.md) per the kit's MADR rule, the clarification must live in a successor ADR.

**Scope**: new MADR ADR using [`docs/templates/5_adr.md`](../templates/5_adr.md) — refines (not supersedes) ADR-0015 §Compliance. The Y-statement:

> In the context of ADR-0015's three-discipline aspiration, facing the practical reality that xeus-cpp install variability makes notebook execution take 15–40 minutes, we decided to **split the reproducibility envelope into two budgets — (a) 30-minute build + test + bench, (b) separate audit step for notebook execution** — to achieve a credible discipline that an external auditor can actually verify, accepting that the original "build + bench + notebook in under 30 minutes" wording overpromised.

**Filename**: `docs/arc42/09-decisions/0017-clarify-reproducibility-envelope.md`.

**Branch / PR title**: `feature/adr-0017-reproducibility-envelope`
`docs(adr): ADR-0017 — clarify reproducibility envelope (build + test + bench vs notebook)`

**Dependency**: should reference T-1..T-7 PRs as evidence the disciplines are otherwise being applied; non-blocking.

## T-9 — New `tutorials/01_formula-is-the-program.ipynb` (F-6)

**Why P1**: "the formula is the program" (ADR-0005) is the project's headline differentiator. Today it lives in section 4 of `00_intro.ipynb` — strategically buried. A dedicated tutorial in the `01` slot (currently empty per PR #63's "explicitly out of scope" annotation) would:

1. Front-and-centre the differentiator;
2. Fill the most natural "01" slot without breaking PR #63's decision (this is a *new* notebook addressing a different topic, not a scaffolded "01" the rehearsal report dropped);
3. Give external readers the demo they expect from the README's `R"(c_{ij} = a_i b_j)"_tex` example.

**Scope**: medium-effort. Walks `_tex` UDL → AST → `Evaluator` end-to-end. Cells include:

1. Motivation — the project's slogan, why a TeX surface (link ADR-0005).
2. The `_tex` UDL with a simple example: `R"(c = a + b)"_tex`.
3. AST visit — what the parser produces.
4. Binding tensors to AST leaves via `Evaluator<float>`.
5. Outer-product example: `R"(c_{ij} = a_i b_j)"_tex` reproducing the README's headline.
6. Inner-product example: `R"(\sum_i {c_i d_i})"_tex` — the Einstein-sum case.
7. Round-trip property — `parse(to_latex(e)) == e`.
8. LyX export module pointer.

Out of scope for this notebook: autograd over `_tex`-bound tensors (already in 05), GPU dispatch (in 06), training loops (in 07).

**Filename**: `tutorials/01_formula-is-the-program.ipynb`.

**Branch / PR title**: `feature/tutorial-01-formula-is-the-program`
`docs(tutorials): add 01_formula-is-the-program — _tex UDL end-to-end`

**Dependency**: should land after T-7 so the new tutorial slots into the updated learning-arc map.

**Update needed in this PR**: `tutorials/README.md` notebook corpus table gains a row; `book/_toc.yml` "Getting started" caption gains the new notebook below `00_intro.ipynb`.

---

## Execution order (recommended)

The P0 tasks land in a tight sequence; the P1 tasks land in parallel afterwards or before the cut depending on maintainer bandwidth.

```
T-1 + T-3      (glossary stale entries — bundled)            PR #X
   └─► T-4    (glossary missing entries — separate PR)        PR #X+1
         └─► T-5  (Mojo NamedMapping disambig)                PR #X+2
T-2            (vcpkg.json description)                       PR #X+3 (parallel to glossary work)
T-6            (tutorial 06 preamble)                          PR #X+4
T-7            (tutorials/README dep map)                      PR #X+5
   ╪═══════════ recommended cut-off line for 0.1.0 ═══════════╪
T-8            (ADR-0017 reproducibility envelope)             PR #X+6 (P1)
T-9            (new tutorial 01_formula-is-the-program)        PR #X+7 (P1)
```

## Out of scope

- The maintainer's `0.1.0` release ceremony (per [release rehearsal #48 §3](../reports/2026-05-11_phase-4-release-rehearsal.md)).
- C4 SVG render generation (kit doc rule §5 violation; requires Structurizr CLI — separate follow-up).
- Tutorial 06 live-execution upgrade (post-`0.1.0` per memory entry; GPU runner required).
- Bibliography audit half-yearly cycle (next due 2026-11-11 per ADR-0015 §Compliance).
- Phase 5 / Phase 6 (post-`0.1.0` per roadmap).

## Cross-references

- Source findings: [`docs/reports/2026-05-12_landscape-recheck-and-adversarial-review.md`](../reports/2026-05-12_landscape-recheck-and-adversarial-review.md)
- Discipline being audited: [ADR-0015](../arc42/09-decisions/0015-aspire-to-canonical-reference-quality-not-self-anoint.md)
- Glossary surface: [`docs/arc42/12-glossary/overview.md`](../arc42/12-glossary/overview.md)
- Tutorial corpus: [`tutorials/README.md`](../../tutorials/README.md)
- Release rehearsal: [`docs/reports/2026-05-11_phase-4-release-rehearsal.md`](../reports/2026-05-11_phase-4-release-rehearsal.md)
