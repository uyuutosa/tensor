---
status: Accepted
owner: tensor
last-reviewed: 2026-05-10
---

# ADR-0009: Adopt DDD ubiquitous language and Hexagonal (Ports & Adapters) "lite" as the structural principle

| Metadata  | Value                                                          |
| --------- | -------------------------------------------------------------- |
| Status    | **Accepted**                                                   |
| Type      | ADR (Type 5 / MADR v3.0 / arc42 §9)                            |
| Date      | 2026-05-10                                                     |
| Deciders  | uyuutosa                                                       |
| Consulted | Claude (V1/V2/V3 design discussion, 2026-05-10 session)        |
| Informed  | future contributors                                            |
| Ticket    | —                                                              |

---

## Context and Problem Statement

ADR-0001..0008 specified *what* containers exist (`tensor::core`, `tensor::tex`, `tensor::autograd`, `tensor::gpu`, `tutorials/`, Jupyter Book site, optional `lyx-export/`) and what each container does. They did not specify *how* the containers relate, *which way dependencies must flow*, or *what naming discipline* governs the codebase.

For an educational library, the absence of such structural discipline becomes a positive signal — "this library is hackable but undisciplined" is still a usable pitch. But the project has a stronger pitch available: the headline feature (named-axis tensor algebra, ADR-0004) and the `_tex` authoring surface (ADR-0005) are both about making *math = code*. The natural extension of that pitch is to make the *architecture* itself a teaching artifact: a working hexagonal codebase in modern C++ that learners can read end-to-end.

We must pick a structural principle that (a) realises the math-as-code pitch at the architectural layer, (b) does not balloon the file tree to the point where readability suffers, and (c) is enforceable by tooling rather than by good intentions.

---

## Decision Drivers

- **DD-1**: Pedagogical clarity at the architectural layer. The structure should *be* a teaching example, not just *contain* teaching examples.
- **DD-2**: Ubiquitous language alignment. Domain terms (`Axis`, `Shape`, `Contraction`, `Broadcast`, `Variable`, `Backward`) are mathematical terms; code names must match without paraphrase.
- **DD-3**: Small-codebase readability. micrograd / tinygrad / Fastor have validated that flat, browsable layouts beat enterprise-grade scaffolding for an educational artifact.
- **DD-4**: Enforceability. A naming convention or layering rule that is not checked by tooling decays under contributor pressure.
- **DD-5**: Future flexibility. Swapping CPU↔WebGPU backends, plugging in Python bindings, or accepting a LyX adapter must not require domain edits.

---

## Considered Options

1. **Flat / unstructured** — keep ADR-0001..0008's container list with no further structural rules.
2. **DDD bounded contexts only (V2)** — enforce ubiquitous language and one namespace per bounded context (`tensor::core`, `tensor::autograd`, `tensor::gpu`, `tensor::tex`); no explicit Port/Adapter discipline, no enforced dependency direction.
3. **Hexagonal "lite" — DDD ubiquitous language + Ports & Adapters at the *conceptual* layer with a flat file tree (V3)** — domain core depends on no adapter; adapters depend on domain via concept-based ports; ports live as `concepts.hpp` files inside each container, not under a `ports/` directory.
4. **Full Hexagonal (V1)** — top-level `include/tensor/{domain,ports,adapters}/` directories, classical abstract base classes for ports.
5. **Onion / Clean architecture** — variant of (4) with concentric layers (entities / use cases / interface adapters / frameworks).

---

## Decision Outcome

**Chosen option: 3 — Hexagonal "lite". Apply Hexagonal as a *mental model and an enforced dependency-direction rule*; keep the file tree flat (no `ports/` or `adapters/` directories).**

Concretely:

- **Each container is classified as one of**: `domain` (the hexagon centre), `driving-adapter` (input port, "north"), or `driven-adapter` (output port, "south"). The classification is recorded in [`docs/arc42/05-building-blocks/overview.md`](../05-building-blocks/overview.md).
- **Ports are C++20 `concept`s** (or pure abstract classes where erasure is required), declared inside each domain container as `concepts.hpp`.
- **Dependency rule (the only hard rule)**: domain headers must not include adapter headers; driving-adapter headers may include domain headers; driven-adapter headers may include domain headers. **No header may depend on another adapter directly** (adapters meet only at the domain layer).
- **Enforcement**: `target_link_libraries` in CMake plus a clang-tidy custom check (or a CI grep) catch violations of the rule.
- **Naming**: namespaces match domain language (`tensor::core::Axis`, `tensor::core::Shape`, `tensor::autograd::Variable`, `tensor::gpu::WgslKernel`, `tensor::tex::Expression`). No abbreviations, no synonyms.
- **GoF patterns are named where they apply** (Composite for expression trees, Visitor for AST traversal, Strategy for backend selection, Command for autograd tape). This is *naming discipline*, not new abstractions: pattern names appear in design-guide and detailed-design docs so learners can connect the code to the textbook vocabulary.

### Y-statement summary

> In the context of **structuring the rewritten `tensor` C++ library beyond the per-container decisions in ADR-0001..0008**, facing **the choice between a flat layout and full Hexagonal scaffolding**, we decided for **DDD ubiquitous language + Hexagonal (Ports & Adapters) "lite": one namespace per bounded context, ports as `concepts.hpp`, an enforced dependency rule, and named GoF patterns where they apply**, to achieve **a codebase whose architecture is itself a teaching artifact and whose layering is enforced by the compiler and CI**, accepting **a slight cost in indirection at the kernel layer compared to a fully flat, fused codebase**.

---

## Pros and Cons of the Options

### Option 1: Flat / unstructured

- Pros: zero overhead; lowest file count.
- Cons: no enforced layering; future GPU adapter creep into domain code is unavoidable; loses the architectural-teaching angle.

### Option 2: DDD bounded contexts only (V2)

- Pros: keeps ubiquitous language discipline; minimal extra structure.
- Cons: no enforced dependency direction → in practice, GPU and TeX would creep into core; the *whole point* of the architectural-teaching pitch is lost.

### Option 3: Hexagonal "lite" (V3, chosen)

- Pros: enforceable dependency direction; teaches Hexagonal in 200 lines of `concepts.hpp` rather than 2,000 lines of base classes; keeps `include/` browsable; pairs naturally with C++20 concepts; matches micrograd/tinygrad-shaped readability while adding architectural backbone.
- Cons: contributors must read the design-guide before adding adapters; the dependency rule must be CI-enforced or it decays.

### Option 4: Full Hexagonal (V1)

- Pros: maximum structural rigor; indistinguishable from enterprise-app reference architecture.
- Cons: enterprise-app smell on a numerical lib; doubles the file count; obscures the *math* with abstract-class ceremony; rejected on educational-readability grounds.

### Option 5: Onion / Clean architecture

- Pros: similar benefits to (4) with explicit concentric layers.
- Cons: same readability cost as (4); the additional layers (use cases vs. entities) don't map cleanly to a numerical library — *use cases* in tensor algebra are barely distinct from *entities*. Rejected.

---

## Consequences

### Positive

- The §5 building-block view becomes a literal hexagon with named ports; the C4 L2 diagram tags each container as `Domain` / `DrivingAdapter` / `DrivenAdapter`.
- Future adapters (Python bindings, ONNX exporter, an alternative kernel backend) plug into the existing ports without touching domain.
- Detailed-design docs gain a standard shape: each adapter doc names the port it implements and the GoF pattern (if any) it instantiates.
- A self-contained tutorial chapter ("the architecture of `tensor`") becomes possible — the project is *a hexagonal C++ library you can read in an afternoon*.

### Negative

- Domain code must now go through concepts at backend boundaries (slight cost in compile-time error messages and template depth). Mitigation: keep concept definitions co-located with the domain types they constrain.
- One extra ADR for contributors to internalize.
- Tooling enforcement (CMake target visibility + clang-tidy / CI grep) must be set up; without it the rule rots.

### Neutral

- The existing 8 ADRs are unaffected; this ADR adds *structural discipline*, it does not contradict them.
- The phase-by-phase introduction of containers ([impl-plan Phase 1](../../impl-plans/2026-05-10_revival-phase-1.md) and successors) is unchanged; each container will be classified at introduction time.

### Follow-ups

- [ ] Write [`docs/design-guide/architectural-discipline.md`](../../design-guide/architectural-discipline.md) with the dependency rule and contributor checklist.
- [ ] Update [`docs/arc42/05-building-blocks/overview.md`](../05-building-blocks/overview.md) with hexagonal layer classification per container and a hexagon ASCII diagram.
- [ ] Update [`docs/arc42/04-solution-strategy/strategy.md`](../04-solution-strategy/strategy.md) navigation index to surface this ADR.
- [ ] Update [`docs/diagrams/c4/workspace.dsl`](../../diagrams/c4/workspace.dsl) container tags to `Domain` / `DrivingAdapter` / `DrivenAdapter` for explicit hexagonal styling.
- [ ] At M2 (core types), introduce `include/tensor/core/concepts.hpp` with the first port (the `TensorLike` concept) as the visible enforcement point.
- [ ] At M3+ (kernel work) and ADR-0006 (WebGPU) follow-up, the WGSL backend implements a `KernelBackend` port declared in `tensor::core::concepts`.
- [ ] CI-time enforcement: add a check (clang-tidy custom matcher or CI grep) for include-direction violations.

---

## Compliance / Validation

- **Verification**: a CI step rejects PRs where a domain header (anything under `include/tensor/core/` not in `concepts.hpp`) includes an adapter header (anything under `include/tensor/{gpu,tex,autograd}/`).
- **Frequency**: every PR.
- **Manual check**: the design-guide contributor checklist must be ticked in PR descriptions for any change touching `include/`.

---

## More Information

### Hexagonal classification of existing containers

Cross-referenced from [`../05-building-blocks/overview.md`](../05-building-blocks/overview.md):

| Container          | Classification     | Port(s) realised                                       |
| ------------------ | ------------------ | ------------------------------------------------------ |
| `tensor::core`     | Domain             | declares `TensorLike`, `Shape`, `Axis` concepts        |
| `tensor::autograd` | Domain (extension) | declares `Differentiable`, `BackwardOp` concepts       |
| `tensor::tex`      | Driving adapter    | implements an inbound `ExpressionSource` port          |
| (future) Python bindings | Driving adapter | implements `ExpressionSource`                          |
| (future) LyX module | Driving adapter   | preprocesses to `tensor::tex` input                    |
| `tensor::gpu`      | Driven adapter     | implements `KernelBackend` port (WGSL codegen + dispatch) |
| (CPU executor inside `tensor::core`) | Driven adapter | implements `KernelBackend` port (reference impl) |
| `mdspan` interop   | Driven adapter     | implements `BufferExporter` port                       |
| LaTeX output renderer | Driven adapter  | implements `ExpressionSink` port                       |
| Tape writer (autograd) | Driven adapter | implements `BackwardSink` port                         |

### Related ADRs

- Foundational: [ADR-0001](0001-pivot-to-educational-named-axis-dsl.md)
- Substrate: [ADR-0002](0002-rewrite-on-cpp20-baseline-with-mdspan-interop.md), [ADR-0003](0003-replace-eclipse-cdt-with-cmake-and-vcpkg.md)
- Container-level decisions touched by the dependency rule: [ADR-0004](0004-adopt-hybrid-named-axis-api.md), [ADR-0005](0005-adopt-tex-lyx-as-authoring-surface.md), [ADR-0006](0006-adopt-webgpu-as-gpu-backend.md), [ADR-0007](0007-adopt-autograd-as-first-class-subsystem.md)

### References

- Alistair Cockburn, ["Hexagonal architecture / Ports and Adapters"](https://alistair.cockburn.us/hexagonal-architecture/)
- Eric Evans, *Domain-Driven Design* (2003) — chapters on Ubiquitous Language and Bounded Contexts
- Vaughn Vernon, *Implementing Domain-Driven Design* (2013)
- "Gang of Four", *Design Patterns* (1995) — Composite, Visitor, Strategy, Command, Bridge as named cross-cutting mechanisms
- [Tom Stuart, *Understanding Computation*](https://computationbook.com/) — pedagogical precedent for "the architecture is the lesson"
- [tinygrad source layout](https://github.com/tinygrad/tinygrad) and [micrograd](https://github.com/karpathy/micrograd) — readability benchmarks
