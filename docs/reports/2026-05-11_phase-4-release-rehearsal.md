---
status: Stable
owner: tensor
last-reviewed: 2026-05-11
---

# Phase 4 release tag rehearsal — `0.1.0` ceremony rundown

| Metadata     | Value                                                          |
| ------------ | -------------------------------------------------------------- |
| Status       | Stable; rehearsal not yet executed against `0.1.0`. Document is a runbook the maintainer follows when actually cutting the release. |
| Type         | Layer B — release-readiness report (dated, append-only)         |
| Owner        | uyuutosa                                                       |
| Triggered by | post-pivot session "全部進めて" slice H — verify Phase 4 close criteria readiness and walk through the Git Flow release ceremony so the actual cut is a checklist, not a design exercise. |
| Related      | [`.claude/rules/version-control.md` §Cutting a release](../../.claude/rules/version-control.md); [arc42 §1 §6 success criteria](../arc42/01-introduction-and-goals/overview.md); [Phase 3 impl-plan](../impl-plans/2026-05-11_phase-3-webgpu.md). |

## Purpose

After 47 merged PRs over 2 days, the project is closer to `0.1.0` than it might appear from outside. This document does two things:

1. **Surveys the §6 success criteria** in [arc42 §1](../arc42/01-introduction-and-goals/overview.md) against the merged state to identify what genuinely blocks the release vs. what is comfort-padding.
2. **Walks through the Git Flow release ceremony** ([`.claude/rules/version-control.md` §Cutting a release](../../.claude/rules/version-control.md)) so the actual cut is a checklist with line-references, not improvisation.

The rehearsal is **not executed** in this document — no release branch is created, no version is bumped, no tag is pushed. The maintainer triggers the actual ceremony when the genuine blockers below are resolved.

---

## 1. Success-criteria audit (as of 2026-05-11 / PR #47)

From [arc42 §1 §6](../arc42/01-introduction-and-goals/overview.md):

| Criterion | State | Verdict |
| --------- | ----- | ------- |
| All ADR-0001 follow-ups closed | ADR-0001 follow-ups are inherited / refined by ADR-0010 + ADR-0013; cross-checked in the discussion-points report. The `tutorials/01-04` scaffolds noted as "scaffolded" in `tutorials/README.md` are the remaining items. | **Partially blocking** — see §2. |
| CMake 10-job CI matrix green | Achieved 2026-05-11 (PRs #9, #21, #38, #41). | ✅ |
| `tutorials/00_intro.ipynb` reproduces the 2016 blog post | Shipped PR #8. | ✅ |
| `tutorials/05_autograd-from-scratch.ipynb` walks the autograd primitive-by-primitive | Shipped PR #15. | ✅ |
| `tutorials/07_mlp-on-toy.ipynb` trains MLP on toy dataset | Shipped PR #17. | ✅ |
| `tutorials/08_swappable-backends.ipynb` demonstrates Hexagonal-lite payoff | Shipped PR #22. | ✅ |
| `tutorials/06_webgpu-acceleration.ipynb` runs WebGPU matmul, matches CPU | **Phase 3 — P3.M3 + P3.M4 WGSL sources shipped; dispatch wiring + tutorial deferred to P3.M3.2 / P3.M4.2.** | **Blocking — pre-condition: GPU runner.** |
| Jupyter Book site reachable on GitHub Pages | Scaffold + deploy workflow shipped PR #28. Site goes live when the maintainer enables Pages in repo settings (Source: GitHub Actions). | **Not blocking the tag; requires one-click maintainer action post-release.** |
| README leads with canonical-reference / educational framing | Reframed PR #39 (canonical-reference), reinforced PR #40 + #45. | ✅ |
| `CITATION.cff` at repo root | Shipped PR #40. | ✅ |
| `third_party/` under `VENDORED_FROM` discipline | Shipped PR #41. | ✅ |

## 2. What actually blocks `0.1.0`

The audit identifies **one** strict blocker and **one** soft blocker.

### Strict blocker — `tutorials/06_webgpu-acceleration.ipynb`

§6 lists this as a success criterion. The current state: WGSL kernel sources for element-wise (PR #43, #44) and tiled GEMM (PR #46) are committed; dispatch wiring is P3.M3.2 / P3.M4.2 and requires a self-hosted GPU runner (precondition tracked in ADR-0014 + impl-plan addendum).

**Options to resolve:**

1. **Wait** for the maintainer to provision a self-hosted GPU runner. Honest timeline depends on hardware availability; nothing the canonical-reference posture promises is broken by waiting.
2. **Re-scope §6** so `0.1.0` does not require tutorial 06. The release becomes the Phase 1-3-sources release; Phase 3 dispatch wiring + tutorial 06 become Phase 3.5 / Phase 4 deliverables. Document the re-scope as an addendum to arc42 §1 §6 with a one-line rationale.
3. **Ship tutorial 06 in "design walkthrough" mode** — the notebook narrates the WGSL kernel sources (already committed) + the dispatch design (already documented) without executing GPU code. This honors the educational-first / canonical-reference framing without waiting for hardware.

**Recommendation: Option 3 in the near term, with Option 2 as the explicit release decision.** A "design walkthrough" tutorial 06 makes the WGSL sources legible to a reader who is not running them, and is fully cite-able under the canonical-reference discipline. The re-scoping of §6 acknowledges that "runs on GPU and matches CPU" is a *post*-`0.1.0` criterion (it requires hardware that the project does not assume).

### Soft blocker — `tutorials/01-04` scaffolded chapters

The `tutorials/README.md` planned progression table lists 01-04 as "scaffolded" — i.e. file slots exist in the planned numbering but no notebooks have been written. The four shipped notebooks (00, 05, 07, 08) cover the headline pedagogical arc. The 01-04 scaffolds were originally meant to be:

| #  | Title |
| -- | ----- |
| 01 | Named-axis fundamentals (Phase 1) |
| 02 | Function and reference tensors as teaching exhibits |
| 03 | Convolutions reformulated as tensor inner products |
| 04 | TeX bridge — *the formula is the program* |

Content for 02 / 03 / 04 is largely covered already: tutorial 00 absorbs 02 and 03's material from the 2016 Qiita post; the `_tex` evaluator (PR #25) + LyX export (PR #31) cover 04's material outside the tutorial corpus.

**Options:**

1. **Drop 01-04 from the success criteria.** The educational arc is covered.
2. **Write 01-04 as short cross-reference notebooks** that point to 00, 05, 07, 08 and `_tex` examples.
3. **Defer to post-0.1.0.**

**Recommendation: Option 1.** The pedagogical arc is complete; the 01-04 scaffolds were architectural placeholders, not deliverables.

## 3. Git Flow release ceremony — checklist with line-references

When the blockers in §2 are resolved (or re-scoped), the actual cut follows [`.claude/rules/version-control.md` §Cutting a release](../../.claude/rules/version-control.md):

### Pre-flight (on `develop`)

1. **Verify `develop` is green.** All 10 CI jobs pass; `vendored-check` passes; notebook-CI validate job passes (notebook-CI execute job is best-effort weekly and not gating).
2. **Verify CHANGELOG `[Unreleased]` is comprehensive.** Every merged PR since the last logical alpha has an entry. The current state (PR #44+ already documented, latest entries cover #47) is in good shape.
3. **Final docs audit.** Confirm README status table, arc42 §1 success criteria, Phase 3 impl-plan addendum all reflect the release-eligible state. Update arc42 §1 last-reviewed date to the cut date.

### Cut

4. **Create the release branch.**

   ```bash
   git checkout develop && git pull
   git checkout -b release/0.1.0
   ```

5. **Version bumps on the release branch (no new features):**

   - `vcpkg.json` — `version-string`: `0.0.1-alpha` → `0.1.0`.
   - `CMakeLists.txt` — `project(tensor VERSION 0.0.1)` → `VERSION 0.1.0`.
   - `CITATION.cff` — confirm authors / preferred-citation are current; no version field yet (CFF versions track release tags via GitHub's "Cite this repository" tooling).
   - `CHANGELOG.md` — promote `[Unreleased]` to `## [0.1.0] — Phase 3 source-complete (YYYY-MM-DD)`; add a new empty `## [Unreleased]` above it. Cross-reference §6 re-scoping decision if Option 2 was chosen.
   - `README.md` status table — add a "Phase 4 — `0.1.0` shipped" row above the in-progress rows.
   - `book/intro.md` status table — same update as README.

6. **Bug fixes only on the release branch.** No new features. Any post-cut issue gets fixed here and back-merged to `develop`.

### Merge to `main` + tag

7. **PR `release/0.1.0` → `main`.** Merge with a merge commit (no squash; preserves Git Flow integration boundary). Tag `0.1.0` on `main`.

   ```bash
   git checkout main && git pull
   git tag -a 0.1.0 -m "0.1.0 — canonical-reference v0.1, Phase 3 source-complete"
   git push origin 0.1.0
   ```

8. **Verify GitHub Release.** Cut a GitHub Release from the tag (gh release create). Body: the `## [0.1.0]` section from CHANGELOG verbatim.

### Back-merge + cleanup

9. **PR `release/0.1.0` → `develop`** (back-merge). Merge commit. This returns the version bumps to integration.

10. **Delete the release branch.**

    ```bash
    git push origin --delete release/0.1.0
    git branch -D release/0.1.0
    ```

11. **Post-release hygiene:**

    - Enable GitHub Pages (Source: GitHub Actions) if not already enabled — the `deploy-book.yml` workflow publishes on the next push to `develop` after the back-merge.
    - Update `MEMORY.md` / project notes with the release pointer.
    - Open the post-release issue: "Phase 3 P3.M3.2 / P3.M4.2 — Dawn dispatch wiring + GPU runner".

## 4. Rehearsal output

This document is the rehearsal output. The actual cut is **not** performed in this PR. The maintainer triggers the ceremony when:

- §2 strict blocker is resolved (tutorial 06 lands in either Option 1 or Option 3 form).
- §2 soft blocker is resolved or formally dropped from §6.
- The CHANGELOG `[Unreleased]` section is freshly audited against `develop` HEAD.

Per the canonical-reference framing (ADR-0013), the `0.1.0` release is "v0.1 of a reference work" — not "an alpha grown up." That framing changes what success looks like: tag the work when the citable artifacts are in place and the headline pedagogical arc runs end-to-end on a reader's laptop, not when every line of GPU code is verified. This document's recommendations follow that framing.

## 5. References

- [`.claude/rules/version-control.md`](../../.claude/rules/version-control.md) — Git Flow ceremony source of truth.
- [arc42 §1 §6 success criteria](../arc42/01-introduction-and-goals/overview.md).
- [ADR-0013](../arc42/09-decisions/0013-reframe-as-canonical-reference-for-named-tensor-computation.md) — canonical-reference framing.
- [Phase 3 impl-plan addendum](../impl-plans/2026-05-11_phase-3-webgpu.md) — P3.M3.x / P3.M4.x sub-split + dispatch-wiring preconditions.
- [`CHANGELOG.md`](../../CHANGELOG.md) — current `[Unreleased]` block.
