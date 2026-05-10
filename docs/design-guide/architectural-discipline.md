---
status: Stable
owner: tensor
last-reviewed: 2026-05-10
---

# Architectural discipline — the dependency rule

This is the **operational convention** that turns [ADR-0009 (DDD ubiquitous language + Hexagonal lite)](../arc42/09-decisions/0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md) from an aspiration into something the build refuses to violate. Every contributor reads this before touching `include/`.

## The rule (one sentence)

**The Domain depends on no Adapter; Adapters depend on the Domain via concepts; Adapters do not depend on each other.**

In code:

- A header under `include/tensor/core/` (the Domain) **must not** `#include` anything from `include/tensor/{tex,gpu,autograd}/` *except* concept declarations the Domain itself owns. The exception narrows to concrete adapter symbols, which Domain may never see.
- A header under `include/tensor/{tex,gpu,autograd}/` (Adapters) **may** `#include` from `include/tensor/core/`.
- A header under `include/tensor/tex/` **must not** `#include` from `include/tensor/gpu/`, and vice versa. If two adapters need to coordinate, they meet at the Domain layer.

## Why this exists

The educational pitch (ADR-0001) and the architectural-as-teaching-artifact stance (ADR-0009) only pay off if the Hexagonal structure is *actually* respected by the code. A rule respected only by good intentions decays in days under contributor pressure. Having the rule enforced by CMake + clang-tidy + CI lets us claim the architectural-clarity-as-feature without having to police it manually.

## Container classification (today)

| Container          | Layer              | Ports it owns / implements                                               |
| ------------------ | ------------------ | ------------------------------------------------------------------------ |
| `tensor::core`     | Domain             | Owns: `TensorLike`, `Shape`, `Axis`, `KernelBackend`, `BufferExporter` concepts. Implements: CPU reference `KernelBackend` (lives inside `core`). |
| `tensor::autograd` | Domain (extension) | Owns: `Differentiable`, `BackwardOp`, `BackwardSink` concepts.            |
| `tensor::tex`      | DrivingAdapter     | Implements: `ExpressionSource`, `ExpressionSink`.                         |
| `tensor::gpu`      | DrivenAdapter      | Implements: `KernelBackend`.                                              |
| `lyx-export/`      | DrivingAdapter (Phase 3+) | Preprocesses to `tensor::tex` input; never touches Domain directly. |

`tutorials/` and the Jupyter Book site are **outside the hexagon** — they are demos and rendering artifacts, not architectural elements.

## Where ports live

- **Concepts file**: `include/tensor/<container>/concepts.hpp`. Contains C++20 `concept` definitions and (for type-erased ports) pure-abstract base classes. **Only** `concepts.hpp` of a Domain container is allowed to be included by another container.
- **Concrete adapter symbols**: `include/tensor/<container>/<adapter>.hpp`. Adapters consume Domain via `concepts.hpp`, never via concrete Domain types beyond the documented interop surfaces (e.g. `tensor::core::Shape`, which is itself a Domain type rather than an adapter type).

## Enforcement

Three layers, in order of trust:

1. **CMake target visibility.** `tensor` (the INTERFACE library) carries the Domain headers via `target_include_directories`; the adapter targets each carry their own subset. Misincludes surface as link / include errors at configure or build time.
2. **clang-tidy custom matcher** (planned). A `bugprone-include-direction` matcher enforces the include direction documented above. Any contributor PR that violates the rule fails clang-tidy in CI.
3. **CI grep fallback** (planned, redundant with 2). A simple `grep` step in `ci.yml` checks: `git ls-files 'include/tensor/core/**.hpp' | xargs grep -l 'include "tensor/\(gpu\|tex\|autograd\)/'` produces no output, except for `concepts.hpp` itself which is allowed to forward-declare adapter-side concept refinements (it does not include adapter headers).

Until (2) and (3) are wired, contributors are expected to self-check; any review comment with the keyword **direction violation** halts the PR.

## Contributor checklist (paste into PR descriptions when touching `include/`)

```markdown
### Hexagonal discipline (ADR-0009)
- [ ] No header in `include/tensor/core/` (excluding `concepts.hpp`) includes
      from `include/tensor/{tex,gpu,autograd}/`.
- [ ] No header in `include/tensor/<adapter>/` includes from any other adapter.
- [ ] Any new container is classified `Domain` / `DrivingAdapter` /
      `DrivenAdapter` in `docs/arc42/05-building-blocks/overview.md`.
- [ ] Any new port is declared as a C++20 concept (or pure-abstract class
      where erasure is required) in the owning container's `concepts.hpp`.
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
