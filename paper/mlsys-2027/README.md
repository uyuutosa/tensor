# MLSys 2027 — Full Conference Paper Track

This directory holds the **MLSys 2027 main-track submission** for the
`tensor` named-axis library project. It is a separate authoring source from
the NeurIPS Workshop 2026 draft in [`../neurips-workshop-2026.tex`](../neurips-workshop-2026.tex)
(see PR #135) — the two papers share bibliography entries and core
content but diverge in length, framing, and evaluation depth.

## Target venue

- **Conference**: MLSys 2027
  ([Conference on Machine Learning and Systems](https://mlsys.org/))
- **Page limit**: 10 pages (excluding references and appendix; subject to
  CFP revision)
- **Submission deadline**: ~October 2026 (provisional — confirm against
  the 2027 CFP when released)
- **Camera-ready style**: `mlsys2027.sty` (not yet released; using
  `article` as placeholder until then)

## Authoring strategy

We follow a **twin-source** model for the paper portfolio:

| Source | Length | Audience | Status |
| ------ | ------ | -------- | ------ |
| `paper/neurips-workshop-2026.tex` | 8 pages | Early reviewer feedback, workshop community | Drafted (PR #135), §7 awaiting B-stage empirical numbers |
| `paper/mlsys-2027/main.tex` (here) | 10 pages | MLSys main-track reviewers | Skeleton — section outlines + framing prose, body TBD |
| `paper/preprint/main.tex` (future) | unbounded | arXiv readers, deep-dive | Will fork from `mlsys-2027` after acceptance/rejection signal |

**Why separate sources rather than `\if\submissionmode` macros**: the
three artefacts target different page budgets and reader priors. Workshop
+ MLSys conference paper + arXiv preprint have genuinely different
structures (preprint can carry full motivation deep-dive, BA case study
with multiple dataset variants, exhaustive related work; conference paper
must compress all of that). Conditional macros that try to share a single
source against three divergent structures degrade into unreadable
`\ifthenelse{...}` thickets.

What stays unified: `refs.bib` (currently duplicated; will collapse to
`paper/refs.bib` shared after PR #135 lands), math/symbol macros
(`\NamedTensor`, `\axis{}`, etc.; planned `paper/common/symbols.tex`).

## A → B staging (inherited from workshop draft)

Per the Phase 7a M3 plan, the paper acknowledges and explicitly schedules
the empirical-evaluation gap:

- **A-stage** (now): write the design + implementation sections + outline
  Case Study with `[TODO: empirical numbers]` markers. Gives a complete
  story you can read end-to-end without hand-waving the architecture.
- **B-stage** (~September 2026): land the micro-benchmark harness
  `bench/bench_mlsys_paper_case_study.py` (`tensor::contract` vs
  `einops.rearrange` vs `haliax.NamedArray` on a representative workload,
  three-backend perf comparison: reference / Eigen / WebGPU), produce
  reproducible numbers on a public substrate, slot them into §5.

## Build

```bash
cd paper/mlsys-2027/
make            # → main.pdf
make clean      # remove intermediates
make watch      # latexmk -pvc continuous rebuild
```

Tested on TeX Live 2022. CI build is planned as a follow-up workflow
(`.github/workflows/paper-build.yml`) once both tracks (workshop + MLSys)
have stable skeletons; for now it is a local-only build.

## Cross-references

- Workshop paper: PR #135 — [`../neurips-workshop-2026.tex`](../neurips-workshop-2026.tex)
- Phase 7a planning + M1 comparison study: PR #134 (`feature/phase-6-6-r-p6-5-5-lift-planning`)
- ADRs referenced in the paper (resolved via `refs.bib` `@misc` entries):
  - ADR-0014 — Dawn-via-vcpkg substrate for the WebGPU kernel backend
  - ADR-0021 — Hexagonal-lite kernel backend port boundary
  - ADR-0025 — `_tex` UDL adoption for shape literals
- Diátaxis PDF (Phase 7a M2): PR #137 — generated via Sphinx LaTeX
  builder, lives at `docs/_build/_static/tensor-named-axis.pdf`. The
  Diátaxis PDF is the *teaching/reference* artefact (book-length); the
  MLSys paper is the *research contribution* artefact (10p). They share
  the named-axis story but serve different audiences.
