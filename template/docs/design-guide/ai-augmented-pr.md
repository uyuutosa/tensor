---
status: Stable
owner: <placeholder>
last-reviewed: 2026-05-10
---

# AI-augmented PR — empirically grounded authoring rules

> **What this file is.** The convention this kit ships for writing the *description* of pull requests where an AI agent contributed code. It is opinionated by design: every rule below traces to a peer-reviewed paper or an industry standard, listed in the [References](#references) section.
>
> **What this file is not.** A tutorial for end users (that lives under [`../user-manual/tutorials/`](../user-manual/tutorials/)). A code-reading guide aimed at new contributors (that lives in [`code-tours.md`](./code-tours.md)). Documentation about AI-generated code at module level (that goes into the relevant [`../detailed-design/`](../detailed-design/) file with a Reading Order subsection).

The companion artefact is [`../../.github/PULL_REQUEST_TEMPLATE.md`](../../.github/PULL_REQUEST_TEMPLATE.md). This file explains *why each section exists*; the template is where authors fill in the blanks.

---

## 1. The problem this kit takes a position on

AI agents now produce reviewable code far faster than humans can verify it. Two recent peer-reviewed studies make the operational consequence explicit:

- LLM coding outputs are typically *miscalibrated* out of the box — confidence estimates do not track correctness. ([Spiess et al., ICSE 2025](https://www.software-lab.org/publications/icse2025_calibration.pdf))
- Field experiments at WirelessCar found that AI summaries on PRs help reviewers *until* the reviewer is "coloured by" the AI's framing, after which false-positive defects rise. ([arXiv 2505.16339](https://arxiv.org/abs/2505.16339))

Practitioners describe the resulting drag with several names — "verification debt", "review fatigue", "epistemic debt". The HCI literature calls the underlying construct **appropriate reliance** (and its inverse, *inappropriate* or *over*-reliance). The latter term is measurable, peer-reviewed ([Schemmer et al., IUI 2023](https://dl.acm.org/doi/10.1145/3581641.3584066)), and the one this kit adopts.

### Terminology this kit uses

- **Appropriate reliance gap** (primary) — the difference between (a) the reliance level a reviewer *should* place on an AI-generated change given its actual correctness, and (b) the level they *do* place on it given the framing in front of them. Empirical interventions are scored by how much they shrink this gap.
- **Epistemic debt** (secondary) — used only when describing *cumulative*, project-level effects of repeatedly merging under-verified AI output.
- **Verification debt** — *not* used as a formal term; appears in popular writing but is not an established construct in the literature.

---

## 2. What the evidence rejects

The most common instinct — "if the PR description is more thorough, reviewers will catch more bugs" — is empirically wrong in its naïve form.

| Intervention | Observed effect on overreliance | Source |
| --- | --- | --- |
| Add a long, full explanation of the AI's reasoning | **0 to negative** (overreliance can *increase*) | [Bansal et al., CHI 2021](https://dl.acm.org/doi/10.1145/3411764.3445717) |
| Show a numeric confidence percentage | Trust shifts, but most reviewers cannot detect miscalibration (≈ 67% miss it) | [Zhang et al., 2024](https://arxiv.org/abs/2402.07632) |
| Highlight individual tokens with confidence colour | Increases cognitive load; in code-review contexts the load swamps the signal | [Visible Language 59-2, 2025](https://www.visible-language.org/journal/issue-59-2-addressing-uncertainty-in-llm-outputs-for-trust-calibration-through-visualization-and-user-interface-design/) |
| Make the description longer than ~1,000 characters | Correlates with **longer** merge latency (no compensating defect-detection gain reported) | [Tao Xiao et al., FSE 2024](https://tao-xiao.github.io/files/Copilot4PR_FSE_2024.pdf) |

This kit therefore does not require any of the above patterns.

---

## 3. What the evidence supports

The following five interventions each have a measured positive effect, and together they are what the kit's [PR template](../../.github/PULL_REQUEST_TEMPLATE.md) is built from.

### 3.1 Risk self-disclosure, not full explanation

Three to five concrete risks, each one line, each labelled with a categorical confidence. Partial / risk-focused explanations reduce overreliance significantly where full explanations do not ([Vasconcelos et al., ACM HCI 2025](https://dl.acm.org/doi/10.1145/3710946); cost-benefit framing in [Vasconcelos et al., CSCW 2023](https://dl.acm.org/doi/10.1145/3579605)).

**Cap:** 3-5 items. More than 5 imposes engagement cost and reviewers skip the section entirely. Less than 3 signals "I have not actually thought about what could break".

### 3.2 Span-level, categorical confidence

Confidence is reported per **file or function**, not per line, and uses three labels (`high` / `medium` / `low`), not percentages. Span-level granularity matches how reviewers actually navigate code; categorical labels are more actionable than numeric ones for non-specialist readers ([Sun et al., UMAP Adjunct 2024](https://dl.acm.org/doi/10.1145/3708319.3734178); Visible Language 59-2, 2025).

If the underlying tool produces calibrated probabilities (Platt scaling, conformal prediction), bin them into the three labels before publishing.

### 3.3 Exactly one cognitive forcing question

Asking the reviewer a single, hard, change-specific question — "Which single line would cause the worst production failure if it has a subtle bug?" — reduces overreliance by approximately 10 percentage points among reviewers high in Need for Cognition ([Buçinca et al., CSCW 2021](https://dl.acm.org/doi/10.1145/3449287)).

**One question, not three.** The same paper warns that excessive forcing causes disengagement, particularly in low-NFC reviewers.

### 3.4 Verification budget

State the diff size and a recommended review time computed as `LoC / 200 × 30 minutes`. The 200-400 LoC band is the published peak for defect detection; detection rate collapses beyond ~1,000 LoC and after 60-90 continuous minutes (Cisco / SmartBear field data, widely cited; convergent with [Gonçalves et al., EMSE 2022](https://link.springer.com/article/10.1007/s10664-022-10123-8) on guided checklists reducing NASA-TLX load).

If the budget exceeds 60 minutes, the change should usually be split.

### 3.5 Disclosure of AI involvement

List the files where an AI agent generated more than ~30% of the new lines. The 30% line is a pragmatic disclosure threshold, not an empirical constant; the *requirement* to disclose at all derives from EU AI Act transparency expectations and from regulated-industry policies that treat AI authorship as a material fact. The mechanical convention of `Co-Authored-By:` trailers in commit messages ([documented by Anthropic Claude Code](https://github.com/anthropics/claude-code/issues/19925) and used by GitHub Copilot) covers the commit side; this section covers the PR-description side.

---

## 4. Length budget

Aim for **300 to 800 characters** total in the description body (excluding the templated section headers). Tao Xiao et al. (FSE 2024) found descriptions longer than ~1,000 characters correlate with longer merge latency without a corresponding quality gain.

The five sections above can comfortably fit in that budget if each list is kept to its cap. If you cannot fit, the change is probably too large; split the PR before lengthening the description.

---

## 5. Lifecycle

This file is durable convention, not a dated report. Update it when:

- New peer-reviewed evidence overturns one of §3's interventions.
- The kit raises or lowers the overall length budget.
- The companion `.github/PULL_REQUEST_TEMPLATE.md` is restructured (the two must stay aligned).

For when to author / supersede / archive design-guide files, see [`../WORKFLOW.md`](../WORKFLOW.md).

---

## References

### Peer-reviewed (primary)

- Bansal et al. (2021), *Does the Whole Exceed its Parts? The Effect of AI Explanations on Complementary Team Performance*, CHI — <https://dl.acm.org/doi/10.1145/3411764.3445717>
- Vasconcelos et al. (2023), *Explanations Can Reduce Overreliance on AI Systems During Decision-Making*, CSCW — <https://dl.acm.org/doi/10.1145/3579605>
- Buçinca et al. (2021), *To Trust or to Think: Cognitive Forcing Functions Can Reduce Overreliance on AI*, CSCW — <https://dl.acm.org/doi/10.1145/3449287>
- Schemmer et al. (2023), *Appropriate Reliance on AI Advice: Conceptualization and the Effect of Explanations*, IUI — <https://dl.acm.org/doi/10.1145/3581641.3584066>
- Spiess et al. (2025), *Calibration and Correctness of Language Models for Code*, ICSE — <https://www.software-lab.org/publications/icse2025_calibration.pdf>
- Vasconcelos et al. (2025), *Cognitive Forcing for Better Decision-Making: Reducing Overreliance Through Partial Explanations*, ACM HCI — <https://dl.acm.org/doi/10.1145/3710946>
- Gonçalves et al. (2022), *Do explicit review strategies improve code review performance?*, EMSE — <https://link.springer.com/article/10.1007/s10664-022-10123-8>
- Tao Xiao et al. (2024), *Generative AI for Pull Request Descriptions*, FSE — <https://tao-xiao.github.io/files/Copilot4PR_FSE_2024.pdf>
- Zhang et al. (2024), *Understanding the Effects of Miscalibrated AI Confidence on User Trust, Reliance, and Decision Efficacy* — <https://arxiv.org/abs/2402.07632>

### Industry standards and primary sources

- EU AI Act transparency provisions — see Anthropic Claude Code attribution issue: <https://github.com/anthropics/claude-code/issues/19925>
- Conventional Commits 1.0 — <https://www.conventionalcommits.org/>
- Google Engineering Practices, *What to look for in a code review* — <https://google.github.io/eng-practices/review/reviewer/looking-for.html>
- GitLab Description Templates — <https://docs.gitlab.com/user/project/description_templates/>
- Visible Language Journal 59-2 (2025) — <https://www.visible-language.org/journal/issue-59-2-addressing-uncertainty-in-llm-outputs-for-trust-calibration-through-visualization-and-user-interface-design/>
- Sun et al. (2024), *The Impact of Confidence Ratings on User Trust in LLMs*, UMAP Adjunct — <https://dl.acm.org/doi/10.1145/3708319.3734178>

### Grey literature (background only)

- *Tutorial-Driven AI Coding* (note.com, 2026) — <https://note.com/uyuutosa/n/n2f4addd6b42e>. The argument that motivated this kit's adoption of the conventions above; the conventions themselves trace to the peer-reviewed sources listed.
- WirelessCar field experiment, *Rethinking Code Review Workflows with LLM Assistance* — <https://arxiv.org/abs/2505.16339>
- Microsoft DevBlog, *Enhancing Code Quality at Scale with AI-Powered Code Reviews* — <https://devblogs.microsoft.com/engineering-at-microsoft/enhancing-code-quality-at-scale-with-ai-powered-code-reviews/>
