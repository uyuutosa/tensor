---
status: Draft
owner: tensor
last-reviewed: 2026-05-12
---

# `tensor` — Risks and Technical Debt (arc42 §11)

| Metadata      | Value                                                          |
| ------------- | -------------------------------------------------------------- |
| Status        | Draft                                                          |
| Type          | arc42 §11 (Risks and Technical Debt)                           |
| Owner         | uyuutosa                                                       |
| Last Updated  | 2026-05-12                                                     |

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
| R-S5 | **vcpkg baseline bump pulls Eigen 3 → 5 and breaks the Eigen backend.** | 🟢 Low | **No longer load-bearing.** PRs #60 / #61 / #62 shipped P3.M3.2 / P3.M4.2 / P3.M5 dispatch wiring against the locally-installed Dawn without needing a baseline bump. The `webgpu` manifest feature in `vcpkg.json` continues to document the bump path for any future user who wants Dawn from a non-local source. | If a future PR forces the bump, bundle Eigen 5 compatibility in the same PR with focused CI iteration. |

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
| R-M3 | **Documentation drift** — `arc42` / `detailed-design` / `ADR` content gets stale relative to merged code. | 🟡 Medium | Eight audits done (PRs #11, #29, #34, #45, #71, #72, #73, #74, #75). | Periodic audits + `.claude/rules/documentation.md` "code change implies doc change" PR-time discipline. R-A3's half-yearly audit also catches drift. |

## 4. Performance / quality risks

| # | Risk | Severity | Current state | Mitigation |
| -- | ---- | -------- | ------------- | ---------- |
| R-Q1 | **Autograd tape allocation pressure** dominates a real-size MLP training loop. | 🟡 Medium | One `std::function<void()>` per registered backward op; thread-local `std::vector` reallocation. | Discussion-points Axis C flags this; mitigation is a profile-driven 1-week investigation slice (P1.5+ slot). No action until evidence demands it (priority 1: clarity > priority 4: performance). |
| R-Q2 | **No real-GPU numerical verification** for the WebGPU backend until a self-hosted runner exists. | 🟢 Low | **No longer load-bearing.** PRs #60 / #61 / #62 ran the shipped WGSL kernels on the maintainer's RTX 3090 via locally-installed Dawn + Vulkan; 12 of 15 `KernelBackend` methods cross-validate against reference within `1e-5` / `1e-3` for `float`. The Phase 4 rehearsal Option 3 "design-walkthrough" framing for tutorial 06 remains in place (so the notebook stays GPU-free in CI), but the underlying *correctness* of the WGSL sources is no longer unverified. | If a self-hosted GPU runner ever lands, expand notebook CI to execute tutorial 06 cells. Until then, the maintainer reruns `webgpu` tests locally before each release per the [release rehearsal report](../../reports/2026-05-11_phase-4-release-rehearsal.md) §3 checklist. |
| R-Q3 | **`f64` operations on the WebGPU backend** fall through to reference, surprising users. | 🟢 Low | ADR-0012 documents the `f32`-only MVP. README + tutorial-08 mention the type-dispatch; PRs #60–#62 use `if constexpr (!std::is_same_v<T, float>)` to delegate non-`float` paths to reference at compile time so the fallthrough is unambiguous to anyone reading the adapter source. | Document explicitly in tutorial 06 once it gains live-execution cells; consider a runtime warning when an `f64` op hits the WebGPU backend if user reports surface. |

## 5. Phase 6 / Phase 6.5 risks (added 2026-05-13)

| Id   | Risk                                                                             | Likelihood | Active mitigations                                                                                                                                                 |
| ---- | -------------------------------------------------------------------------------- | ---------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| R-P1 | **nanobind 2.x API churn.** Pinned `>= 2.0, < 3` in `pyproject.toml`. | 🟢 Low | Half-yearly bibliography audit re-validates the pin. Phase 6 retrospective records four boundary papercuts that surfaced during M1–M5 implementation — none from nanobind churn, all from "convention learned on first trip". |
| R-P2 | **CPython stable-ABI surface vs minor-version perf trade-off.** | 🟢 Low | The cibuildwheel matrix covers CPython 3.9–3.13 explicitly. Stable-ABI is *not* enforced; each minor version gets its own wheel. |
| R-P3 | **PyPI `tensor-named-axis` name collides or maintainer can't register.** | 🟢 Low | M1 spike (PR #95) confirmed `tensor-named-axis` was free. Trusted-publisher policy is the maintainer's one-time setup before the first `0.2.0` tag. |
| R-P6.5.1 | **Companion-project versioning drift** — `tensor-named-axis-eigen==0.3.0` depending on `tensor-named-axis==0.3.0` means a `0.3.0` patch needs lockstep bumps. | 🟡 Medium | A `release.sh` helper bumps all three `pyproject.toml`s + tags them together. Tracked in [Phase 6.5 impl-plan §Risks](../../impl-plans/2026-05-13_phase-6-5-set-backend.md). |
| R-P6.5.2 | **Namespace-package conflict on `tensor/`.** Both `tensor-named-axis` and `tensor-named-axis-eigen` install under `tensor/`. | 🟢 Low | PEP-420 implicit namespace package (no `__init__.py` in the shared namespace dir); M1 smoke import verifies. |
| R-P6.5.3 | **`pip install` resolver pinning across companions.** A `0.3.0` base + `0.2.9` companion would mismatch ABI. | 🟢 Low | Each companion pins `tensor-named-axis==<exact-version>` so pip either upgrades the base or refuses. |
| R-P6.5.4 | **Dawn vcpkg port instability on Windows.** Dawn + Windows toolchain is known-fragile per ADR-0014 §1. | 🟡 Medium | Phase 6.5 M3 scopes Windows WebGPU out for the first cut; M4 stretch goal if maintainer's local Windows verification succeeds before `0.3.0`. |
| R-P6.5.5 | **nanobind 2.x type-registry is process-global; multiple `_tensor_native_*.so` cannot coexist in one process.** Surfaced during Phase 6.5 M3 implementation (PR #124): the second backend's `import` warns about duplicate `Axis` / `DynamicTensor` / etc. registration and the second module's class bindings are missing. nanobind 2.12 lacks pybind11's `py::module_local()` feature. | 🟡 Medium | M3 ships single-backend-per-process semantics: `tensor/__init__.py` detects which backend extensions are *installed* but lazy-loads exactly one (preference: `TENSOR_BACKEND` env var > reference > eigen > webgpu). `set_backend()` to a different installed backend raises with the env-var workaround. Phase 6.5 follow-up: explore the [`pyMODINIT_FUNC` symbol-mangling pattern](https://github.com/wjakob/nanobind/discussions) or switch the binding-level dispatch surface to be pure-Python (the C++-side adapter routing is unchanged). |
| R-P6 | **MathJax v2 (Plotly) vs MathJax v3 (Jupyter Book) collision.** Caught the hard way in PR #120 — the BA notebook's `$…$` math appeared as raw `\(…\)` text on the published site because Plotly's `notebook_connected` renderer hard-coded `include_mathjax="cdn"`. | 🟢 Low | Workaround documented in [`../../design-guide/python-notebook-authoring.md`](../../design-guide/python-notebook-authoring.md): monkey-patch `pio.to_html` to force `include_mathjax=False` in every notebook that imports Plotly. |
| R-P7 | **Notebooks committed un-executed render source-only on the published site.** Caught in PR #117 (Python notebooks): every `python/notebooks/*.ipynb` had `execution_count: None` so the Jupyter Book deploy emitted code blocks with no outputs. The same bug class held for `tutorials/*.ipynb` (C++) since Phase 4. | 🟢 Low | **Python side**: PR #118 added a CI gate that fails when any `python/notebooks/*.ipynb` is committed without `execution_count` or outputs. **C++ side**: PR #127 added xeus-cpp install + execute step in `deploy-book.yml` (`continue-on-error: true` because xeus-cpp is R-S2 / Medium). The two sides use different patterns — Python pre-executes locally, C++ executes at deploy time — but both end with rendered-with-outputs HTML. Documented in [`CONTRIBUTING.md` §Python notebooks](../../../CONTRIBUTING.md) and [`../../design-guide/python-notebook-authoring.md`](../../design-guide/python-notebook-authoring.md) §1. |
| R-P8 | **`book/_toc.yml` external paths silently 404 on the deployed site.** Pre-PR #116 every notebook + report chapter was missing from the published Jupyter Book because `../tutorials/…` paths lay outside Sphinx's `book/`-rooted source tree. | 🟢 Low | PR #116 added `book/stage.sh` (symlinks `book/{tutorials,python,docs} -> ../<same>`) called by `deploy-book.yml` before `jupyter-book build`. Pattern documented in `README.md` and CONTRIBUTING.md. |

## 6. Risk monitoring methodology

Each R-* row carries an implicit monitoring contract:

- **Likelihood column** is updated whenever evidence shifts. The 🟢/🟡/🔴 symbol is not decorative — it's the current best assessment as of the file's `last-reviewed` frontmatter date.
- **Active mitigations column** must include at least one *verifiable* item per row (CI job, ADR clause, code-level guard). "We watch carefully" is not a mitigation.
- **Lessons-learned entries (R-P6 / R-P7 / R-P8)** carry the introducing PR number so the next reviewer can read the change-history.

Review cadence (piggy-backed on existing artifacts per §10 §4):

| Trigger                       | What gets re-validated                                                                |
| ----------------------------- | ------------------------------------------------------------------------------------- |
| Half-yearly bibliography audit (next: **2026-11-11**) | Every R-* row: is the likelihood still accurate? Is each mitigation still in place? Are there new risks worth filing? |
| Every phase close (retrospective) | Phase-specific risks (R-P*, R-P3-related, etc.). New risks named in the retrospective land here as R-* rows in the next docs PR. |
| Per-PR that touches CI / build / deploy | The relevant R-* row's mitigation column is verified against the change. |
| Any "I just spent N hours debugging a class of bug I didn't see coming" event | File a new R-* row before closing the issue. |

The 🟢 / 🟡 / 🔴 symbols obey a strict definition:

- 🔴 **High**: actively threatens an unmet quality scenario or success criterion; needs a mitigation plan in the current PR.
- 🟡 **Medium**: known issue with active mitigation; review at next phase close.
- 🟢 **Low**: theoretical, well-mitigated, or transitive (depends on an external system that has its own monitoring).

A row that has been 🟢 for two consecutive audits without changes can be moved to a "Resolved / archived" subsection in a future cycle; the row's history stays for context.

## 7. Cross-references

- §1 success criteria these risks could block: [`../01-introduction-and-goals/overview.md`](../01-introduction-and-goals/overview.md)
- §2 constraints that bound these risks' impact: [`../02-architecture-constraints/overview.md`](../02-architecture-constraints/overview.md)
- §9 ADRs that codify mitigations: [`../09-decisions/`](../09-decisions/) — especially [ADR-0010](../09-decisions/0010-refine-positioning-to-educational-first-production-capable.md), [ADR-0015](../09-decisions/0015-aspire-to-canonical-reference-quality-not-self-anoint.md) (superseding [ADR-0013](../09-decisions/0013-reframe-as-canonical-reference-for-named-tensor-computation.md)), [ADR-0014](../09-decisions/0014-external-substrate-strategy.md), [ADR-0018](../09-decisions/0018-phase-6-python-sdk-entry-via-nanobind.md), [ADR-0019](../09-decisions/0019-phase-6-5-runtime-backend-selection-via-extras.md).
- §10 quality scenarios these risks threaten: [`../10-quality/overview.md`](../10-quality/overview.md). Cadence is shared (§10 §4).
- Per-phase risks-and-mitigations tables: [`../../impl-plans/`](../../impl-plans/) — Phase 3 plan has a richer GPU-specific table; [Phase 6.5 plan](../../impl-plans/2026-05-13_phase-6-5-set-backend.md) has R-P6.5.* tracking.
- Postmortems for materialised incidents (none yet, as of 2026-05-14): [`../../postmortems/`](../../postmortems/).

- §1 success criteria these risks could block: [`../01-introduction-and-goals/overview.md`](../01-introduction-and-goals/overview.md)
- §2 constraints that bound these risks' impact: [`../02-architecture-constraints/overview.md`](../02-architecture-constraints/overview.md)
- §9 ADRs that codify mitigations: [`../09-decisions/`](../09-decisions/) — especially [ADR-0010](../09-decisions/0010-refine-positioning-to-educational-first-production-capable.md), [ADR-0015](../09-decisions/0015-aspire-to-canonical-reference-quality-not-self-anoint.md) (superseding [ADR-0013](../09-decisions/0013-reframe-as-canonical-reference-for-named-tensor-computation.md)), [ADR-0014](../09-decisions/0014-external-substrate-strategy.md), [ADR-0018](../09-decisions/0018-phase-6-python-sdk-entry-via-nanobind.md), [ADR-0019](../09-decisions/0019-phase-6-5-runtime-backend-selection-via-extras.md).
- §10 quality scenarios these risks threaten: [`../10-quality/overview.md`](../10-quality/overview.md)
- Per-phase risks-and-mitigations tables: [`../../impl-plans/`](../../impl-plans/) — Phase 3 plan has a richer GPU-specific table; [Phase 6.5 plan](../../impl-plans/2026-05-13_phase-6-5-set-backend.md) has R-P6.5.* tracking.
