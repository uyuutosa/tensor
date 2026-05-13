---
status: Stable
owner: tensor
last-reviewed: 2026-05-14
---

# api-contract — public symbol contracts

> **Use Template 0** ([`../templates/0_default.md`](../templates/0_default.md)) for new files. The project has no network surface, so the contracts here pin **public symbol vocabulary**, not REST / GraphQL endpoints.

## Layout

```text
api-contract/
├── README.md                                # this file
├── python-public-surface.md                  # tensor.* (Python SDK)
├── cpp-tensor-core-surface.md                # tensor::core
├── cpp-tensor-autograd-surface.md            # tensor::autograd
├── cpp-tensor-tex-surface.md                 # tensor::tex
├── cpp-kernel-backend-port-surface.md        # tensor::core::concepts::KernelBackend port
└── cpp-tensor-namespace-summary.md           # 4-namespace quick lookup
```

## What this directory is for

`tensor` is a header-only C++ library + a Python SDK. There is no network surface; what would normally be the API contract is the public *symbol* surface. Each file in this directory pins:

- Every public type / function / concept / UDL.
- The introducing version (or PR).
- The header / module that defines it.
- A pointer to the per-module detailed design.

The test surface enforces the pinning: `python/tests/test_smoke.py::test_public_surface_is_minimal` asserts exactly the symbols listed in [`./python-public-surface.md` §2-§4](./python-public-surface.md). The C++ side has analogous concept-conformance + cross-validation tests per `tests/test_*.cpp`.

## Why two parallel surfaces?

Phase 6 added the Python SDK as a *DrivingAdapter* over the same C++ Domain. Every Python entry point routes into the same `tensor::core` / `tensor::autograd` / `tensor::tex` headers — nothing is reimplemented in Python. The two surfaces have *parallel* contracts, not duplicate ones; numerical agreement is the QO-4 quality scenario in [`../arc42/10-quality/overview.md`](../arc42/10-quality/overview.md).

## Rules

1. **Symbol is the source of truth.** The contract pins symbol *vocabulary* (name + signature + introducing version); the symbol's *behaviour* lives in the corresponding detailed-design.
2. **Cross-link from the corresponding `../detailed-design/<module>.md` §6 Testing strategy** so the test surface is discoverable from both directions.
3. **Every new public symbol gets a row here in the same PR that adds it.** The smoke test fails otherwise.

For lifecycle / when to update, see [`../WORKFLOW.md`](../WORKFLOW.md).

## Cross-references

- [`../templates/0_default.md`](../templates/0_default.md) — the base template.
- [`../detailed-design/`](../detailed-design/) — the HOW for each surface.
- [`../user-manual/reference/`](../user-manual/reference/) — the human-readable face of these contracts.
- [`../arc42/12-glossary/overview.md`](../arc42/12-glossary/overview.md) — vocabulary anchors.
