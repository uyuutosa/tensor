# arXiv v1 Submission Metadata

Use this exact text when filling the arxiv.org submission form.
Copy fields verbatim — arXiv does its own LaTeX rendering on
mathematical content but the abstract must be **plain text**.

---

## Title

```
Named Axes as a Systems Discipline: Hybrid Compile-Time and Runtime Coordination for Tensor Programs
```

## Authors

```
Yu Sato (Trait LLC)
```

Affiliation string: `Trait LLC`
Contact email: `sayu819@gmail.com`

## Subject classification

arXiv lets you pick **one primary** and any number of **cross-list** categories.

- **Primary**: `cs.PL` (Programming Languages)
- **Cross-list**:
  - `cs.LG` (Machine Learning)
  - `cs.MS` (Mathematical Software)

Rationale: the contribution is a **PL/type-system** result (hybrid
NTTP × DynamicShape) applied to the ML/numerical-computing domain.
Reviewers in cs.PL appreciate the type-level move; cs.LG readers
find it via the tensor-library use case; cs.MS for the
numerical-software audience that maintains the BLAS/Eigen ancestry.

## MSC / ACM codes (optional but recommended)

- MSC: `68N15` (Theory of programming languages); `65Y15` (Packaged
  methods for numerical algorithms)
- ACM: `D.3.3` (Language Constructs and Features); `G.4` (Mathematical
  Software)

## Comments field

```
13 pages, 1 figure, 4 tables. Companion code, reproducibility package, and tutorial:
https://github.com/uyuutosa/tensor. Pre-print of MLSys 2027 submission (target).
```

## Journal-ref / DOI

Leave blank for v1. Update on v2 if accepted to MLSys.

## License

Recommended: **CC BY 4.0** (attribution only; permits derivative work
and republication with credit). This is the default open-license
choice for arXiv ML/systems preprints and removes barriers to citation
+ inclusion in surveys.

## Abstract (plain text — paste this into the arxiv "Abstract" field)

```
Tensor programs routinely fail on axis-bug classes that positional shape
systems cannot catch: silent broadcasts across mis-named dimensions,
transposed contractions, reductions over the wrong axis. Existing named-axis
libraries (einops, einx, Haliax, Penzai, PyTorch named tensors) push these
failures from "silent wrong output" to "runtime exception" but leave the
host language's static type system unused.

We present a hybrid C++20 design that encodes axis information across the
compile-time / runtime boundary, with three contributions: (i) a "_tex"
user-defined literal that parses a LaTeX-math subset into a named-axis
expression AST whose structural indices the type system can reason about;
(ii) a hybrid system using non-type template parameter (NTTP) class types
for the static path and DynamicShape for the runtime path, with label
mismatches refused at template instantiation when shapes are known and
runtime guards when they are not -- one kernel implementation across both
paths; and (iii) a Hexagonal-lite kernel-backend port carrying three
substrates (a reference implementation, Eigen, and WebGPU via Dawn) behind
one C++20 concept.

A preliminary 10-entry static-catch benchmark across five named-tensor
libraries finds einops, einx, Haliax, and Penzai produce silent-wrong-output
on every entry; our static path refuses every entry at template
instantiation. A bundle-adjustment case study at 50-view by 200-landmark
scale across four backends operationalises the substrate-cost claim: the
named-axis layer adds no measurable overhead between the reference and
vectorised backends.

Code, bench harness, and reproducibility package are open-source at
https://github.com/uyuutosa/tensor.
```

Word count: ~250 words / ~1700 characters. arXiv limit is 1920 chars.

---

## Files in this submission

`tensor-arxiv-v1.tar.gz` contains:

| File | Purpose |
| ---- | ------- |
| `main.tex` | LaTeX source (will be re-compiled on arXiv's TeX Live) |
| `main.bbl` | Pre-compiled bibliography (so reviewers see the same refs even if their TeX Live differs) |
| `refs.bib` | BibTeX source (for completeness; arXiv compiles `main.tex` not `refs.bib`) |

Note on `main.bbl`: arXiv strongly recommends shipping the pre-compiled
`.bbl` so the bibliography renders identically to your local build. Do
**not** delete it.

## Pre-submission checklist

- [ ] Open https://arxiv.org/submit
- [ ] Pick "New submission"
- [ ] Step 1 — License: CC BY 4.0
- [ ] Step 2 — Title / Authors: paste from above
- [ ] Step 3 — Subject classes: cs.PL primary, cs.LG + cs.MS cross-list
- [ ] Step 4 — Abstract: paste plain-text abstract from above
- [ ] Step 5 — Upload: `tensor-arxiv-v1.tar.gz` (the tarball, not the .pdf — arXiv prefers source so its TeX Live can re-render)
- [ ] Step 6 — Preview: arXiv re-renders the .tex; check the rendered PDF matches `paper/mlsys-2027/main.pdf`
- [ ] Step 7 — Submit. Receive an arXiv ID like `2605.XXXXX`.
- [ ] After acceptance: announce the arXiv ID in this repo's README + the project blog if any.

## After v1 lands

The arXiv ID is permanent; v2 / v3 share the same ID with a `v2` / `v3`
suffix. Plan for at least two more versions:

- **v2** (September 2026): when the B-stage bench harness lands real
  ETH3D / Strecha / BAL numbers + the WebGPU row populates against a
  pinned-CUDA Docker image, push v2 with the complete Tables 3 & 4.
- **v3** (October 2026, pre-MLSys-submission): apply the MLSys 2027
  style file when CFP drops, polish for the conference reviewers.

Each version's commit hash and exact `main.tex` should be tagged on
this repository (`paper/v1`, `paper/v2`, `paper/v3`) for traceability.

---

## What I (the assistant) can and cannot do

I **cannot** submit to arXiv on your behalf — arxiv.org requires you to
log in with your endorsed author account, accept the licence terms,
and confirm the submission interactively. This is by design: arXiv
deliberately keeps the human-in-the-loop step to prevent automated
bulk uploads.

I **can**, and have, prepared the complete submission package above so
that the human-in-the-loop step is reduced to ~5 minutes of form-filling
on arxiv.org.
