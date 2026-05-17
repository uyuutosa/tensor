---
status: Active
owner: tensor
date: 2026-05-14
type: Layer B implementation plan
---

# Phase 7a — Documentation-as-Product (2026-05-14)

> Builds on [ADR-0021](../arc42/09-decisions/0021-strategic-narrowing-to-specialist-reference-positioning.md) (strategic narrowing) + [ADR-0015](../arc42/09-decisions/0015-aspire-to-canonical-reference-quality-not-self-anoint.md) (canonical-reference quality aspiration). Operationalises the moats identified in [`../reports/2026-05-14_landscape-recheck-and-phase-7-roadmap.md`](../reports/2026-05-14_landscape-recheck-and-phase-7-roadmap.md) §3a. Target exit: `0.5.0` tag (after Phase 6.6's `0.4.0`).

| Metadata     | Value                                                                            |
| ------------ | -------------------------------------------------------------------------------- |
| Layer        | B (volatile working material — dated, append-only)                              |
| Predecessor  | [`2026-05-14_phase-6-6-r-p6-5-5-lift.md`](./2026-05-14_phase-6-6-r-p6-5-5-lift.md) (Phase 6.6 R-P6.5.5 lift, parallel-able with M1/M2) |
| Anchor ADRs  | [ADR-0021](../arc42/09-decisions/0021-strategic-narrowing-to-specialist-reference-positioning.md), [ADR-0015](../arc42/09-decisions/0015-aspire-to-canonical-reference-quality-not-self-anoint.md) |
| Target window | 2026-05-14 → 2026-06-30 (~6 weeks; four milestones, ~one PR each)               |
| Exit         | `0.5.0` tag — citable PDF artifact + NeurIPS workshop paper draft + landscape comparison study, all under `docs/` |

---

## Strategic context

Phase 7a operationalises ADR-0021's strategic narrowing. The 2026-05-14 landscape recheck identified four uncontested moats:

1. **C++20 hybrid named-axis API** (NTTP + DynamicShape) — no competitor pairs compile-time and runtime checking.
2. **`_tex` UDL** — "the formula is the program" via LaTeX-subset parsing at compile time.
3. **Hexagonal-lite + KernelBackend port as teaching surface** — three swappable adapters as a worked architectural example.
4. **Canonical-reference-quality docs discipline** — arc42 §1–12, 19 ADRs (20 with ADR-0020, 21 with ADR-0021), 8 detailed-design instances, full Diátaxis, ~125 markdown files.

Phase 7a productises the **fourth** moat by packaging the existing documentation system into citable artifacts. The first three moats remain unchanged; this phase makes them *discoverable* to future implementers, textbook authors, and university courses.

---

## Milestones

The order is dependency-driven; each milestone exits when its deliverable is reviewable.

### P7a.M1 — Landscape comparison study

**Goal**: produce a citable comparison study placing `tensor` in matrix against the named-axis / educational-ML / DSL landscape as of 2026-05-14. Anchors the strategic-narrowing claim of ADR-0021 with concrete library-by-library comparison evidence.

**Deliverables**:

- `docs/reports/2026-05-14_named-tensor-library-comparison.md` (new) — 9-library × ~11-axis matrix covering: `tensor`, xarray, PyTorch named tensors, einops, einx, Haliax, Penzai, Nx, tinygrad. Plus narrative sections "What `tensor` does that nobody else does" / "What others do that `tensor` doesn't (and why we decline)" / cross-references to each library's authoritative documentation.

**Exit criteria**: the comparison study is reviewable; every cited library has a working link to its documentation as of the report date; the "What `tensor` does that nobody else does" claims each map back to a `tensor` ADR or detailed-design.

**Effort**: M (~1 day; M1 ships in the same PR as this impl-plan).

### P7a.M2 — Diátaxis-to-PDF packaging

**Goal**: assemble a single PDF artifact from the existing arc42 + detailed-design + Diátaxis user-manual + ADRs, with a working table of contents + cross-reference resolution + citation discipline aligned to ADR-0015 G-8.

**Deliverables**:

- `book/_config.yml` — Jupyter Book configuration extended to enable LaTeX PDF export (`builder: pdflatex`). Existing HTML build path preserved.
- `book/_toc.yml` — verify the existing TOC produces a coherent PDF chapter sequence (rearrange if not).
- `book/pdf-cover.tex` (new) — cover page with title, authors (initially `uyuutosa`), date, repository URL, version.
- `.github/workflows/deploy-book.yml` — extends to produce PDF output as a release artifact + uploads to GitHub Releases on tag push.
- `docs/user-manual/explanation/pdf-export.md` (new) — explanation document for the PDF artifact: what's inside, how to cite it, version pinning.

**Exit criteria**: a green CI run produces a downloadable `tensor-named-axis-0.4.x.pdf` artifact attached to the next dev-build release; cross-references resolve within the PDF; citations are bibliography-grade.

**Effort**: L (~3 days).

### P7a.M3 — NeurIPS workshop paper draft

**Goal**: produce a 4-page (workshop short paper standard) draft paper presenting `tensor` to the NeurIPS / ICLR ML systems / programming-languages workshop community. The paper is the citable elevator pitch; the PDF artifact (M2) is the supporting reference.

**Deliverables**:

- `paper/neurips-workshop-2026.tex` (new) — 4-page workshop paper:
  - §1 Introduction — the named-axis tensor algebra problem; landscape positioning.
  - §2 The `_tex` UDL — "the formula is the program" with a worked example.
  - §3 Hybrid named-axis API — NTTP compile-time checks + runtime DynamicShape.
  - §4 KernelBackend port as architectural teaching surface — three swappable adapters.
  - §5 Related work — pointing to the comparison study (M1) for detail.
  - §6 Open questions + future work — Phase 7c `_tex` parser expansion, Phase 8 LLM-tutorials.
- `paper/refs.bib` (new) — BibTeX entries for Haliax / Penzai / einx / xarray / PyTorch named tensors / Named Tensor Notation (Chiang & Rush 2021) / tinygrad.
- `paper/README.md` (new) — build instructions (`make paper` → `paper/neurips-workshop-2026.pdf`).
- `docs/reports/2026-MM-DD_phase-7a-paper-submission-target-selection.md` (new) — Layer-B note recording the chosen target venue + submission timeline + rejection-handling alternative venues.

**Exit criteria**: the paper compiles cleanly via `pdflatex` + `bibtex` + `pdflatex` × 2; arXiv-compatible source ships in `paper/`; the target venue is recorded.

**Effort**: L (~1 week, mostly writing).

### P7a.M4 — `0.5.0` release ceremony

**Goal**: ship Phase 7a to `main` as `0.5.0`. PDF artifact attached to the GitHub release; paper draft pushed to arXiv (if maintainer's arXiv endorsement is in place) or to OpenReview (if the target venue uses OR).

**Deliverables**:

- `release/0.5.0` branch from `develop`: version bumps + CHANGELOG `[0.5.0]` section + roadmap update (Phase 7a → Shipped).
- `tools/release.sh` — no change (the PDF + paper artifacts are static; they ship from the source tree).
- GitHub Release page: PDF artifact attached; paper preprint URL noted; comparison study (M1) link in the release notes.
- `README.md` — add a "Cite this work" section with the BibTeX entry pointing at the M3 paper + the M2 PDF artifact.
- HuggingFace Space `requirements.txt` — no change (Phase 7a is documentation-only; no Python SDK surface change).

**Exit**: `0.5.0` tag on `main`; PDF artifact + paper preprint URL both live; "Cite this work" section in README points at both.

**Effort**: M (~1 day).

---

## Out of scope

- **Full textbook draft**. M3's workshop paper is a 4-page artifact; a full textbook is a Phase 7+ follow-up if the workshop paper generates demand.
- **Translations**. The PDF + paper ship in English per [`docs/STRATEGY.md`](../../STRATEGY.md) defaults + the pentaglyph documentation rule §6. Translations are a follow-up.
- **Print-on-demand publication**. The PDF is downloadable from GitHub Releases; commercial print routing (Amazon KDP / Lulu / etc.) is a follow-up if demand materialises.
- **Video tutorials**. Outside Phase 7a's text-focused scope.

---

## CI changes

- Adds PDF build step to `deploy-book.yml`. Adds ~5 min to deploy time on a slot that already takes ~10 min.
- New `paper-build.yml` workflow on `paper/*.tex` change — runs `pdflatex` + `bibtex` to verify the workshop paper still compiles.

---

## Risks

- **R-P7a.1 — LaTeX toolchain on CI is fragile**. Jupyter Book's PDF export pulls a full TeX Live install (~5 GB). Mitigation: use the official `texlive/texlive:latest` container in CI; pin the version; skip PDF build on PRs (run only on `workflow_dispatch` + tag push).
- **R-P7a.2 — Workshop paper review acceptance is uncertain**. NeurIPS / ICLR workshop acceptance rates are ~40-60%. Mitigation: the paper draft is valuable regardless (it's the citable elevator pitch); arXiv preprint route is the fallback if the target venue rejects.
- **R-P7a.3 — `tensor` evolves between PDF cut and reader citation**. A reader citing the `0.5.0` PDF may find `0.6.0` has different surface. Mitigation: PDF artifact has the version in its filename + on the cover page; documentation includes a "version pinning" how-to.
- **R-P7a.4 — Landscape changes invalidate the comparison study before publication**. Haliax/Penzai/einx may ship breaking changes between the M1 study cut and M4 release. Mitigation: the comparison study has a "last reviewed" date; M4 ceremony includes a re-check pass; major shifts trigger a study refresh PR.

---

## Sequencing relative to other work

| Phase | Status (2026-05-14) | Relation to Phase 7a |
| ----- | -------------------- | --------------------- |
| Phase 6.5 | Implementation closed; awaiting `0.3.0` ceremony | predecessor (Phase 7a depends on a stable Python SDK for cite-ability) |
| Phase 6.6 (R-P6.5.5 lift) | Planning landed (ADR-0020 + impl-plan), awaiting `0.3.0` to start implementation | parallel-able to Phase 7a M1/M2 (different surfaces; no file overlap) |
| Phase 7a (this plan) | **Active** | runs alongside Phase 6.6 |
| Phase 7c (`_tex` parser expansion) | proposed | sequential to Phase 6.6 + Phase 7a (shares the kernel-module Python surface from 6.6 + benefits from the M3 paper as advance-citation) |

Phase 7a M1 (comparison study) ships in the same PR as this impl-plan + ADR-0021. M2-M4 ship in subsequent PRs targeting develop. The `0.5.0` release ceremony at M4 follows the `0.4.0` Phase 6.6 ceremony (so the ship order matches the strategic order — Phase 6.6 closes tech debt, Phase 7a productises the docs).

---

## Cross-references

- Anchor ADRs: [ADR-0021](../arc42/09-decisions/0021-strategic-narrowing-to-specialist-reference-positioning.md), [ADR-0015](../arc42/09-decisions/0015-aspire-to-canonical-reference-quality-not-self-anoint.md).
- Strategic context: [`../reports/2026-05-14_landscape-recheck-and-phase-7-roadmap.md`](../reports/2026-05-14_landscape-recheck-and-phase-7-roadmap.md) §3a + §4.
- M1 deliverable (shipped in the same PR): [`../reports/2026-05-14_named-tensor-library-comparison.md`](../reports/2026-05-14_named-tensor-library-comparison.md).
- Documentation system reference: [`docs/AI_INSTRUCTIONS.md`](../AI_INSTRUCTIONS.md), [`docs/WORKFLOW.md`](../WORKFLOW.md).
- Parallel work: [`2026-05-14_phase-6-6-r-p6-5-5-lift.md`](./2026-05-14_phase-6-6-r-p6-5-5-lift.md) — Phase 6.6 (R-P6.5.5 lift).
