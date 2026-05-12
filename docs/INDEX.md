---
status: Stable
owner: tensor
last-reviewed: 2026-05-12
---

# `tensor` `docs/` — Index

`tensor` is a named-axis differentiable tensor library that aspires to be the canonical reference for differentiable named-tensor computation in modern C++ ([ADR-0015](./arc42/09-decisions/0015-aspire-to-canonical-reference-quality-not-self-anoint.md), supersedes [ADR-0013](./arc42/09-decisions/0013-reframe-as-canonical-reference-for-named-tensor-computation.md)). This `docs/` tree is its companion: every public name in the library has a definition here, and every accepted ADR is part of the bibliography. The directory follows [pentaglyph-docs](https://github.com/uyuutosa/pentaglyph-docs) — **arc42 + C4 + MADR + Diátaxis bound by one workflow**.

- **For "where do I put this doc?"** → [`WORKFLOW.md`](./WORKFLOW.md)
- **For "why is the layout this way?"** → [`STRATEGY.md`](./STRATEGY.md)
- **If you are an AI agent** → [`AI_INSTRUCTIONS.md`](./AI_INSTRUCTIONS.md)

## Where to start by audience

| Audience | First read | Then |
| -------- | ---------- | ---- |
| **New reader / API user** | [`../README.md`](../README.md) → [`book/intro.md`](../book/intro.md) | [`arc42/01-introduction-and-goals/overview.md`](./arc42/01-introduction-and-goals/overview.md) §1 + [`../tutorials/00_intro.ipynb`](../tutorials/00_intro.ipynb) |
| **Contributor** | [`../CONTRIBUTING.md`](../CONTRIBUTING.md) → [`design-guide/architectural-discipline.md`](./design-guide/architectural-discipline.md) | [`arc42/05-building-blocks/overview.md`](./arc42/05-building-blocks/overview.md) + the [`detailed-design/`](./detailed-design/) instance closest to your work |
| **Implementer / future port** | [`arc42/04-solution-strategy/strategy.md`](./arc42/04-solution-strategy/strategy.md) → [`arc42/09-decisions/`](./arc42/09-decisions/) (16 ADRs) | [`detailed-design/kernel-backend-port.md`](./detailed-design/kernel-backend-port.md) for the port contract; [`detailed-design/webgpu-*.md`](./detailed-design/) for adapter examples |
| **Textbook author / researcher** | [`arc42/12-glossary/overview.md`](./arc42/12-glossary/overview.md) → [`../CITATION.cff`](../CITATION.cff) | the four `detailed-design/tensor-*.md` (`core` / `autograd` / `tex`) Domain instances + the `reports/` historical record |

## Standards adopted

| Concern                  | Standard       | Authoritative source           | Local home                                 |
| ------------------------ | -------------- | ------------------------------ | ------------------------------------------ |
| Architecture description | **arc42**      | <https://arc42.org/overview/>  | [`arc42/`](./arc42/)                       |
| Architecture diagrams    | **C4 model**   | <https://c4model.com>          | [`diagrams/c4/`](./diagrams/c4/)           |
| Decision records         | **MADR v3.0**  | <https://adr.github.io/madr/>  | [`arc42/09-decisions/`](./arc42/09-decisions/) |
| User docs taxonomy       | **Diátaxis**   | <https://diataxis.fr>          | [`user-manual/`](./user-manual/)           |

## Layer A — durable design (slow change, code-coupled)

- [`arc42/`](./arc42/) — arc42 §1–§12 architecture description. **All 12 sections substantive** as of 2026-05-12; entry points: [§1 Goals](./arc42/01-introduction-and-goals/overview.md), [§4 Strategy](./arc42/04-solution-strategy/strategy.md), [§5 Building blocks](./arc42/05-building-blocks/overview.md), [§9 ADR index](./arc42/09-decisions/), [§12 Glossary](./arc42/12-glossary/overview.md).
- [`arc42/09-decisions/`](./arc42/09-decisions/) — **16 ADRs** (Accepted). ADR-0013 → superseded by [ADR-0015](./arc42/09-decisions/0015-aspire-to-canonical-reference-quality-not-self-anoint.md); ADR-0014 §Decision Outcome point 2 → refined by [ADR-0016](./arc42/09-decisions/0016-substrate-refinement-drop-gpu-cpp-talk-to-dawn-directly.md). Accepted ADR bodies are immutable — supersede with a new ADR.
- [`detailed-design/`](./detailed-design/) — **7 Template-3 instances**: [`tensor-core.md`](./detailed-design/tensor-core.md) (Domain centerpiece), [`tensor-autograd.md`](./detailed-design/tensor-autograd.md), [`tensor-tex.md`](./detailed-design/tensor-tex.md), [`webgpu-element-wise-kernels.md`](./detailed-design/webgpu-element-wise-kernels.md), [`webgpu-gemm-kernel.md`](./detailed-design/webgpu-gemm-kernel.md), [`webgpu-broadcast-kernels.md`](./detailed-design/webgpu-broadcast-kernels.md), [`kernel-backend-port.md`](./detailed-design/kernel-backend-port.md).
- [`diagrams/c4/`](./diagrams/c4/) — Structurizr DSL (`workspace.dsl`) is the source of truth; SVG renders are committed under `exports/` so repo web UIs can display them without local tooling.
- [`design-guide/`](./design-guide/) — operational conventions: [`architectural-discipline.md`](./design-guide/architectural-discipline.md) (Hexagonal enforcement), [`version-control.md`](./design-guide/version-control.md), [`code-tours.md`](./design-guide/code-tours.md), [`ai-augmented-pr.md`](./design-guide/ai-augmented-pr.md).
- [`user-manual/`](./user-manual/) — Diátaxis quadrants. Currently populated: [`how-to/named-tensor-types.md`](./user-manual/how-to/named-tensor-types.md) (Tensor vs DynamicTensor vs TypedTensor selection).
- [`api-contract/`](./api-contract/) — scaffold; `tensor` is a C++ header-only library and has no network surface, so this directory stays empty by design.

## Layer B — volatile working material (dated, append-only)

- [`impl-plans/`](./impl-plans/) — dated per-phase plans (5 plans: revival Phase 1, Phase 2 autograd, Phase 2.5 backend port + Eigen, Phase 3 WebGPU, …).
- [`reports/`](./reports/) — one-shot research / evaluation / retrospective reports (8 reports as of 2026-05-12). Entry points: [`2026-05-11_phase-1-retrospective.md`](./reports/2026-05-11_phase-1-retrospective.md), [`2026-05-11_phase-2-and-2-5-retrospective.md`](./reports/2026-05-11_phase-2-and-2-5-retrospective.md), [`2026-05-11_backend-performance-comparison.md`](./reports/2026-05-11_backend-performance-comparison.md) (with 2026-05-12 three-backend RTX 3090 measurements), [`2026-05-11_open-discussion-points.md`](./reports/2026-05-11_open-discussion-points.md) (eight axes of "next decisions"), [`2026-05-11_phase-4-release-rehearsal.md`](./reports/2026-05-11_phase-4-release-rehearsal.md), [`2026-05-12_gpu-cpp-dawn-abi-drift-and-raw-dawn-smoke.md`](./reports/2026-05-12_gpu-cpp-dawn-abi-drift-and-raw-dawn-smoke.md).
- [`postmortems/`](./postmortems/) — scaffold; no incidents to date.
- `task-list/`, `cost-estimates/` — scaffolds; not used by this project.

## Templates

See [`templates/README.md`](./templates/README.md) for the index and selection flow.

- [`templates/0_default.md`](./templates/0_default.md) — fallback when none of 1–8 fit
- [`templates/1_architecture-overview.md`](./templates/1_architecture-overview.md) — arc42 §1+§3+§4+§5+§8 + C4 L1/L2 system overview
- [`templates/2_prd.md`](./templates/2_prd.md) — PRD with `FR-<CAT>-NNN` / `NFR-<CAT>-NNN` IDs
- [`templates/3_module-detailed-design.md`](./templates/3_module-detailed-design.md) — Google Design Doc + Pragmatic Engineer module spec (this project's 7 `detailed-design/` instances all use it)
- [`templates/4_use-case.md`](./templates/4_use-case.md) — Cockburn casual + user story + Given/When/Then
- [`templates/5_adr.md`](./templates/5_adr.md) — MADR v3.0 ADR with Y-statement (this project's 16 ADRs all use it)
- [`templates/6_persona.md`](./templates/6_persona.md) — Cooper goal-directed persona (UX research, optional)
- [`templates/7_journey-map.md`](./templates/7_journey-map.md) — Kalbach customer journey map (UX research, optional)
- [`templates/8_service-blueprint.md`](./templates/8_service-blueprint.md) — Bitner service blueprint (UX research, optional, cross-functional services)
