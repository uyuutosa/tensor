---
status: Draft
owner: tensor
last-reviewed: 2026-05-12
---

# `tensor` — Architecture Constraints (arc42 §2)

| Metadata      | Value                                                          |
| ------------- | -------------------------------------------------------------- |
| Status        | Draft                                                          |
| Type          | arc42 §2 (Architecture Constraints)                            |
| Owner         | uyuutosa                                                       |
| Last Updated  | 2026-05-12                                                     |

> Per arc42 §2: this file records **only what is genuinely fixed** — things that cannot be renegotiated by architecture. Items still up for debate live in §9 ADRs (and may move here once Accepted), not here.

## 1. Technical constraints

| #     | Constraint                                                              | Source                                                                                       | Expires when                                                          |
| ----- | ----------------------------------------------------------------------- | -------------------------------------------------------------------------------------------- | --------------------------------------------------------------------- |
| TC-1  | **C++20 minimum, C++23 features allowed under feature-test gating.**    | [ADR-0002](../09-decisions/0002-rewrite-on-cpp20-baseline-with-mdspan-interop.md)            | Phase 5+, when `std::linalg` ships widely — re-evaluate per ADR-0014. |
| TC-2  | **Header-only distribution.**                                            | [ADR-0008](../09-decisions/0008-distribute-as-header-only-with-jupyter-tutorials.md)         | Never under current scope — would supersede with a new ADR.            |
| TC-3  | **Build system: CMake ≥ 3.25 + vcpkg manifest mode.**                    | [ADR-0003](../09-decisions/0003-replace-eclipse-cdt-with-cmake-and-vcpkg.md)                 | Never under current scope.                                             |
| TC-4  | **Compiler matrix: GCC ≥ 11, Clang ≥ 13, MSVC ≥ 19.30, AppleClang ≥ 14.** | CI matrix in [`.github/workflows/ci.yml`](../../../.github/workflows/ci.yml)                  | Phase 5+ if C++23 / C++26 baseline forces a higher floor.              |
| TC-5  | **No proprietary GPU toolchain.** GPU work uses WebGPU via Dawn (`webgpu_cpp.h` direct per [ADR-0016](../09-decisions/0016-substrate-refinement-drop-gpu-cpp-talk-to-dawn-directly.md)); CUDA-direct is explicitly disqualified. | [ADR-0006](../09-decisions/0006-adopt-webgpu-as-gpu-backend.md), [§5 ADR-0001 § Out of scope](../01-introduction-and-goals/overview.md)                | Never under current scope.                                             |
| TC-6  | **MIT license for first-party code under `include/`, `tests/`, `bench/`, `tutorials/`, `docs/`.** Vendored content under `third_party/` retains its upstream license. | `LICENSE` file at repo root; [ADR-0014 §Decision Outcome point 2](../09-decisions/0014-external-substrate-strategy.md) | Never.                                                                |
| TC-7  | **One canonical kernel-port surface (`KernelBackend`).** All GPU / SIMD / BLAS work plugs in via this port; the Domain depends on no adapter directly. | [ADR-0009](../09-decisions/0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md), [ADR-0011](../09-decisions/0011-kernel-backend-port-api.md)                                   | Never under current scope.                                             |
| TC-8  | **No ABI stability guarantee.** Versions remain alpha until the maintainer chooses to commit; even at `1.0.0` the project may break ABI in minor versions per the as-is positioning. | [ADR-0010 §Decision Outcome point 3](../09-decisions/0010-refine-positioning-to-educational-first-production-capable.md)                              | Only via a new ADR that explicitly supersedes ADR-0010 point 3.        |
| TC-9  | **Substrate vendoring discipline.** Bus-factor-1 dependencies are vendored under `third_party/<name>/` with a `VENDORED_FROM` record; CI (`tools/check-vendored.sh`) enforces. | [ADR-0014](../09-decisions/0014-external-substrate-strategy.md), `CONTRIBUTING.md` § Vendored third-party code | Never under current scope.                                             |
| TC-10 | **Python SDK build chain.** nanobind ≥ 2.0 (`<3`) + scikit-build-core ≥ 0.10 + CPython ≥ 3.9 (3.9–3.13 in the cibuildwheel matrix); NumPy ≥ 1.20 runtime dep. Stable ABI is *not* enforced — the per-version wheel matrix covers the supported range explicitly. | [ADR-0018](../09-decisions/0018-phase-6-python-sdk-entry-via-nanobind.md), `pyproject.toml` | Phase 6.6+ if nanobind 3.x ships breaking changes — re-evaluate per the half-yearly bibliography audit. |
| TC-11 | **Python wheel packaging via PEP-508 extras, not a fat wheel.** `tensor-named-axis` ships reference-only (~5 MB); `[eigen]` / `[webgpu]` / `[all]` pull companion projects with the matching adapter compiled in. Runtime `tensor.set_backend()` switches between *installed* backends; missing backends raise `RuntimeError` with install instructions. | [ADR-0019](../09-decisions/0019-phase-6-5-runtime-backend-selection-via-extras.md) | Only via a new ADR that supersedes ADR-0019 (e.g. if PyPI per-file size limits change materially). |
| TC-12 | **One canonical Python entry surface (`tensor` package).** All Python entry points route through nanobind into the same C++ Domain; the Python side ships no algebra logic. The companion projects (`tensor-named-axis-eigen`, `tensor-named-axis-webgpu`) install under the same `tensor/` PEP-420 implicit namespace package so `import tensor` works regardless of which extras are installed. | [ADR-0018 §F](../09-decisions/0018-phase-6-python-sdk-entry-via-nanobind.md), [ADR-0019 §"Risks"](../09-decisions/0019-phase-6-5-runtime-backend-selection-via-extras.md) R-P6.5.2 | Never under current scope. |

## 2. Organisational constraints

| #     | Constraint                                                              | Source                                                                                       |
| ----- | ----------------------------------------------------------------------- | -------------------------------------------------------------------------------------------- |
| OC-1  | **Solo maintainer bandwidth.** No CI matrix expansion, no operator-coverage commitment, no formal support SLA is sustainable from this budget. | [ADR-0001](../09-decisions/0001-pivot-to-educational-named-axis-dsl.md), [ADR-0010](../09-decisions/0010-refine-positioning-to-educational-first-production-capable.md) |
| OC-2  | **No external paid dependencies.** All build / test / publish tooling is free or already provided (vcpkg, GitHub Actions, conda-forge xeus-cpp, Jupyter Book). | repo root — no `package.json` runtime deps, no SaaS subscriptions |
| OC-3  | **Git Flow with merge commits.** Squash-merge is forbidden because it erases `git-subtree-dir:` metadata required by the `libs/pentaglyph-docs/` subtree. | [`.claude/rules/version-control.md` §Hard rules #8](../../../.claude/rules/version-control.md) |
| OC-4  | **English-by-default for docs.** Conversation between maintainer and contributors may be in Japanese; ADRs, arc42, detailed-design, CHANGELOG are English. | pentaglyph-docs scaffold convention; reinforced in `docs/STRATEGY.md` of the kit |
| OC-5  | **Canonical-reference-quality discipline is a maintenance multiplier.** Every new public name must trace to a source (paper / ADR / textbook); ubiquitous-language and reproducibility lines are PR-time review obligations. | [ADR-0015](../09-decisions/0015-aspire-to-canonical-reference-quality-not-self-anoint.md) (superseding [ADR-0013](../09-decisions/0013-reframe-as-canonical-reference-for-named-tensor-computation.md)), `CONTRIBUTING.md` depth-over-breadth heuristic |

## 3. Regulatory constraints

None apply to first-party content. The project does not handle PII, does not perform regulated computation (medical / financial / safety-critical), and has no certification target.

Vendored content carries the upstream's license. As of 2026-05-12, `third_party/` is empty — the only vendored substrate (`gpu_cpp/` at tag 0.2.0) was removed once [ADR-0016](../09-decisions/0016-substrate-refinement-drop-gpu-cpp-talk-to-dawn-directly.md) superseded ADR-0014 §Decision Outcome point 2 (Dawn is reached directly via `webgpu_cpp.h` instead). For any future vendoring (e.g. `kokkos/stdBLAS` for the `tensor::linalg` shim), the maintainer's obligation is to preserve the LICENSE file and not modify the vendored code without re-vendoring discipline (per TC-9 above).

## 4. Constraint review cadence

Constraints expire (per the "Expires when" column). The review cadence is **piggy-backed on the half-yearly bibliography audit** (per [ADR-0015 §Compliance](../09-decisions/0015-aspire-to-canonical-reference-quality-not-self-anoint.md) / [ADR-0017](../09-decisions/0017-clarify-reproducibility-envelope.md)):

| Half-yearly audit | What gets re-validated for §2                                                                |
| ----------------- | -------------------------------------------------------------------------------------------- |
| TC-1, TC-4         | C++ baseline: GCC / Clang / MSVC / AppleClang floors. Bumped if Phase 5+ adopts C++23 features the floor can't compile. |
| TC-3               | CMake floor; bumped when a vcpkg port requires it.                                            |
| TC-9               | `third_party/` inventory + `VENDORED_FROM` records resolve. Empty as of 2026-05-13.           |
| TC-10              | nanobind upper-bound (`< 3`) — bumped if nanobind 3.x ships breaking changes that affect the binding surface. |
| TC-11              | PEP-508 extras packaging — re-evaluated if PyPI per-file size limits change materially or fat-wheel becomes preferable. |
| OC-2               | Free-tier tooling — re-validated if GitHub Actions / Pages / Codespaces / mybinder.org pricing changes. |

First audit: **2026-11-11** (six months from ADR-0015 + ADR-0017 acceptance). Output: a Layer-B report under [`../../reports/`](../../reports/) titled `YYYY-MM-DD_bibliography-audit.md` per the half-yearly convention.

Constraints **lifted** out of cycle (without waiting for the audit) require:

1. A new ADR superseding the relevant `Source` ADR in column 3.
2. Updates to every cross-reference in this table.
3. A retrospective report covering the why-now.

The §11 risk register row that pointed at the constraint gets a status update on every audit (active / resolved / superseded).

## 5. Cross-references

- §1 Introduction and Goals: [`../01-introduction-and-goals/overview.md`](../01-introduction-and-goals/overview.md) — goals these constraints shape.
- §4 Solution Strategy: [`../04-solution-strategy/strategy.md`](../04-solution-strategy/strategy.md) — strategies that respect these constraints.
- §10 Quality Requirements: [`../10-quality/overview.md`](../10-quality/overview.md) — quality targets bounded by these constraints.
- §11 Risks: [`../11-risks/`](../11-risks/) — risks introduced by these constraints.
- ADR sequence: [`../09-decisions/`](../09-decisions/) — fourteen decisions, of which TC-1, TC-2, TC-3, TC-5, TC-6, TC-7, TC-8, TC-9, OC-1, OC-3, OC-5 quote directly.
