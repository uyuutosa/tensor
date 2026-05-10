<!--
Pentaglyph default PR template.

The structure below is empirically grounded — see
`docs/design-guide/ai-augmented-pr.md` for the research backing each section.

Replace inline guidance (the parts in <angle brackets>) with your content.
Keep the *whole* description between roughly 300 and 800 characters; longer
descriptions correlate with longer merge latency (Tao Xiao et al., FSE 2024).
-->

## Summary

<One paragraph. State the goal of this change and why it matters now. Avoid restating the diff.>

## What changed

- <bullet 1>
- <bullet 2>

## How to test

- <Specific commands or steps a reviewer can copy-paste. Cite test names where relevant.>

## Risks I am uncertain about

<!--
List 3-5 specific risks. Each item: <file or function>: <one-line risk>. Confidence: high | medium | low.

Empirical basis: partial / risk-focused explanations reduce reviewer overreliance
(Vasconcelos et al., ACM HCI 2025; Schemmer et al., IUI 2023). Long, confidence-
boosting explanations have been shown to *increase* overreliance instead
(Bansal et al., CHI 2021).

Use *categorical* confidence labels — not numeric percentages. Most reviewers
cannot detect miscalibration in numeric AI confidence (Zhang et al., 2024).
-->

1. `<file:function>` — <one-line risk>. Confidence: <high | medium | low>.
2. `<file:function>` — <one-line risk>. Confidence: <high | medium | low>.
3. `<file:function>` — <one-line risk>. Confidence: <high | medium | low>.

## Cognitive forcing question for reviewers

<!--
Pick exactly one question that requires the reviewer to *think*, not just nod.
A single forcing question reduces overreliance (Buçinca et al., CSCW 2021);
multiple forcing questions become noise.
-->

> Before approving: which single line in this PR would cause the worst
> production failure if it has a subtle bug, and why?

## Verification budget

<!--
Recommended review time = (diff LoC) / 200 × 30 minutes.
Cisco/SmartBear field data: defect detection drops from 87% to 28% above ~1,000 LoC,
and again after 60-90 minutes of continuous review. Splitting large PRs is cheaper
than asking a reviewer to push past these thresholds.
-->

- Diff size: <N> LoC. Recommended review time: <N/200 × 30> minutes.
- Skim OK (high-confidence sections): <list>
- Deep review required (low-confidence sections): <list>

## AI involvement

<!--
Disclose which sections of this PR were written or substantially shaped by an AI
agent. EU AI Act transparency expectations and several regulated-industry policies
treat this as a material disclosure, not a courtesy.

Use Co-Authored-By trailers in the *commit messages* themselves, not here.
-->

- AI-assisted: <yes | no | partial>
- Files where AI generated more than ~30% of the new lines: <list, or "none">

## Tests

- Added: <list of added test names>
- Not covered (and why): <list, or "none">

## Related

- ADR(s): <NNNN, or "none">
- Issue / ticket: <ID, or "none">
- Code tour (if any): <path/to/tour.tour>
