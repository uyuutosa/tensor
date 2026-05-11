---
status: Draft
owner: tensor
last-reviewed: 2026-05-11
---

# `tensor` — Risks and Technical Debt (arc42 §11)

| Metadata      | Value                                                          |
| ------------- | -------------------------------------------------------------- |
| Status        | Draft                                                          |
| Type          | arc42 §11 (Risks and Technical Debt)                           |
| Owner         | uyuutosa                                                       |
| Last Updated  | 2026-05-11                                                     |

> Per arc42 §11: this file consolidates the **risks** that could derail the project and the **technical debt** the maintainer has accepted. Per-phase impl-plans carry their own risks-and-mitigations tables; this file promotes the durable / cross-phase risks to a single audited place.

## Severity legend

- **🔴 High** — would block a phase deliverable or force a strategic pivot if it materialises.
- **🟡 Medium** — would slow a phase deliverable; mitigation cost ≤ a few PRs.
- **🟢 Low** — would cause a contained inconvenience; mitigated by documentation alone.

## 1. Substrate risks (external dependencies)

These are the risks ADR-0014 was written to manage. The 2026-05-11 external-substrate research ([`docs/reports/2026-05-11_external-substrate-research.md`](../../reports/2026-05-11_external-substrate-research.md)) drove the current mitigation set.

| # | Risk | Severity | Current state | Mitigation |
| -- | ---- | -------- | ------------- | ---------- |
| R-S1 | **gpu.cpp goes dormant** (was the originally-chosen WebGPU C++ wrapper). | 🟢 Low | **No longer load-bearing.** [ADR-0016](../09-decisions/0016-substrate-refinement-drop-gpu-cpp-talk-to-dawn-directly.md) (2026-05-12) supersedes ADR-0014 §Decision Outcome point 2: the project talks to Dawn directly via Dawn's own `webgpu_cpp.h` (Google-maintained, always synchronised with the linked binary). `third_party/gpu_cpp/` was removed once Stage 3 dispatch wiring (PRs #60 / #61 / #62) committed against `webgpu_cpp.h`. The vendoring discipline that originally caught this risk (PR #41 + the local build that surfaced the 14-month ABI drift) remains in `tools/check-vendored.sh` for future vendored substrates. |
| R-S2 | **xeus-cpp breaks on a Clang upgrade.** Young project (v0.10.0 April 2026); short stability track record. | 🟡 Medium | Active development; new releases ~ monthly. | `notebook-ci.yml` `legacy-xeus-cling` job runs `00_intro.ipynb` against xeus-cling as a smoke fallback. If xeus-cpp breaks, the worst case is the Jupyter Book deploy falls back to pre-rendered output. |
| R-S3 | **Dawn vcpkg port disappears or refuses to build on a supported triplet.** | 🟡 Medium | Port present and current at `20260410.140140` (2026-04-20). | `wgpu-native` is the documented fallback ([ADR-0006](../09-decisions/0006-adopt-webgpu-as-gpu-backend.md), [ADR-0012](../09-decisions/0012-webgpu-adapter-implementation-design.md)). The Hexagonal port keeps the runtime swap to one CMake variable. |
| R-S4 | **`std::linalg` ships earlier than expected and diverges from kokkos/stdBLAS.** | 🟢 Low | Not shipped in any vendor STL as of 2026-05. | The planned `tensor::linalg` shim uses `__cpp_lib_linalg` feature detection (per ADR-0014); when `<linalg>` ships, the shim flips and downstream sees no break. |
| R-S5 | **vcpkg baseline bump (needed for P3.M3.2 / P3.M4.2) pulls Eigen 3 → 5 and breaks the Eigen backend.** | 🟡 Medium | Baseline pinned at `99a97de2...` (no Eigen 5); the `webgpu` manifest feature documents the bump path. | When P3.M3.2 lands, bundle the baseline bump + Eigen 5 compatibility in one PR with focused CI iteration. CHANGELOG `[Unreleased]` documents the choice. |

## 2. Architectural risks

| # | Risk | Severity | Current state | Mitigation |
| -- | ---- | -------- | ------------- | ---------- |
| R-A1 | **Domain-adapter coupling drifts** — a future PR includes `tensor/core/<X>.hpp` from an adapter header without noticing. | 🟢 Low | One hard rule from ADR-0009; reviewed at PR time. | Plan to add a `grep` CI job that fails when `include/tensor/core/*.hpp` (excluding `concepts.hpp`) imports from `tensor/{autograd,tex}/` or from a sibling adapter. Tracked in [`docs/design-guide/architectural-discipline.md`](../../design-guide/architectural-discipline.md). |
| R-A2 | **Three named-tensor types (Tensor / DynamicTensor / TypedTensor) confuse readers.** | 🟢 Low | Decision guide [`docs/user-manual/how-to/named-tensor-types.md`](../../user-manual/how-to/named-tensor-types.md) (PR #35). | Re-audit at every major release whether the three types still serve distinct purposes; collapse one if redundant. The discussion-points report Axis B records the rejected collapse options. |
| R-A3 | **Three discipline-claims (bibliography / ubiquitous-language / reproducibility — G-8) silently rot.** | 🟡 Medium | Half-yearly bibliography audit per ADR-0015 §Compliance (superseding ADR-0013 §Compliance). | First audit due 2026-11-11. The audit checks: ADR cross-refs resolve; glossary covers every public name; clean clone → build + bench + notebook in under 30 minutes; no user-facing string asserts declarative "is the canonical reference" form. |

## 3. Maintainer-bandwidth risks

| # | Risk | Severity | Current state | Mitigation |
| -- | ---- | -------- | ------------- | ---------- |
| R-M1 | **Solo bandwidth saturates** — incoming issues or PR review backlog grow faster than the maintainer's capacity. | 🟡 Medium | None to-date. | `CONTRIBUTING.md` sets expectations: depth-over-breadth heuristic; fast responses not guaranteed. ADR-0010 + ADR-0013 limit acceptable contribution scope. Worst-case fallback is archiving the project with a public message; OC-1 already accepts this risk. |
| R-M2 | **Production users file coverage-parity issues.** | 🟢 Low | None to-date. | README + ADR-0010 disclaimer language; CONTRIBUTING.md depth-over-breadth heuristic. Politely close with a link to the relevant ADR. |
| R-M3 | **Documentation drift** — `arc42` / `detailed-design` / `ADR` content gets stale relative to merged code. | 🟡 Medium | Three audits done (PR #11, PR #29, PR #34, PR #45). | Periodic audits + `.claude/rules/documentation.md` "code change implies doc change" PR-time discipline. R-A3's half-yearly audit also catches drift. |

## 4. Performance / quality risks

| # | Risk | Severity | Current state | Mitigation |
| -- | ---- | -------- | ------------- | ---------- |
| R-Q1 | **Autograd tape allocation pressure** dominates a real-size MLP training loop. | 🟡 Medium | One `std::function<void()>` per registered backward op; thread-local `std::vector` reallocation. | Discussion-points Axis C flags this; mitigation is a profile-driven 1-week investigation slice (P1.5+ slot). No action until evidence demands it (priority 1: clarity > priority 4: performance). |
| R-Q2 | **No real-GPU numerical verification** for the WebGPU backend until a self-hosted runner exists. | 🔴 High (for Phase 3 close) | WGSL kernel sources committed (PRs #43, #44, #46); dispatch wiring stub-delegates. | Phase 4 rehearsal report (#48) recommends Option 3 "design-walkthrough" tutorial 06 — narrate the WGSL sources + dispatch design without executing GPU code. `0.1.0` ships under this framing; real GPU verification is post-`0.1.0`. |
| R-Q3 | **`f64` operations on the WebGPU backend** fall through to reference, surprising users. | 🟢 Low | ADR-0012 documents the `f32`-only MVP. README + tutorial-08 mention the type-dispatch. | Document explicitly in tutorial 06 (when written); add a runtime warning when a `f64` op hits the WebGPU backend (deferred to P3.M3.2 PR). |

## 5. Cross-references

- §1 success criteria these risks could block: [`../01-introduction-and-goals/overview.md`](../01-introduction-and-goals/overview.md)
- §2 constraints that bound these risks' impact: [`../02-architecture-constraints/overview.md`](../02-architecture-constraints/overview.md)
- §9 ADRs that codify mitigations: [`../09-decisions/`](../09-decisions/) — especially [ADR-0010](../09-decisions/0010-refine-positioning-to-educational-first-production-capable.md), [ADR-0015](../09-decisions/0015-aspire-to-canonical-reference-quality-not-self-anoint.md) (superseding [ADR-0013](../09-decisions/0013-reframe-as-canonical-reference-for-named-tensor-computation.md)), [ADR-0014](../09-decisions/0014-external-substrate-strategy.md).
- §10 quality scenarios these risks threaten: [`../10-quality/overview.md`](../10-quality/overview.md)
- Per-phase risks-and-mitigations tables: [`../../impl-plans/`](../../impl-plans/) — Phase 3 plan has a richer GPU-specific table.
