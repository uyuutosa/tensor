---
status: Accepted
owner: tensor
last-reviewed: 2026-05-12
---

# ADR-0017: Clarify the reproducibility envelope — build + test + bench is the 30-minute budget; notebook execution is a separate audit step

| Metadata  | Value                                                                  |
| --------- | ---------------------------------------------------------------------- |
| Status    | **Accepted**                                                           |
| Type      | ADR (Type 5 / MADR v3.0 / arc42 §9)                                    |
| Date      | 2026-05-12                                                             |
| Deciders  | uyuutosa                                                               |
| Consulted | Claude (session of 2026-05-12) + the post-investigation report.        |
| Informed  | future contributors; future bibliography auditors                      |
| Ticket    | —                                                                      |
| Refines   | [ADR-0015 §Compliance](0015-aspire-to-canonical-reference-quality-not-self-anoint.md) — the "30 minute" envelope wording is clarified, not retracted. ADR-0015 remains in force; only its §Compliance third bullet is rewritten. |
| Triggered by | [Post-investigation landscape re-check + adversarial self-review (2026-05-12)](../../reports/2026-05-12_landscape-recheck-and-adversarial-review.md) §B.3 — finding F-4. |

---

## Context and Problem Statement

[ADR-0015 §Compliance](0015-aspire-to-canonical-reference-quality-not-self-anoint.md) states the third forcing-function discipline ("reproducibility") as:

> "clean clone → build + bench + notebook in under 30 minutes."

The 2026-05-12 adversarial self-review checked whether this envelope is achievable in practice. The component costs on a 2026 Linux workstation:

| Component                                            | Wall-clock estimate           |
| ---------------------------------------------------- | ----------------------------- |
| `git clone` + vcpkg bootstrap                        | 3–5 min                       |
| `cmake --preset=default` (config)                    | 2–3 min                       |
| `cmake --build --preset=default` (compile)           | 5–8 min (header-only)         |
| `ctest --preset=default`                             | 1–2 min                       |
| Optional `-DTENSOR_BUILD_BENCH=ON` + bench run       | + 2–3 min                     |
| Subtotal — build + test + bench                      | **13–21 min** ✅ in 30 min     |
| **Notebook environment**: `conda env create -f tutorials/environment.yml` + xeus-cpp install + Clang-Repl + CppInterOp | **+ 5–10 min** if conda-forge channel cached, **+ 20+ min** if anything needs source build |
| **Notebook execution**: 5 notebooks × 3–8 min each  | + 15–40 min                   |
| Subtotal — notebook                                  | **20–50 min**                 |

The build + test + bench half **fits in 30 minutes**. The notebook half **does not** — its lower bound (20 min on a warm conda cache, 5 notebooks executing fast) is already a meaningful share of the 30-min budget; its upper bound (50 min on a cold cache) blows the budget on its own.

Two problems with the current wording:

1. **It overpromises.** A bibliography auditor with the [ADR-0015 compliance check](0015-aspire-to-canonical-reference-quality-not-self-anoint.md) in hand who tries to verify the 30-min envelope on a clean machine will fail it 30–50% of the time on the *notebook* dimension alone — even when build + test + bench were green. That undermines the discipline.
2. **It conflates two distinct verification surfaces.** Build + test + bench tests the *library*; notebook execution tests the *Jupyter substrate + tutorial code*. Bundling them under one timer makes failures hard to diagnose.

[ADR-0015's body is immutable](0015-aspire-to-canonical-reference-quality-not-self-anoint.md) per the kit's MADR rule (no edits to Accepted ADRs). The clarification must live in a successor ADR.

---

## Decision Drivers

- **DD-1**: Verifiability. A discipline that an auditor can actually verify in practice is worth more than an aspirational claim that fails most of the time.
- **DD-2**: Diagnostic separability. Build + test + bench failures and notebook-execution failures have completely different root causes (toolchain / Eigen vs. xeus-cpp / Clang-Repl); merging them under one stopwatch is anti-pedagogical.
- **DD-3**: Substrate honesty. Notebook execution depends on `xeus-cpp` ([ADR-0014 §3](0014-external-substrate-strategy.md)) which is young (v0.10, April 2026) and whose install time varies with the user's conda cache state. We don't control that variance; pretending we do is dishonest.
- **DD-4**: Discipline preservation. The point of the reproducibility forcing function is to keep the project *checkable* — the clarification must keep the substantive bite, not soften the discipline.
- **DD-5**: ADR-0015 framing alignment. The aspirational ("aspires to canonical-reference quality") framing of ADR-0015 means we should aspire *honestly*, not optimistically. A 30-min envelope we hit ≥ 90% of the time is a stronger discipline than a 30-min envelope we hit ≤ 50% of the time.

---

## Considered Options

1. **Leave ADR-0015 §Compliance as-is; trust the auditor to interpret the wording charitably.** (Status quo.)
2. **Split the envelope into two budgets** (chosen): "(a) clean clone → build + test + bench in under 30 minutes; (b) notebook execution is a separate audit step, run when reasonable, without a fixed time budget."
3. **Drop "notebook" from the discipline entirely** — only commit to build + test + bench. Notebooks become a documentation deliverable, not a reproducibility discipline.
4. **Relax the budget to 60 minutes** to include the notebook half.

---

## Decision Outcome

**Chosen option: 2 — split the envelope.**

The reproducibility discipline of [ADR-0015 §Compliance bullet 3](0015-aspire-to-canonical-reference-quality-not-self-anoint.md) is read in force as:

> **(a)** clean clone → `cmake --preset=default` + `cmake --build --preset=default` + `ctest --preset=default` + (optional) `-DTENSOR_BUILD_BENCH=ON` and a bench run **in under 30 minutes**, on a 2026 Linux / macOS / Windows workstation with a warm package cache.
>
> **(b)** Notebook execution is a **separate audit step**. The auditor runs `conda env create -f tutorials/environment.yml` once, then `jupyter nbconvert --to notebook --execute` on each notebook in [`tutorials/`](../../../tutorials/). No fixed time budget; expected total in the 20–50 min range depending on conda cache state. The audit step is satisfied when **every notebook executes top-to-bottom without error**, output cells match the committed values (modulo platform-specific transient floating-point differences within `1e-5`), and the final state of `00_intro` reproduces the 2016 Qiita post's convolutions-as-inner-products result.

Both halves remain part of the bibliography audit ([ADR-0015 §Compliance bullet 4](0015-aspire-to-canonical-reference-quality-not-self-anoint.md): half-yearly, first due 2026-11-11). The half-yearly auditor reports both budgets independently.

### Y-statement summary

> In the context of **ADR-0015's three-discipline aspiration, where the third discipline's "build + bench + notebook in under 30 minutes" wording overpromised by counting notebook execution (20–50 min) within the same envelope as build + test + bench (13–21 min)**, facing **the choice between leaving the wording optimistic-but-inaccurate, dropping notebook from the discipline, relaxing the budget, or splitting it**, we decided for **splitting the envelope into (a) 30-minute build + test + bench and (b) separate notebook audit step with no fixed time budget**, to achieve **a discipline an auditor can actually verify ≥ 90% of the time without weakening the bite**, accepting **two compliance checkpoints instead of one (a minor bookkeeping cost the bibliography audit absorbs cleanly)**.

---

## Pros and Cons of the Options

### Option 1: Leave ADR-0015 §Compliance as-is

- Pros: zero churn; no new ADR to track.
- Cons: an auditor who reads the discipline literally will fail the envelope on the notebook dimension half the time, even when the library is healthy. The discipline loses credibility through false-positive failure.

### Option 2: Split the envelope (chosen)

- Pros: the build-+-test-+-bench budget is genuinely tight and genuinely meaningful (a green half-hour clean-clone build is a strong correctness signal); the notebook half stays in the audit but does not have to hit an arbitrary deadline; failures are diagnosable.
- Cons: two budgets to track; ADR-0015 §Compliance wording needs a pointer to ADR-0017.

### Option 3: Drop "notebook" from the discipline entirely

- Pros: simplest.
- Cons: tutorials are the [primary educational deliverable per ADR-0008](0008-distribute-as-header-only-with-jupyter-tutorials.md); dropping them from the reproducibility forcing function would weaken the canonical-reference framing where it matters most.

### Option 4: Relax the budget to 60 minutes

- Pros: keeps one stopwatch.
- Cons: 60 minutes for "clean clone → working" is not credible as a discipline; it would invite auditors to skip the check entirely. The whole point of a 30-minute envelope is that it is achievable on a coffee break.

---

## Consequences

### Positive

- The bibliography auditor can verify (a) deterministically (most of the time, on a warm cache) and report (b) separately.
- The forcing-function discipline keeps its bite: 30 min for the library proper is genuinely strict; the notebook audit gives independent signal about the Jupyter substrate.
- Failures get diagnosable: "the build is broken" vs "the notebook substrate is broken" become separable causes.

### Negative

- ADR-0015 §Compliance now reads with a "→ see ADR-0017 for the operational split" pointer; readers must follow the link.
- One more place to keep in sync if the budget needs to change in future (which it might if Phase 5 / 6 / linalg shim add compile-time cost).

### Neutral

- The other two disciplines of ADR-0015 (bibliography, ubiquitous-language) are not touched.
- The half-yearly audit cadence (first audit 2026-11-11) is unchanged.

### Follow-ups

- [ ] `docs/reports/2026-11-11_first-bibliography-audit.md` (when written) reports both (a) and (b) as separate sections.
- [ ] `tutorials/environment.yml` should be sanity-checked at every notebook-CI run so the audit's setup step has a known-good baseline.

---

## Compliance / Validation

- **Verification**: at each half-yearly bibliography audit, the auditor runs (a) and (b) on a clean checkout and records the wall-clock times. The audit's output report names both budgets and their pass/fail state. A failure of (a) is treated as a discipline regression (open an issue); a failure of (b) is treated as a substrate or tutorial bug (file accordingly).
- **Frequency**: per-audit (half-yearly cadence from ADR-0015 §Compliance bullet 4).

---

## More Information

- Source of the finding: [`docs/reports/2026-05-12_landscape-recheck-and-adversarial-review.md`](../../reports/2026-05-12_landscape-recheck-and-adversarial-review.md) §B.3 (F-4).
- Refined: [ADR-0015 §Compliance bullet 3](0015-aspire-to-canonical-reference-quality-not-self-anoint.md).
- Related notebook substrate decision: [ADR-0014 §Decision Outcome point 3](0014-external-substrate-strategy.md) — the xeus-cpp choice that drives the notebook-execution variance.
- Related distribution decision: [ADR-0008](0008-distribute-as-header-only-with-jupyter-tutorials.md) — notebooks as the primary educational deliverable.
- impl-plan for the post-investigation P0 + P1 tasks: [`docs/impl-plans/2026-05-12_post-investigation-tasks.md`](../../impl-plans/2026-05-12_post-investigation-tasks.md) — this ADR is T-8.
