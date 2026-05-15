---
status: Stable
owner: tensor
last-reviewed: 2026-05-15
---

# Explanation — the `tensor` PDF artifact

> Diátaxis quadrant: **Explanation** ([why](https://diataxis.fr/explanation/)). For the build steps see [`../how-to/run-notebooks-locally.md`](../how-to/run-notebooks-locally.md). For the strategic reasoning see [ADR-0021](../../arc42/09-decisions/0021-strategic-narrowing-to-specialist-reference-positioning.md).

The project's documentation corpus is also released as a single citable PDF on every tag. This page explains what the PDF *is*, what it is *for*, and how to *cite* it. It does not explain how to build the PDF — see [`book/README.md`](../../../book/README.md) for the build commands.

## Why a PDF at all?

The HTML site at <https://uyuutosa.github.io/tensor/> is the canonical reading surface. It is searchable, linkable, browsable, and continuously deployed from `develop`. So why also ship a PDF?

**Three reasons, all from [ADR-0021](../../arc42/09-decisions/0021-strategic-narrowing-to-specialist-reference-positioning.md)'s strategic narrowing:**

1. **Citation discipline.** A PDF artifact has a stable filename and a stable internal pagination. Future implementers porting the named-axis substrate to another language can cite "page 47 of `tensor-named-axis-0.5.0.pdf`" and that reference will resolve identically a decade later. A live HTML link can break, redirect, or silently update its content.
2. **Audit-time grounding.** When the half-yearly landscape audit per [ADR-0017](../../arc42/09-decisions/0017-clarify-reproducibility-envelope.md) recheck happens, the PDF artifact for the previous audit cycle is the ground truth. It records what was true at the time the strategic positioning was set, not what is true today.
3. **Offline-readable canonical reference.** A textbook author writing about named-axis tensor algebra can read the PDF on a flight without needing a network. The HTML site doesn't degrade gracefully to offline; the PDF does.

## What's inside the PDF

The PDF includes exactly what the HTML build at <https://uyuutosa.github.io/tensor/> includes, in the same order. As of Phase 7a M2:

- The full tutorial corpus (six C++ notebooks executed via xeus-cpp + six Python notebooks executed locally and committed with outputs).
- Selected `docs/reports/` Layer-B reports — backend performance comparisons, the 2026-05 landscape recheck, the named-tensor library comparison study.
- The full Diátaxis user manual (tutorials / how-to / reference / explanation).
- Reference architecture excerpts — arc42 §1 (Introduction and Goals), §5 (Building Blocks), §9 (ADR index), §12 (Glossary).
- Selected detailed-design documents — the KernelBackend port, the Python SDK binding surface.

The exact chapter order is captured in [`book/_toc.yml`](../../../book/_toc.yml). When that file changes, both the HTML site and the PDF artifact reflect the change on the next release.

## What's NOT in the PDF

- The 21 ADRs in their entirety (only the index is in the PDF). The individual ADR pages are in the HTML site under [arc42 §9](../../arc42/09-decisions/). The PDF would be 300+ pages if it embedded every ADR; the trade-off is "browsable HTML for the body, citation-grade PDF for the headline".
- The C4 model diagram source (`docs/diagrams/c4/workspace.dsl`). The rendered SVGs are included in the HTML and would in principle be in the PDF; in practice the LaTeX builder doesn't always honour SVG embeds, so we treat them as HTML-canonical.
- Repository-level metadata like `CHANGELOG.md`. CHANGELOG belongs in the repo, not the PDF.

If you need any of these, cite the HTML location.

## How to cite the PDF

A BibTeX-shaped citation form:

```bibtex
@misc{tensorPDF,
  author       = {uyuutosa},
  title        = {{tensor: A C++20 Named-Axis Differentiable Tensor Library
                   — Canonical Reference}},
  year         = {2026},
  howpublished = {Version 0.5.0 PDF artifact, attached to
                  \url{https://github.com/uyuutosa/tensor/releases/tag/v0.5.0}},
  note         = {Snapshot 2026-MM-DD; see \url{https://uyuutosa.github.io/tensor/}
                  for the continuously updated HTML edition.},
}
```

Substitute the exact version + tag URL + snapshot date for your citation. The version is part of the PDF filename and the cover page; the snapshot date matches the `last-reviewed` field in the docs front-matter at PDF cut time.

## Version pinning — which PDF to cite

Each release ships its own PDF, attached to the GitHub Release for that tag. A reader citing the project as it existed at `0.4.x` should cite the `0.4.x` PDF specifically — not the latest. The continuously-updated HTML site is for the current state; the dated PDF is for historical / archival reference.

## Reproducibility envelope

Per [ADR-0017](../../arc42/09-decisions/0017-clarify-reproducibility-envelope.md), the PDF artifact falls inside the **build-and-test** reproducibility envelope of the project. The Sphinx LaTeX toolchain + TeX Live + the project's `_config.yml` configuration are deterministic given the same source tree. The PDF for tag `v0.5.0` produced on 2026-MM-DD and the PDF for the same tag rebuilt three years later should be byte-for-byte identical, modulo Sphinx's compile-time timestamp.

## Cross-references

- Strategic context: [ADR-0021](../../arc42/09-decisions/0021-strategic-narrowing-to-specialist-reference-positioning.md) — strategic narrowing to specialist-reference positioning.
- Quality discipline: [ADR-0015](../../arc42/09-decisions/0015-aspire-to-canonical-reference-quality-not-self-anoint.md) — aspire to canonical-reference quality.
- Reproducibility envelope: [ADR-0017](../../arc42/09-decisions/0017-clarify-reproducibility-envelope.md).
- Phase 7a impl-plan: [`../../impl-plans/2026-05-14_phase-7a-docs-as-product.md`](../../impl-plans/2026-05-14_phase-7a-docs-as-product.md) M2.
- Build configuration: [`../../../book/_config.yml`](../../../book/_config.yml) `sphinx.config.latex_engine` + `latex.latex_documents.targetname`.
- CI workflow: [`.github/workflows/deploy-book.yml`](../../../.github/workflows/deploy-book.yml) — PDF artifact build + upload on tag push / workflow_dispatch.
