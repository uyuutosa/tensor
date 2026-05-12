---
status: Active
owner: tensor
date: 2026-05-12
type: Layer B research / evaluation report
---

# Landscape re-check + adversarial self-review (2026-05-12)

> A pre-`0.1.0` self-audit conducted under [ADR-0015](../arc42/09-decisions/0015-aspire-to-canonical-reference-quality-not-self-anoint.md) §Compliance. After 77 merged PRs in seven days the project's internal documentation is mutually consistent; this report asks the two questions that internal consistency does not answer: **(A) has the external world moved in a way that invalidates our positioning?** and **(B) where would an adversarial outside reader catch us making promises we cannot keep?**

| Metadata     | Value                                                                                   |
| ------------ | --------------------------------------------------------------------------------------- |
| Layer        | B (volatile working material — dated, append-only)                                      |
| Inputs       | `WebSearch` digest (Investigation A); read-only file audit (Investigation B)            |
| Outputs      | Findings table (this file §4) → impl-plan (`docs/impl-plans/2026-05-12_post-investigation-tasks.md`) → executed in PR sequence |
| Predecessors | [`2026-05-10_tensor-revival-landscape.md`](./2026-05-10_tensor-revival-landscape.md); [`2026-05-11_external-substrate-research.md`](./2026-05-11_external-substrate-research.md) |
| Prompted by  | Maintainer 2026-05-12 request: "一度このソフトウェアの方向性などを改めて調査を通して検討したら良いと思う" |

---

## 1. Context

By 2026-05-12 the project has shipped Phase 1 + 1.5 + 2 + 2.5 + 3 (functionally complete on RTX 3090; 12 of 15 `KernelBackend` methods dispatch real Dawn compute on `float`) and audited every active arc42 / book / design-guide / CONTRIBUTING / detailed-design surface to current state (PRs #71–#77). The release-readiness audit ([`2026-05-11_phase-4-release-rehearsal.md`](./2026-05-11_phase-4-release-rehearsal.md)) is GREEN; only the maintainer's release-ceremony steps remain.

The project's positioning ([ADR-0015](../arc42/09-decisions/0015-aspire-to-canonical-reference-quality-not-self-anoint.md), superseding [ADR-0013](../arc42/09-decisions/0013-reframe-as-canonical-reference-for-named-tensor-computation.md)) is *aspirational canonical-reference quality* — a label earned by external adoption, not by self-declaration. The three self-imposed disciplines (bibliography / ubiquitous-language / reproducibility) are forcing functions, not claims of recognition. This report is the first independent check that those forcing functions are still pointing at the right thing.

The two questions:

- **A — Has the niche shifted?** ADR-0015's aspiration only makes sense if "the canonical reference for differentiable named-axis tensor computation in modern C++" remains a *defensible position* in the May 2026 landscape. If Mojo or PyTorch or a new C++ entrant has claimed that ground in the eight months since [`2026-05-10_tensor-revival-landscape.md`](./2026-05-10_tensor-revival-landscape.md), the positioning needs to evolve before the public 0.1.0 cut.
- **B — Are the forcing functions actually firing?** Self-imposed discipline is credible only insofar as it can be checked. Bibliography discipline requires every ADR to cite primary sources; ubiquitous-language requires every public symbol to have a glossary entry; reproducibility requires the README Quickstart to actually deliver in 30 minutes. An adversarial reader gets to call BS on each of these.

## 2. Investigation A — external landscape delta

Method: eight targeted `WebSearch` queries across the substrate stack the project committed to in ADR-0014 + ADR-0016, plus the adjacent competitor space.

### A.1 `std::linalg` (P1673) — vendor STL ship status

[P1673R13](https://isocpp.org/files/papers/P1673R13.html) remains the working draft for `<linalg>`. **No vendor STL has tagged a `<linalg>` ship as of 2026-05.** [cppreference compiler-support](https://en.cppreference.com/w/cpp/compiler_support.html) shows libc++ / libstdc++ / MSVC STL all tracking C++26 features with no `<linalg>` entry; [microsoft/STL](https://github.com/microsoft/STL) tracks N5032 working draft. [kokkos/stdBLAS](https://github.com/kokkos/stdBLAS) is the only production-grade reference implementation outside the standard library tree.

**Project impact**: ADR-0014 §Decision Outcome point 4 — the `tensor::linalg` namespace shim over kokkos/stdBLAS with `__cpp_lib_linalg` feature-test — **remains the right pre-emption**. Phase 5 timing is unchanged (post-`0.1.0`).

### A.2 Dawn / WebGPU runtime

[WebGPU spec at W3C](https://www.w3.org/TR/webgpu/) is still **Candidate Recommendation Draft** (not REC). Major browsers ship: Chrome / Edge (April 2023), Safari 26 (June 2025), Firefox 141 (July 2025); Mozilla expects Linux + Android wgpu shipping during 2026. [Dawn](https://dawn.googlesource.com/dawn) and [wgpu-native](https://github.com/gfx-rs/wgpu-native) are converging on a shared `webgpu.h` C header. No spec breakage announced.

**Project impact**: ADR-0006 + ADR-0012 + ADR-0014 §1 + ADR-0016 chain holds. The local Dawn `20260410.140140` port the project built against (PRs #60 / #61 / #62) is on the stable side of the WebGPU lifecycle.

### A.3 xeus-cpp + xeus-cling

[compiler-research/xeus-cpp](https://github.com/compiler-research/xeus-cpp) (the project's notebook execution path since [PR #42](https://github.com/uyuutosa/tensor/pull/42)) is the active fork — merged xeus-clang-repl features, added code completion, **dropped xtl**, **added C++20 kernel**, and dropped C++11 / C++14 kernels after xeus 5. [jupyter-xeus/xeus-cling](https://github.com/jupyter-xeus/xeus-cling) has not cut a release; **de facto deprecation** with no formal end-of-life announcement.

**Project impact**: ADR-0014 §Decision Outcome point 3 (migrate notebook execution to xeus-cpp; keep xeus-cling as `legacy-xeus-cling` smoke for `00_intro.ipynb`) — **vindicated**. The migration looks well-timed.

### A.4 Mojo / MAX — the **vocabulary clash** finding

[Modular 26.3 (April 2026)](https://www.modular.com/blog/modular-26-3-mojo-1-0-beta-max-video-gen-and-more) released **Mojo 1.0 beta**. MAX added `max.experimental.sharding` with `DeviceMesh`, `PlacementMapping`, **and `NamedMapping`** ([MAX docs](https://docs.modular.com/max/changelog/)). The community-maintained [Tenmo](https://forum.modular.com/t/tenmo-a-lean-tensor-nn-library-in-pure-mojo/2609) library provides pure-Mojo tensor + autograd + layers at ~1.3× PyTorch CPU on MNIST.

**Important disambiguation**: Mojo's `NamedMapping` names **mesh axes for sharding** (like JAX `PartitionSpec`), not **tensor semantic axes**. Mojo's core `Tensor` type stayed positional; [`LayoutTensor`](https://docs.modular.com/mojo/kernels/layout/layout_tensor/LayoutTensor/) is the GPU kernel primitive, also positional.

**Project impact**: **a vocabulary collision risk, not a positioning collision**. A reader searching "Mojo named tensor" in May 2026 will find Mojo MAX's mesh-sharding `NamedMapping`, which has nothing to do with this project's per-tensor axis labels. The disambiguation belongs in this project's documentation (a FAQ entry; a glossary cross-reference; possibly an arc42 §1 footnote) before the `0.1.0` cut, so the term doesn't get conflated when external readers arrive.

### A.5 PyTorch named tensors / JAX axis annotation

[PyTorch named tensors](https://docs.pytorch.org/docs/stable/named_tensor.html) remain **experimental** in 2.11 docs (June 2025). A January 2026 dev-discuss thread ["Reviving Named Tensors: Strategy for addressing the Experimental backlog"](https://dev-discuss.pytorch.org/t/reviving-named-tensors-strategy-for-addressing-the-experimental-backlog/3289) confirms **no promotion to stable and no removal**. [JAX issue #5048 (Named tensors)](https://github.com/jax-ml/jax/issues/5048) remains open; the ecosystem standard for axis annotation in JAX is [jaxtyping](https://github.com/patrick-kidger/jaxtyping) (string-shape type annotations, runtime-checked), **not first-class named axes**.

**Project impact**: **nobody in Python ML has a stable, first-class named-axis tensor as of 2026-05**. The project's per-symbol named-axis API is genuinely uncontested in production Python ML; this strengthens, not weakens, the canonical-reference framing.

### A.6 tinygrad

[tinygrad 0.12.0](https://pypi.org/project/tinygrad/) released 2026-01-12; still bounded by the < 1000 LOC self-imposed limit. Educational angle is implicit (small, hackable); no curriculum or doc-quality push announced. Positional axes only.

**Project impact**: tinygrad remains the philosophical sibling for the "small, readable, hackable" stance, but **does not compete on the canonical-reference axis** (no documentation discipline, no glossary, no ADR sequence). The "tinygrad-class teaching ML framework grounded in named-axis semantics" framing in ADR-0007 stays valid.

### A.7 New C++ tensor entrants

No new entrant since 2026-01 claims "canonical-reference" or "educational" framing. [Tenseur](https://github.com/istmarc/tenseur) (C++20 header-only, lazy eval, BLAS + SIMD) is the closest newcomer but **performance-focused, not pedagogy-focused, no autograd, no named axes**. [Fastor](https://github.com/romeric/Fastor), [bassoy/ttv](https://github.com/bassoy/ttv), acrotensor, PHYSLIB, TenDeC++, robclu/tensor are all positional. [ATen / libtorch](https://github.com/pytorch/pytorch/tree/main/aten) is the only differentiable C++ tensor with broad reach and is decidedly not header-only or educational.

**Project impact**: the C++ niche is unchanged. The four-prong wedge **(named-axis + differentiable + header-only C++20 + canonical-reference framing)** remains uncontested.

### A.8 Vocabulary contest — "named-axis differentiable header-only C++ tensor"

No competitor in the C++ niche claimed this framing in 2025-10..2026-05. [xtensor](https://github.com/xtensor-stack/xtensor) is header-only but positional and non-differentiable. Eigen's `Tensor` (unsupported module) is header-only, positional, non-differentiable. Kokkos `View` has axis labels but is positional and non-differentiable. SYCL has no tensor type.

**Project impact**: the framing is unclaimed; the **only conflict is vocabulary** (the "named" word — addressed in §A.4).

### A.9 Section A summary

The eight-month delta **does not invalidate the project's positioning**. The four-prong wedge holds; the substrate decisions (Dawn / xeus-cpp / kokkos+stdBLAS shim path) all came out of this audit reinforced rather than weakened. The one new thing to address is the **Mojo MAX `NamedMapping` vocabulary clash** — it's a documentation hygiene matter, not a strategic one.

## 3. Investigation B — adversarial self-review

Method: five checks against the three ADR-0015 disciplines + the educational pitch + the cross-reference graph. Each finding has severity (🔴 High / 🟡 Medium / 🟢 Low) and a recommended remediation.

### B.1 — Bibliography discipline (✅ pass with 1 caveat)

All 16 ADRs (0001–0016) cite external sources: papers, standard-library specs, GitHub repositories, established precedents (tinygrad, micrograd, CLRS, etc.). No ADR is self-referential only.

**One drift caveat found**:

- [🟡 Medium] **`vcpkg.json:5` still references ADR-0013 with the declarative wording** (`"the canonical reference for differentiable named-axis tensor computation in modern C++ (ADR-0013)."`). ADR-0013 was superseded by ADR-0015 in PR #58 — wording should be aspirational and cite ADR-0015. **Remediation**: edit the `description` field. (One-line change, no semver concern.)

### B.2 — Ubiquitous-language coverage (⚠️ incomplete — 6 + 2 missing entries / stale)

Glossary at [`docs/arc42/12-glossary/overview.md`](../arc42/12-glossary/overview.md) defines 17 terms but `include/tensor/core/` exposes 27+ public symbols.

**Missing or thin entries**:

- [🟡 Medium] **`FixedString<N>` has no standalone entry** (mentioned inline in the `Axis` entry, line 30). It is the NTTP storage type for compile-time axis labels per ADR-0004; a foundational type with public visibility deserves its own glossary page. **Remediation**: add entry between `Axis` and `Shape` defining `FixedString<N>` as the NTTP-friendly fixed-length string type backing the `_ax` UDL.
- [🟡 Medium] **`LabelTag<S>` has no standalone entry** (mentioned inline in `Axis`, line 33). It is the compile-time form of `Axis`; ADR-0004's hybrid named-axis API rests on it. **Remediation**: add entry parallel to `Axis` framing them as runtime / compile-time twins.
- [🟡 Medium] **`BroadcastPlan` is named in the `Broadcast` code list** (line 54) but has **no definition of what a plan is or how it's used**. **Remediation**: add a paragraph: "A metadata structure (index mappings, extent reductions, source-axis identifiers) computed by the Domain layer and passed to the `KernelBackend` for execution. Decouples the named-axis algebra from the byte-level dispatch."
- [🟡 Medium] **`ContractPlan` is named in the `Contraction` code list** (line 61) but has **no definition** (same gap as `BroadcastPlan`). **Remediation**: parallel paragraph.
- [🟢 Low] **`AxisLike` (concept) has no entry**; a public concept declared in `concepts.hpp`. **Remediation**: brief entry under the Architecture section describing it as the compile-time constraint on axis-template arguments.
- [🟢 Low] **`DynamicTensor<T>` is mentioned inline in `Tensor`** (line 47) but has no standalone entry; the runtime-rank twin would benefit from one — the [`user-manual/how-to/named-tensor-types.md`](../user-manual/how-to/named-tensor-types.md) decision guide is the closest thing. **Remediation**: optional. The inline coverage may be sufficient given the how-to exists.

**Stale entries** (not noted by the agent — caught on direct re-read):

- [🔴 High] **`KernelBackend` entry (line 140) is stale**: "Currently three concrete adapters: `reference`, `eigen`, `webgpu` (stub, dispatch wiring deferred)." The "stub, dispatch wiring deferred" parenthetical is wrong as of 2026-05-12 — PRs #60 / #61 / #62 shipped the dispatch wiring; 12 of 15 methods run real Dawn compute on RTX 3090. **Remediation**: replace the parenthetical with "12 of 15 methods dispatch real Dawn compute on `float` (the rest delegate to reference, matching the Eigen adapter's scope)".
- [🟡 Medium] **`Dawn` entry (line 161) is stale**: "the project's planned WebGPU build path is `find_package(dawn CONFIG REQUIRED)` after a vcpkg baseline bump (deferred to P3.M3.2 / P3.M4.2)." Phase 3 P3.M3.2 / P3.M4.2 / P3.M5 all shipped; the build path is no longer "planned". **Remediation**: rewrite the entry to describe the actual shipped build path (local `vcpkg install 'dawn[core,vulkan]'` for the maintainer's RTX 3090 verification; a vcpkg baseline bump remains a future option for users without local Dawn).

### B.3 — Reproducibility envelope (⚠️ feasible with caveats — 2 findings)

The README Quickstart is build + test only, which is what ADR-0015 §Compliance requires for the 30-minute envelope. The two paths to actually testing reproducibility:

| Step                                                  | Wall-clock estimate           |
| ----------------------------------------------------- | ----------------------------- |
| `git clone` + `vcpkg` bootstrap                       | 3–5 min                       |
| `cmake --preset=default` (config)                     | 2–3 min                       |
| `cmake --build --preset=default` (compile)            | 5–8 min (header-only, no link)|
| `ctest --preset=default`                              | 1–2 min                       |
| **Subtotal — build + test**                           | **11–18 min** ✅              |
| Add `-DTENSOR_BUILD_BENCH=ON` (optional, Quickstart §) | + 2–3 min                    |
| Add notebook tutorials (`conda env create` + jupyter) | + 15–40 min (xeus-cpp install variability) |

**Findings**:

- [🟡 Medium] **ADR-0015 §Compliance language "build + bench + notebook in under 30 minutes" is unrealistic if interpreted literally** — notebook execution alone can take 15–40 minutes depending on whether xeus-cpp is conda-available locally. The README Quickstart does not gate on notebook execution; the discipline statement does. **Remediation**: clarify ADR-0015 §Compliance language as a *new ADR* (per the immutability rule, we cannot edit ADR-0015's body) that says "clean clone → build + test + bench in under 30 minutes; notebook execution is a separate audit step." Alternatively, drop "notebook" from the 30-min envelope.
- [🟢 Low] **The default CMake preset does not build the bench** (`TENSOR_BUILD_BENCH=ON` is opt-in). The "build + bench" half of the envelope therefore needs an extra flag the new user must discover. **Remediation**: either keep the opt-in but document it in the discipline statement, or add a `bench` preset alongside `default` so the verifier can simply `cmake --preset=bench && cmake --build --preset=bench`. Lower-effort: documentation only.

### B.4 — Educational pitch coherence (⚠️ partial — 3 findings)

Five notebooks shipped (00, 05, 06, 07, 08). The numbering gap (01–04 explicitly dropped per [PR #63](https://github.com/uyuutosa/tensor/pull/63)) is a deliberate decision, not a bug — [`tutorials/README.md`](../../tutorials/README.md) documents the rationale. However:

- [🟡 Medium] **The `_tex` UDL (ADR-0005) is the project's headline differentiator ("the formula is the program") yet no notebook is dedicated to it.** The 2016 Qiita post content lives inside `00_intro.ipynb`; the `_tex` material is in section ~4 of `00_intro`. A reader skimming the tutorial titles sees autograd / WebGPU / MLP / backends — but no "the formula is the program" demo. **Remediation**: either rename `00_intro.ipynb` to make the `_tex` content prominent in section headings, or split it into a dedicated **`01_formula-is-the-program.ipynb`** that fits the (currently empty) `01` slot. The latter creates a learning arc with no gaps in slot numbering.
- [🟡 Medium] **Tutorial 06 (WebGPU acceleration — design walkthrough) does not state its prerequisites.** From context it does *not* require autograd knowledge from 05; it builds on forward kernels only. A reader landing on `06` from a "WebGPU C++" search will not know whether to read 05 first. **Remediation**: add a 2-line preamble to `06` stating "Prerequisites: notebook 00 (named-axis fundamentals). No autograd knowledge required — this notebook covers forward execution on GPU only."
- [🟢 Low] **There is no learning-arc dependency map.** [`tutorials/README.md`](../../tutorials/README.md) lists the corpus in numeric order but does not state "read 00 first, then 05 or 06 in either order, then 07 (requires 00 + 05), then 08 (requires 00 + a peek at `docs/arc42/05-building-blocks/`)." **Remediation**: add a dependency-map section to `tutorials/README.md`.

### B.5 — Cross-reference resolution (✅ pass)

Fifteen randomly sampled markdown links across arc42 §1, §4, and `kernel-backend-port.md` all resolve. No broken links detected.

### B.6 — Section B summary

**Bibliography**: 1 metadata-sync finding (vcpkg.json) — trivial.
**Ubiquitous language**: 6 missing-entry + 2 stale-entry findings — the highest-yield repair, since 8 of the project's 27+ public symbols touch the glossary either weakly or incorrectly.
**Reproducibility**: 1 envelope-language finding that needs a new ADR (immutability of ADR-0015 means we cannot edit its §Compliance text in place).
**Educational coherence**: 3 findings, all low-medium severity; the `_tex` notebook gap is the most strategically meaningful (headline feature, buried).
**Cross-references**: clean.

## 4. Synthesis — findings table

| ID    | Source check | Severity | Finding                                                                                  | Doc surface to touch                                            |
| ----- | ------------ | -------- | ---------------------------------------------------------------------------------------- | --------------------------------------------------------------- |
| F-1   | A.4          | 🟡 Med   | Mojo MAX `NamedMapping` vocabulary clash; project's "named-axis" is per-tensor, Mojo's is mesh-sharding | new glossary entry; possibly arc42 §1 footnote                  |
| F-2   | B.1          | 🟡 Med   | `vcpkg.json:5` description still cites ADR-0013 declarative wording                      | `vcpkg.json`                                                    |
| F-3a  | B.2          | 🔴 High  | Glossary `KernelBackend` entry says "stub, dispatch wiring deferred" — false since #60–#62 | `docs/arc42/12-glossary/overview.md`                            |
| F-3b  | B.2          | 🟡 Med   | Glossary `Dawn` entry says "planned ... deferred to P3.M3.2 / P3.M4.2" — Phase 3 shipped | `docs/arc42/12-glossary/overview.md`                            |
| F-3c  | B.2          | 🟡 Med   | Glossary missing standalone entries for `FixedString`, `LabelTag`, `BroadcastPlan`, `ContractPlan`, `AxisLike`, `DynamicTensor` | `docs/arc42/12-glossary/overview.md`                            |
| F-4   | B.3          | 🟡 Med   | ADR-0015 §Compliance "30 min build + bench + notebook" is unrealistic if notebook is counted | **new ADR** clarifying envelope (cannot edit ADR-0015 body)     |
| F-5   | B.3          | 🟢 Low   | Default CMake preset does not build bench; `TENSOR_BUILD_BENCH=ON` is opt-in              | `tutorials/README.md` or `CMakePresets.json` (decide)           |
| F-6   | B.4          | 🟡 Med   | `_tex` UDL has no dedicated notebook; headline feature buried in `00_intro` section 4    | new `tutorials/01_formula-is-the-program.ipynb`                 |
| F-7   | B.4          | 🟡 Med   | Tutorial 06 has no prerequisites preamble                                                | `tutorials/06_webgpu-acceleration.ipynb` (preamble cell)        |
| F-8   | B.4          | 🟢 Low   | No learning-arc dependency map in `tutorials/README.md`                                  | `tutorials/README.md`                                           |

## 5. Recommended actions (preview)

The detailed task breakdown lives in the impl-plan ([`docs/impl-plans/2026-05-12_post-investigation-tasks.md`](../impl-plans/2026-05-12_post-investigation-tasks.md)). At the strategic level:

- **Strategy unchanged**: the four-prong wedge holds; no ADR supersession needed for positioning.
- **One new ADR required** (F-4): clarify the reproducibility envelope language. This is the only finding that crosses an immutable boundary.
- **Six doc updates** (F-2, F-3a/b/c, F-7, F-8) — small, well-scoped, single-PR each.
- **One new tutorial** (F-6) — medium-effort; the dedicated `_tex` notebook is the highest-leverage *educational* finding.
- **One new glossary cross-reference** (F-1) — disambiguates the vocabulary clash with Mojo MAX.

Release-ceremony gating: none of the findings block `0.1.0`. F-3a (the 🔴 High finding) is the only one a hostile reader would flag pre-release, but it's a one-line glossary edit. Recommend executing F-2, F-3a, F-3b, F-3c, F-7, F-8, F-1 before `0.1.0` cut; F-4 (new ADR) and F-6 (new notebook) can land either before or after the cut without strategic risk.

## 6. References

- [ADR-0015](../arc42/09-decisions/0015-aspire-to-canonical-reference-quality-not-self-anoint.md) — the canonical-reference-quality aspiration framing this audit interrogates
- [`2026-05-10_tensor-revival-landscape.md`](./2026-05-10_tensor-revival-landscape.md) — the predecessor landscape report
- [`2026-05-11_external-substrate-research.md`](./2026-05-11_external-substrate-research.md) — the substrate research that produced ADR-0014
- [`2026-05-11_phase-4-release-rehearsal.md`](./2026-05-11_phase-4-release-rehearsal.md) — release-readiness audit (GREEN as of 2026-05-12)
- WebSearch sources cited inline in §2 (P1673R13, WebGPU CR, Dawn, xeus-cpp, Modular 26.3, PyTorch named-tensor docs, JAX issue #5048, tinygrad PyPI, Tenseur)
