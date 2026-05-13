---
status: Stable
owner: tensor
last-reviewed: 2026-05-04
---

# detailed-design — per-module implementation specs (HOW)

> **Use Template 3** ([`../templates/3_module-detailed-design.md`](../templates/3_module-detailed-design.md)) for everything in this directory.

This directory holds the implementation HOW for each module. arc42 §5 (`../arc42/05-building-blocks/`) is a navigation index that points here; the actual data models, API contracts, error handling, and design intent live in the per-module files below.

## File layout

```text
detailed-design/
├── README.md                           # this file
├── <module-1>.md                       # Template 3 instance per module
├── <module-2>.md
└── <subsystem>/                        # group related modules in subdirectories if it grows large
    ├── <module-3>.md
    └── <module-4>.md
```

## Naming

`<module-name>.md` in lower-kebab-case, matching the building-block name in `arc42/05-building-blocks/` and the element name in `diagrams/c4/workspace.dsl`. Synonyms across these three locations are bugs.

## Cross-references from each module file

Each module file should link out to:

- The PRD that motivated it (`arc42/03-context-and-scope/prds/<file>.md`)
- The ADRs that shaped it (`arc42/09-decisions/NNNN-...md`)
- The crosscutting concerns that apply to it (`arc42/08-crosscutting/<concern>.md`)
- The API contract (`api-contract/<module>.md`)
- The use cases that exercise it (`arc42/03-context-and-scope/use-cases/<file>.md`)

For lifecycle / when to update, see [`../WORKFLOW.md`](../WORKFLOW.md).

## Index — instances in this project (8 as of 2026-05-14)

| File                                                                                 | Module / scope                                                                                                | Hex. classification | Anchor ADR(s)        |
| ------------------------------------------------------------------------------------ | -------------------------------------------------------------------------------------------------------------- | ------------------- | -------------------- |
| [`tensor-core.md`](./tensor-core.md)                                                  | `tensor::core` Domain centerpiece — `Axis`, `Shape<N>`, `DynamicShape`, `Tensor<T,N>`, `DynamicTensor<T>`, `TypedTensor`, broadcast / contract Domain functions, the `KernelBackend` port declaration. | Domain               | 0002, 0004, 0009, 0011 |
| [`tensor-autograd.md`](./tensor-autograd.md)                                          | `tensor::autograd` — `Variable<T,N>`, `DynamicVariable<T>`, Tape, registered backwards, activations, `gradient_check`, Bundle B extensions. | Domain extension     | 0007, 0009            |
| [`tensor-tex.md`](./tensor-tex.md)                                                    | `tensor::tex` — LaTeX-subset parser, `Expression` AST, `Evaluator<T>`, `to_latex` renderer, the `_tex` UDL. | DrivingAdapter       | 0005, 0009, 0015 (was 0013) |
| [`kernel-backend-port.md`](./kernel-backend-port.md)                                  | The 15-method `tensor::core::concepts::KernelBackend` port surface itself — the central Hexagonal port. | Port (in Domain)     | 0009, 0010, 0011, 0012, 0016 |
| [`webgpu-element-wise-kernels.md`](./webgpu-element-wise-kernels.md)                  | WebGPU adapter — 4 binary + 4 unary element-wise WGSL kernels and their dispatch. | DrivenAdapter        | 0006, 0011, 0012, 0014, 0016 |
| [`webgpu-gemm-kernel.md`](./webgpu-gemm-kernel.md)                                    | WebGPU adapter — tiled GEMM WGSL kernel and its dispatch. | DrivenAdapter        | 0006, 0011, 0012, 0014, 0016 |
| [`webgpu-broadcast-kernels.md`](./webgpu-broadcast-kernels.md)                        | WebGPU adapter — broadcast WGSL kernels (the `c = a + b` GPU path for shape-mismatched inputs). | DrivenAdapter        | 0006, 0011, 0012, 0016 |
| [`python-sdk-binding-surface.md`](./python-sdk-binding-surface.md)                    | Python SDK via nanobind — the boundary conventions (the four papercuts + Phase 6.5 forward-anchor for the PEP-420 + lazy-import patterns). | DrivingAdapter       | 0018, 0019            |

Naming verification (per the §"Naming" rule above): every file maps 1:1 to a row in [`../arc42/05-building-blocks/overview.md`](../arc42/05-building-blocks/overview.md) and a container in [`../diagrams/c4/workspace.dsl`](../diagrams/c4/workspace.dsl). Synonyms across these three locations are bugs.
