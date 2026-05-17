---
name: paper-review
description: >
  Top-tier-venue review of a research paper. Scores against the 7-dimension
  rubric (novelty, soundness, reproducibility, comparisons, limitations,
  related work, presentation) used by MLSys / NeurIPS / ICML / PLDI / OOPSLA
  programme committees. Outputs structured per-dimension scores (0–10) with
  specific evidence, ranked actionable gaps, and an accept/reject
  recommendation. Read-only: never modifies the paper.
disable-model-invocation: false
---

# /paper-review — top-tier venue review

Use this skill when you want a **conversation-free, structured review** of a
research paper, scored as a top-tier ML / systems / PL programme committee
member would. Useful before arXiv submission, before camera-ready, or any
time you want a sanity check.

## What it does

1. Locates the paper (PDF + LaTeX source) at one of:
   - `$1` (explicit path argument)
   - `paper/**/main.{tex,pdf}` (project default)
   - `papers/**/main.{tex,pdf}` (alternative layout)
2. Invokes the **`paper-reviewer`** agent with the located source.
3. The agent reads the paper end-to-end and scores against the 7-dimension
   rubric used in top-tier programme-committee review.
4. Returns a structured report: scores, evidence per dimension, ranked
   gaps, accept/borderline/reject recommendation.

This skill **does not** modify the paper, write reviews-as-documents, or
contact any external service. It is safe to run repeatedly.

## When to use

- **Before arXiv v1**: catch overclaim, missing baselines, citation gaps
  before the public timestamp.
- **Before submission**: predict what the actual reviewers will flag.
- **After every major PDCA iteration**: check whether the dimensions
  whose scores were low have improved.
- **Pre-rebuttal**: run again after addressing reviewer comments to verify
  no regression.

## When NOT to use

- You want to apply the changes the review surfaces → review first, then
  apply by hand (the skill is intentionally read-only).
- You want a single-issue check (e.g. "does §3 have a figure?") → use
  Read / Grep directly; this skill is full-rubric.
- The paper is a draft outline with most sections empty → wait until at
  least one full PDCA pass has been done; the rubric is calibrated for
  draft-ready text.

## Arguments

```
/paper-review [<path>]
```

- `<path>` (optional): path to a `.tex`, `.pdf`, or directory containing
  `main.tex`. Defaults to the project default search.
- No args: searches the project defaults; if multiple papers are found,
  asks the user which to review.

## Output shape

```text
==============================================================
PAPER:   paper/mlsys-2027/main.pdf
TARGET:  MLSys 2027 (inferred from path + bib style)
PAGES:   13 (body 10p, refs 11–13)
==============================================================

RUBRIC SCORES (target: ≥ 7/10 per dimension for top-tier acceptance)
  Novelty            : 8/10  hybrid NTTP × DynamicShape is C++-novel
  Soundness          : 9/10  methodology rigorous; one open question on §4.2
  Reproducibility    : 9/10  bench/ harness runs end-to-end
  Comparisons        : 9/10  6-library × n=10 real data
  Limitations        : 8/10  L1–L4 honest; L4 could quantify
  Related work       : 9/10  38 cites spanning frameworks / PL theory / SfM
  Presentation       : 7/10  figure count low; could use 1 more diagram
  ──────────────────────────────────
  Overall            : 8.4/10  → ACCEPT (target ≥ 7.0)

EVIDENCE PER DIMENSION
  Novelty (8):
    +  central design (hybrid NTTP × DynamicShape) is the only entry in
       the (type-parameter, compile-time-when-known) cell of Tab. 2
    +  Appendix A shows the static_assert fires reproducibly
    -  the consteval lift (§3.4) is future work, not new tonight

  Soundness (9):
    +  §6.1 methodology distinguishes structural vs numerical Silent
    +  §6.2 includes a numerical-correctness gate before timing
    ?  §4.2 promotion uses a from_dynamic that is described but not
       benchmarked separately for its overhead

  ... (one block per dimension)

RANKED GAPS (top three; address before submission)
  G1 [presentation]   Add a Figure 2 showing the static-catch table as a
                      stacked-bar chart; a reviewer's eye lands on figures
                      first and Tab. 3's numbers are visually flat.
  G2 [soundness]      Either benchmark or argue-around the from_dynamic
                      cost in §4.2; right now it is hand-waved.
  G3 [comparisons]    The WebGPU row is a $^\flat$ placeholder; either
                      drop the row from Tab. 4 (clean look) or move to
                      §8.L3 with a 2-line explanation of the environment
                      block.

RECOMMENDATION: ACCEPT (overall 8.4 / 10 ≥ 7.0 threshold).
  Conditional on addressing G1 and G2 before submission.
  G3 can wait for the v2 / camera-ready iteration.
```

## Tuning the rubric

The 7-dimension rubric is implemented in
`.claude/agents/paper-reviewer.md`. The dimensions and weights are derived
from MLSys / NeurIPS / ICML / PLDI / OOPSLA reviewer guidelines (the
intersection — every dimension shows up in every venue's guidelines we've
seen). To re-target for a different venue (e.g. ACM CHI emphasises study
design + ecological validity differently), edit the agent's rubric block.

## See also

- `/review` — code review (different artefact type)
- `/security-review` — security-specific review
- `.claude/agents/paper-reviewer.md` — the underlying rubric and scoring
  agent
