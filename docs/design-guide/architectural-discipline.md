---
status: Stable
owner: tensor
last-reviewed: 2026-05-12
---

# Architectural discipline — the dependency rule

This is the **operational convention** that turns [ADR-0009 (DDD ubiquitous language + Hexagonal lite)](../arc42/09-decisions/0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md) and [ADR-0011 (`KernelBackend` port API)](../arc42/09-decisions/0011-kernel-backend-port-api.md) from aspirations into something the build refuses to violate. Every contributor reads this before touching `include/`.

## The rule (one sentence)

**The Domain depends on no Adapter; Adapters depend on the Domain via concepts; Adapters do not depend on each other.**

In code:

- A header under `include/tensor/core/` (the Domain) **must not** `#include` anything from `include/tensor/{tex,autograd}/` or from `include/tensor/core/backend/<other_adapter>/`. The Domain may declare concepts in `include/tensor/core/concepts.hpp` that adapters then implement.
- A header under `include/tensor/{tex,autograd}/` or `include/tensor/core/backend/<adapter>/` (the Adapters) **may** `#include` from `include/tensor/core/`.
- No adapter header includes from another adapter directly. The `KernelBackend` adapters (`reference`, `eigen`, `webgpu`) may keep a private `reference::Backend ref_` member to delegate out-of-scope methods — this is implementation-level composition, not a header-include dependency.

## Why this exists

The educational pitch ([ADR-0001](../arc42/09-decisions/0001-pivot-to-educational-named-axis-dsl.md), refined by [ADR-0010](../arc42/09-decisions/0010-refine-positioning-to-educational-first-production-capable.md)) and the architectural-as-teaching-artifact stance ([ADR-0009](../arc42/09-decisions/0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md) + [ADR-0011](../arc42/09-decisions/0011-kernel-backend-port-api.md)) only pay off if the Hexagonal structure is *actually* respected by the code. A rule respected only by good intentions decays in days under contributor pressure. Having the rule enforced by CMake + clang-tidy + CI lets us claim the architectural-clarity-as-feature without having to police it manually. As of 2026-05-12 all three planned `KernelBackend` adapters ship: `reference` (canonical), `eigen` (SIMD + GEMM), and `webgpu` (Dawn — 12 of 15 methods dispatch real GPU compute on `float`).

## Container classification (today)

| Container          | Layer              | Ports it owns / implements                                               |
| ------------------ | ------------------ | ------------------------------------------------------------------------ |
| `tensor::core`     | Domain             | Owns: `TensorLike`, `Shape`, `Axis`, `KernelBackend`, `BufferExporter` concepts. |
| `tensor::autograd` | Domain (extension) | Owns: `Differentiable`, `BackwardOp`, `BackwardSink` concepts.            |
| `tensor::tex`      | DrivingAdapter     | Implements: `ExpressionSource`, `ExpressionSink`.                         |
| `tensor::core::backend::reference` | DrivenAdapter | Implements: `KernelBackend` (canonical).                            |
| `tensor::core::backend::eigen`     | DrivenAdapter | Implements: `KernelBackend` (Eigen 3.4 SIMD + GEMM; delegates out-of-scope methods to `reference`). |
| `tensor::core::backend::webgpu`    | DrivenAdapter | Implements: `KernelBackend` (Dawn via `webgpu_cpp.h`; delegates non-`float` and non-simple-GEMM and reduce / unbroadcast to `reference`). |
| `lyx-export/`      | DrivingAdapter     | Preprocesses to `tensor::tex` input via the `lyx_to_tex.py` translator; never touches Domain directly. |

`tutorials/` and the Jupyter Book site are **outside the hexagon** — they are demos and rendering artifacts, not architectural elements.

## Where ports live

- **Concepts file**: `include/tensor/<container>/concepts.hpp`. Contains C++20 `concept` definitions and (for type-erased ports) pure-abstract base classes. **Only** `concepts.hpp` of a Domain container is allowed to be included by another container.
- **Concrete adapter symbols**: `include/tensor/<container>/<adapter>.hpp`. Adapters consume Domain via `concepts.hpp`, never via concrete Domain types beyond the documented interop surfaces (e.g. `tensor::core::Shape`, which is itself a Domain type rather than an adapter type).

## Enforcement

Three layers, in order of trust:

1. **CMake target visibility.** `tensor` (the INTERFACE library) carries the Domain headers via `target_include_directories`; the adapter targets each carry their own subset. Misincludes surface as link / include errors at configure or build time.
2. **clang-tidy custom matcher** (planned). A `bugprone-include-direction` matcher enforces the include direction documented above. Any contributor PR that violates the rule fails clang-tidy in CI.
3. **CI grep fallback** (planned, redundant with 2). A simple `grep` step in `ci.yml` checks two patterns: (a) `git ls-files 'include/tensor/core/*.hpp' | xargs grep -l 'include "tensor/\(tex\|autograd\)/'` produces no output (Domain headers do not pull in DrivingAdapters); (b) `git ls-files 'include/tensor/core/backend/*/*.hpp' | xargs grep -l 'include "tensor/core/backend/\(?!<self>\)/'` produces no output (no adapter includes from a sibling adapter). `concepts.hpp` is allowed to forward-declare adapter-side concept refinements (it does not include adapter headers).

Until (2) and (3) are wired, contributors are expected to self-check; any review comment with the keyword **direction violation** halts the PR.

## Contributor checklist (paste into PR descriptions when touching `include/`)

```markdown
### Hexagonal discipline (ADR-0009 / ADR-0011)
- [ ] No header in `include/tensor/core/` (excluding `concepts.hpp` and
      `include/tensor/core/backend/<adapter>/`) includes from
      `include/tensor/{tex,autograd}/` or from another adapter.
- [ ] No header in `include/tensor/core/backend/<adapter>/` includes from
      another `backend/<adapter>/`.
- [ ] Any new container is classified `Domain` / `DrivingAdapter` /
      `DrivenAdapter` in `docs/arc42/05-building-blocks/overview.md` and
      mirrored in `docs/diagrams/c4/workspace.dsl`.
- [ ] Any new port is declared as a C++20 concept (or pure-abstract class
      where erasure is required) in the owning container's `concepts.hpp`.
- [ ] Any new adapter has `static_assert(KernelBackend<Backend>)` next to
      its definition (the conformance pattern documented in
      `docs/detailed-design/kernel-backend-port.md`).
- [ ] GoF patterns used for cross-cutting concerns are named in the
      adapter's detailed-design doc (Composite / Visitor / Strategy /
      Command / Bridge / etc.).
```

## When the rule is wrong

The rule above is intentionally narrow. If a real situation makes it impossible to comply, the right response is:

1. **Don't break the rule silently.** A bypass that "just for now" goes around the rule never gets cleaned up.
2. **Open a draft PR with the bypass** clearly marked, along with the constraint that prevents compliance.
3. **Either** restructure to fit the rule, **or** propose a successor ADR that supersedes the relevant part of ADR-0009.

ADRs in `Accepted` status are immutable; changing the discipline means a new ADR linking back, not editing the existing one.

## Cross-references

- [ADR-0009](../arc42/09-decisions/0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md) — the decision this guide enforces
- [arc42 §5 — building-blocks/overview.md](../arc42/05-building-blocks/overview.md) — current container classification
- [diagrams/c4/workspace.dsl](../diagrams/c4/workspace.dsl) — DSL tags reflect the same classification
- Alistair Cockburn, ["Hexagonal Architecture / Ports and Adapters"](https://alistair.cockburn.us/hexagonal-architecture/)
