---
status: Stable
owner: tensor
last-reviewed: 2026-05-04
---

# arc42 §9 — Architecture Decision Records (ADRs)

> Authoritative source for the section: <https://docs.arc42.org/section-9/>
> Authoritative source for the format: **MADR v3.0** — <https://adr.github.io/madr/>

## File naming

`NNNN-<kebab-title>.md` where `NNNN` is a zero-padded 4-digit sequence, globally unique across the project.

Examples:

- `0001-adopt-postgres-as-primary-store.md`
- `0002-use-jwt-for-service-to-service-auth.md`

## Template

Always start from [`../../templates/5_adr.md`](../../templates/5_adr.md). Do not invent a homemade ADR format.

## Status legend

| Status                | Meaning                                                                   |
| --------------------- | ------------------------------------------------------------------------- |
| **Proposed**          | Draft; under team review. Body may still be edited.                       |
| **Accepted**          | Ratified. **Body is immutable** — supersede with a new ADR, do not edit.  |
| **Rejected**          | Considered and explicitly rejected. Kept for traceability.                |
| **Superseded by NNNN**| Replaced by the cited ADR. Read the superseding ADR for the current decision. |
| **Deprecated**        | No longer relevant; no active replacement.                                |

## Authoring rules

1. **One file per decision.** No grouped ADRs.
2. **Status field** must be one of the values in the legend above.
3. **All external rationale** (regulatory documents, design specs, business analysis) must be absorbed into the ADR body or linked. Do not link to ephemeral chat / ticket comments as the only source.
4. **Date in ISO 8601** (`YYYY-MM-DD`).
5. **Once `Accepted`, body is immutable.** If the decision changes, write a new ADR with `Supersedes: NNNN`, then update the old ADR's status to `Superseded by MMMM`.
6. **Y-statement** required in the Decision Outcome section (Olaf Zimmermann form).
7. **Minimums**: 3 Decision Drivers, 2 Considered Options, Consequences in Positive / Negative / Neutral form, Compliance / Validation section.

## Index (this project's actual ADR roll-call, 2026-05-14)

21 ADRs as of 2026-05-14 (0001-0019 Accepted; 0020 + 0021 Proposed). Grouped thematically; the supersession chain is captured in the diagram below the table.

### Positioning + canonical-reference framing

| #     | File                                                                                                  | Title                                                                                                            | Status                                          | Date       |
| ----- | ----------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------- | ----------------------------------------------- | ---------- |
| 0001  | [`0001-pivot-to-educational-named-axis-dsl.md`](./0001-pivot-to-educational-named-axis-dsl.md)         | Pivot to educational named-axis DSL                                                                              | Accepted (refined by 0010, framed by 0015)      | 2026-05-10 |
| 0010  | [`0010-refine-positioning-to-educational-first-production-capable.md`](./0010-refine-positioning-to-educational-first-production-capable.md) | Educational-first, production-capable                                                                            | Accepted                                        | 2026-05-11 |
| 0013  | [`0013-reframe-as-canonical-reference-for-named-tensor-computation.md`](./0013-reframe-as-canonical-reference-for-named-tensor-computation.md) | Reframe as canonical reference                                                                                   | **Superseded by 0015**                          | 2026-05-11 |
| 0015  | [`0015-aspire-to-canonical-reference-quality-not-self-anoint.md`](./0015-aspire-to-canonical-reference-quality-not-self-anoint.md)            | Aspire to canonical-reference quality (don't self-anoint)                                                        | Accepted (refined by 0017)                     | 2026-05-11 |
| 0017  | [`0017-clarify-reproducibility-envelope.md`](./0017-clarify-reproducibility-envelope.md)               | Clarify reproducibility envelope (build+test+bench vs notebook audit)                                            | Accepted                                        | 2026-05-12 |
| 0021  | [`0021-strategic-narrowing-to-specialist-reference-positioning.md`](./0021-strategic-narrowing-to-specialist-reference-positioning.md) | Strategic narrowing — adopt specialist-reference positioning, decline production-scale ML chase                  | Proposed                                        | 2026-05-14 |

### Substrate + build

| #     | File                                                                                                  | Title                                                                                                            | Status                                          | Date       |
| ----- | ----------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------- | ----------------------------------------------- | ---------- |
| 0002  | [`0002-rewrite-on-cpp20-baseline-with-mdspan-interop.md`](./0002-rewrite-on-cpp20-baseline-with-mdspan-interop.md) | Rewrite on C++20 baseline with `mdspan` interop                                                                  | Accepted                                        | 2026-05-10 |
| 0003  | [`0003-replace-eclipse-cdt-with-cmake-and-vcpkg.md`](./0003-replace-eclipse-cdt-with-cmake-and-vcpkg.md) | Replace Eclipse CDT with CMake + vcpkg                                                                            | Accepted                                        | 2026-05-10 |
| 0014  | [`0014-external-substrate-strategy.md`](./0014-external-substrate-strategy.md)                         | External substrate strategy (Dawn / xeus-cpp / kokkos-stdBLAS)                                                   | Accepted (point 2 refined by 0016)              | 2026-05-12 |
| 0016  | [`0016-substrate-refinement-drop-gpu-cpp-talk-to-dawn-directly.md`](./0016-substrate-refinement-drop-gpu-cpp-talk-to-dawn-directly.md) | Drop gpu.cpp wrapper, talk to Dawn directly                                                                       | Accepted                                        | 2026-05-12 |

### API surface (C++ side)

| #     | File                                                                                                  | Title                                                                                                            | Status                                          | Date       |
| ----- | ----------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------- | ----------------------------------------------- | ---------- |
| 0004  | [`0004-adopt-hybrid-named-axis-api.md`](./0004-adopt-hybrid-named-axis-api.md)                         | Adopt hybrid runtime + NTTP named-axis API                                                                       | Accepted                                        | 2026-05-10 |
| 0005  | [`0005-adopt-tex-lyx-as-authoring-surface.md`](./0005-adopt-tex-lyx-as-authoring-surface.md)           | Adopt TeX / LyX as authoring surface (`_tex` UDL)                                                                | Accepted                                        | 2026-05-10 |
| 0007  | [`0007-adopt-autograd-as-first-class-subsystem.md`](./0007-adopt-autograd-as-first-class-subsystem.md) | Adopt tape-based reverse-mode autograd                                                                            | Accepted                                        | 2026-05-10 |
| 0008  | [`0008-distribute-as-header-only-with-jupyter-tutorials.md`](./0008-distribute-as-header-only-with-jupyter-tutorials.md) | Distribute as header-only with bundled Jupyter tutorials                                                          | Accepted                                        | 2026-05-10 |

### Architecture + backend port

| #     | File                                                                                                  | Title                                                                                                            | Status                                          | Date       |
| ----- | ----------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------- | ----------------------------------------------- | ---------- |
| 0006  | [`0006-adopt-webgpu-as-gpu-backend.md`](./0006-adopt-webgpu-as-gpu-backend.md)                         | Adopt WebGPU (Dawn) as GPU backend                                                                                | Accepted                                        | 2026-05-10 |
| 0009  | [`0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md`](./0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md) | Adopt DDD ubiquitous-language + Hexagonal-lite                                                                    | Accepted                                        | 2026-05-11 |
| 0011  | [`0011-kernel-backend-port-api.md`](./0011-kernel-backend-port-api.md)                                 | `KernelBackend` port API (15 methods)                                                                            | Accepted                                        | 2026-05-11 |
| 0012  | [`0012-webgpu-adapter-implementation-design.md`](./0012-webgpu-adapter-implementation-design.md)       | WebGPU adapter implementation design (Phase 3 anchor)                                                            | Accepted                                        | 2026-05-11 |

### Phase 6 + 6.5 + 6.6 (Python SDK)

| #     | File                                                                                                  | Title                                                                                                            | Status                                          | Date       |
| ----- | ----------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------- | ----------------------------------------------- | ---------- |
| 0018  | [`0018-phase-6-python-sdk-entry-via-nanobind.md`](./0018-phase-6-python-sdk-entry-via-nanobind.md)     | Phase 6 — Python SDK entry via nanobind                                                                          | Accepted                                        | 2026-05-12 |
| 0019  | [`0019-phase-6-5-runtime-backend-selection-via-extras.md`](./0019-phase-6-5-runtime-backend-selection-via-extras.md) | Phase 6.5 — runtime backend selection via PEP-508 extras                                                          | Accepted (set_backend rebind partially superseded by 0020) | 2026-05-13 |
| 0020  | [`0020-multi-backend-runtime-dispatch-via-type-owner-separation.md`](./0020-multi-backend-runtime-dispatch-via-type-owner-separation.md) | Phase 6.6 — multi-backend runtime dispatch via type-owner-separation (lifts R-P6.5.5)                            | Proposed                                        | 2026-05-14 |

### Supersession / refinement chain

```text
ADR-0001 ─── refined by ──→ ADR-0010 (educational-first, production-capable)
   │
   └─ framed by ADR-0013 (canonical-reference declaration)
                  │
                  └─ SUPERSEDED BY ADR-0015 (aspire to canonical-reference quality)
                                      │
                                      ├─ Compliance bullet 3 refined by ADR-0017
                                      └─ operationalised by ADR-0021 (strategic narrowing; 2026-05-14 landscape response)

ADR-0014 §Decision Outcome point 2 ── refined by ──→ ADR-0016
          (gpu.cpp wrapper)                            (Dawn direct via webgpu_cpp.h)

ADR-0018 (Phase 6 entry) ── §F deferred to ──→ ADR-0019 (Phase 6.5 packaging)
                                                       │
                                                       └─ set_backend rebind point partially SUPERSEDED BY
                                                          ADR-0020 (Phase 6.6 type-owner-separation; R-P6.5.5 lift)
```

A row's "Status" column is canonical; this diagram is the human-readable summary. When a new supersession lands, both the row's status field and this diagram are bumped in the same docs PR.

## References

- MADR v3.0 — <https://adr.github.io/madr/>
- Michael Nygard original (2011) — <https://www.cognitect.com/blog/2011/11/15/documenting-architecture-decisions>
- Y-statements (Olaf Zimmermann) — <https://medium.com/olzzio/y-statements-10eb07b5a177>
- arc42 §9 — <https://docs.arc42.org/section-9/>
