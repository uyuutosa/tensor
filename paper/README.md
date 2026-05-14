# `paper/` — workshop paper draft

This directory contains the LaTeX source for the project's workshop paper draft, the Phase 7a M3 deliverable. See [`../docs/impl-plans/2026-05-14_phase-7a-docs-as-product.md`](../docs/impl-plans/2026-05-14_phase-7a-docs-as-product.md) for the surrounding plan and [`../docs/arc42/09-decisions/0021-strategic-narrowing-to-specialist-reference-positioning.md`](../docs/arc42/09-decisions/0021-strategic-narrowing-to-specialist-reference-positioning.md) for the strategic context.

## Files

| File                        | Purpose                                                                  |
| --------------------------- | ------------------------------------------------------------------------ |
| `neurips-workshop-2026.tex` | Main paper source (4 pages content + references; `article` baseline).    |
| `refs.bib`                  | BibTeX entries — external literature + project-internal ADR / report refs. |
| `Makefile`                  | `make paper` → `neurips-workshop-2026.pdf` (pdflatex + bibtex × 3).      |

## Build

```bash
make paper          # one-shot build
make watch          # latexmk continuous build (requires latexmk)
make clean          # remove intermediates, keep PDF
make distclean      # remove intermediates AND PDF
```

Requires TeX Live ≥ 2023. CI build verification is added separately under `.github/workflows/paper-build.yml` (Phase 7a M3 follow-up).

## Status

**A-stage draft (2026-05-14)**, per the [Phase 7a M3 A→B staging plan](../docs/impl-plans/2026-05-14_phase-7a-docs-as-product.md). The structural skeleton, design sections, and related-work positioning are written. **Empirical evaluation in §6 carries `\todo{}` markers** until the Phase 7a M2 micro-benchmark harness against einops on NumPy and Haliax on JAX lands.

The draft is usable now as:

- A **citable elevator pitch** for the project's distinctive design.
- A **synthesised vision document** for new contributors orienting on what `tensor` is.
- A **submission seed** for a target venue (workshop or arXiv) once the empirical pass concludes.

## Target venue (TBD)

Plausible targets, in approximate order of A-draft fit:

1. **arXiv preprint** — no review pressure; usable today.
2. **NeurIPS / ICLR workshop track** on ML systems or programming-languages-for-ML — fits the 4-page short-paper format; needs the empirical pass first.
3. **POPL / OOPSLA short paper** — would lean harder on the `_tex` UDL and NTTP type-system contribution; longer write-up + heavier formalisation required.

The venue selection is recorded as part of Phase 7a M3's `docs/reports/2026-MM-DD_phase-7a-paper-submission-target-selection.md` (deferred until empirical pass concludes).

## Template

The current draft uses generic `\documentclass{article}` with sensible packages so it compiles anywhere. Before submission to a specific venue, swap in that venue's official style file (`neurips_2026.sty`, `iclr2026_conference.sty`, etc.) and re-flow the content.

## Cross-references

- Phase 7a impl-plan: [`../docs/impl-plans/2026-05-14_phase-7a-docs-as-product.md`](../docs/impl-plans/2026-05-14_phase-7a-docs-as-product.md)
- Strategic narrowing ADR: [`../docs/arc42/09-decisions/0021-strategic-narrowing-to-specialist-reference-positioning.md`](../docs/arc42/09-decisions/0021-strategic-narrowing-to-specialist-reference-positioning.md)
- M1 landscape comparison study (related-work source): [`../docs/reports/2026-05-14_named-tensor-library-comparison.md`](../docs/reports/2026-05-14_named-tensor-library-comparison.md)
- Roadmap context: [`../docs/reports/2026-05-14_landscape-recheck-and-phase-7-roadmap.md`](../docs/reports/2026-05-14_landscape-recheck-and-phase-7-roadmap.md)
