---
status: Draft
owner: tensor
last-reviewed: 2026-05-11
---

# `tensor` — Architecture Constraints (arc42 §2)

| Metadata      | Value                                                          |
| ------------- | -------------------------------------------------------------- |
| Status        | Draft                                                          |
| Type          | arc42 §2 (Architecture Constraints)                            |
| Owner         | uyuutosa                                                       |
| Last Updated  | 2026-05-11                                                     |

> Per arc42 §2: this file records **only what is genuinely fixed** — things that cannot be renegotiated by architecture. Items still up for debate live in §9 ADRs (and may move here once Accepted), not here.

## 1. Technical constraints

| #     | Constraint                                                              | Source                                                                                       | Expires when                                                          |
| ----- | ----------------------------------------------------------------------- | -------------------------------------------------------------------------------------------- | --------------------------------------------------------------------- |
| TC-1  | **C++20 minimum, C++23 features allowed under feature-test gating.**    | [ADR-0002](../09-decisions/0002-rewrite-on-cpp20-baseline-with-mdspan-interop.md)            | Phase 5+, when `std::linalg` ships widely — re-evaluate per ADR-0014. |
| TC-2  | **Header-only distribution.**                                            | [ADR-0008](../09-decisions/0008-distribute-as-header-only-with-jupyter-tutorials.md)         | Never under current scope — would supersede with a new ADR.            |
| TC-3  | **Build system: CMake ≥ 3.25 + vcpkg manifest mode.**                    | [ADR-0003](../09-decisions/0003-replace-eclipse-cdt-with-cmake-and-vcpkg.md)                 | Never under current scope.                                             |
| TC-4  | **Compiler matrix: GCC ≥ 11, Clang ≥ 13, MSVC ≥ 19.30, AppleClang ≥ 14.** | CI matrix in [`.github/workflows/ci.yml`](../../../.github/workflows/ci.yml)                  | Phase 5+ if C++23 / C++26 baseline forces a higher floor.              |
| TC-5  | **No proprietary GPU toolchain.** GPU work uses WebGPU via Dawn / gpu.cpp; CUDA-direct is explicitly disqualified. | [ADR-0006](../09-decisions/0006-adopt-webgpu-as-gpu-backend.md), [§5 ADR-0001 § Out of scope](../01-introduction-and-goals/overview.md)                | Never under current scope.                                             |
| TC-6  | **MIT license for first-party code under `include/`, `tests/`, `bench/`, `tutorials/`, `docs/`.** Vendored content under `third_party/` retains its upstream license. | `LICENSE` file at repo root; [ADR-0014 §Decision Outcome point 2](../09-decisions/0014-external-substrate-strategy.md) | Never.                                                                |
| TC-7  | **One canonical kernel-port surface (`KernelBackend`).** All GPU / SIMD / BLAS work plugs in via this port; the Domain depends on no adapter directly. | [ADR-0009](../09-decisions/0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md), [ADR-0011](../09-decisions/0011-kernel-backend-port-api.md)                                   | Never under current scope.                                             |
| TC-8  | **No ABI stability guarantee.** Versions remain alpha until the maintainer chooses to commit; even at `1.0.0` the project may break ABI in minor versions per the as-is positioning. | [ADR-0010 §Decision Outcome point 3](../09-decisions/0010-refine-positioning-to-educational-first-production-capable.md)                              | Only via a new ADR that explicitly supersedes ADR-0010 point 3.        |
| TC-9  | **Substrate vendoring discipline.** Bus-factor-1 dependencies are vendored under `third_party/<name>/` with a `VENDORED_FROM` record; CI (`tools/check-vendored.sh`) enforces. | [ADR-0014](../09-decisions/0014-external-substrate-strategy.md), `CONTRIBUTING.md` § Vendored third-party code | Never under current scope.                                             |

## 2. Organisational constraints

| #     | Constraint                                                              | Source                                                                                       |
| ----- | ----------------------------------------------------------------------- | -------------------------------------------------------------------------------------------- |
| OC-1  | **Solo maintainer bandwidth.** No CI matrix expansion, no operator-coverage commitment, no formal support SLA is sustainable from this budget. | [ADR-0001](../09-decisions/0001-pivot-to-educational-named-axis-dsl.md), [ADR-0010](../09-decisions/0010-refine-positioning-to-educational-first-production-capable.md) |
| OC-2  | **No external paid dependencies.** All build / test / publish tooling is free or already provided (vcpkg, GitHub Actions, conda-forge xeus-cpp, Jupyter Book). | repo root — no `package.json` runtime deps, no SaaS subscriptions |
| OC-3  | **Git Flow with merge commits.** Squash-merge is forbidden because it erases `git-subtree-dir:` metadata required by the `libs/pentaglyph-docs/` subtree. | [`.claude/rules/version-control.md` §Hard rules #8](../../../.claude/rules/version-control.md) |
| OC-4  | **English-by-default for docs.** Conversation between maintainer and contributors may be in Japanese; ADRs, arc42, detailed-design, CHANGELOG are English. | pentaglyph-docs scaffold convention; reinforced in `docs/STRATEGY.md` of the kit |
| OC-5  | **Canonical-reference posture is a maintenance multiplier.** Every new public name must trace to a source (paper / ADR / textbook). | [ADR-0013](../09-decisions/0013-reframe-as-canonical-reference-for-named-tensor-computation.md), `CONTRIBUTING.md` depth-over-breadth heuristic |

## 3. Regulatory constraints

None apply to first-party content. The project does not handle PII, does not perform regulated computation (medical / financial / safety-critical), and has no certification target.

Vendored content (currently only `third_party/gpu_cpp/`) carries the upstream's license. The maintainer's obligation is to preserve the LICENSE file and not modify the vendored code without re-vendoring discipline (per OC noted in TC-9 above).

## 4. Cross-references

- §1 Introduction and Goals: [`../01-introduction-and-goals/overview.md`](../01-introduction-and-goals/overview.md) — goals these constraints shape.
- §4 Solution Strategy: [`../04-solution-strategy/strategy.md`](../04-solution-strategy/strategy.md) — strategies that respect these constraints.
- §10 Quality Requirements: [`../10-quality/overview.md`](../10-quality/overview.md) — quality targets bounded by these constraints.
- §11 Risks: [`../11-risks/`](../11-risks/) — risks introduced by these constraints.
- ADR sequence: [`../09-decisions/`](../09-decisions/) — fourteen decisions, of which TC-1, TC-2, TC-3, TC-5, TC-6, TC-7, TC-8, TC-9, OC-1, OC-3, OC-5 quote directly.
