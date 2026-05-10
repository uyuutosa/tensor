---
status: Accepted
owner: tensor
last-reviewed: 2026-05-10
---

# ADR-0007: Adopt automatic differentiation as a first-class subsystem

| Metadata  | Value                                                          |
| --------- | -------------------------------------------------------------- |
| Status    | **Accepted**                                                   |
| Type      | ADR (Type 5 / MADR v3.0 / arc42 §9)                            |
| Date      | 2026-05-10                                                     |
| Deciders  | uyuutosa                                                       |
| Consulted | Claude (research brief §5)                                     |
| Informed  | future contributors                                            |
| Ticket    | —                                                              |

---

## Context and Problem Statement

A pure tensor algebra DSL and a tinygrad-class teaching ML framework are both defensible niches for an educational C++ tensor library (research brief §5, D-9). With autograd, the library teaches reverse-mode automatic differentiation alongside named-axis tensor algebra; without it, the library is a smaller, tighter algebra DSL.

The maintainer has chosen to add autograd. This ADR records that choice, picks the autograd model (tape-based vs. expression-template-based vs. operator-overloading-based), and bounds the scope.

---

## Decision Drivers

- **DD-1**: Pedagogical scope. The library should teach *one* clear autograd model, not survey several.
- **DD-2**: Composition with named-axis API (ADR-0004). The autograd graph must respect axis labels.
- **DD-3**: Implementation cost. Solo project; the autograd implementation must fit in the maintainer's bandwidth.
- **DD-4**: Production aspirations are explicitly out of scope (ADR-0001). Autograd here is a teaching artifact, not a competitor to PyTorch.

---

## Considered Options

1. **No autograd** — pure tensor algebra DSL.
2. **Tape-based reverse-mode autograd** (PyTorch / [tinygrad](https://github.com/tinygrad/tinygrad) / [micrograd](https://github.com/karpathy/micrograd)).
3. **Expression-template forward-mode autograd** (compile-time, e.g. [Stan](https://mc-stan.org/) style).
4. **Source-to-source / compiler-driven autograd** (Enzyme, Tapenade).

---

## Decision Outcome

**Chosen option: 2 — Tape-based reverse-mode autograd, modeled after micrograd / tinygrad in shape but typed against named-axis tensors.**

Reverse-mode is the right teaching model because it matches what every modern deep learning framework does, makes a clear pedagogical contrast with forward-mode, and pairs naturally with named-axis tensors (the gradient of a named-axis op preserves axis labels). A small, hand-rolled tape implementation (one `Tape` object, one `Variable` wrapper, registered backward functions per op) is bounded scope and the canonical educational pattern.

Forward-mode (option 3) and source-to-source (option 4) are interesting but the cost-vs-pedagogical-gain ratio is wrong for this project at this scale. They can be added in later as alternative backends if the project grows; this ADR does not foreclose them.

### Y-statement summary

> In the context of **extending the rewritten `tensor` library beyond pure tensor algebra**, facing **the choice of whether and how to add automatic differentiation**, we decided for **tape-based reverse-mode autograd modeled after micrograd / tinygrad and typed against named-axis tensors**, to achieve **a tinygrad-class teaching ML framework grounded in named-axis semantics**, accepting **that this expands scope significantly relative to a pure DSL**.

---

## Pros and Cons of the Options

### Option 1: No autograd

- Pros: smallest scope; library stays a tight tensor algebra DSL.
- Cons: forfeits the entire "build your own ML framework" educational genre that micrograd / tinygrad have validated as healthy.

### Option 2: Tape-based reverse-mode (chosen)

- Pros: matches what learners already half-know from PyTorch; canonical 100-line micrograd pattern is a known good starting point; named-axis labels propagate naturally through backward functions.
- Cons: tape allocation is a runtime concern; heap pressure on small ops; thread-safety has to be handled.

### Option 3: Expression-template forward-mode

- Pros: compile-time elegance; zero runtime tape allocation.
- Cons: scales poorly with output count; not the model learners are looking for.

### Option 4: Source-to-source / compiler-driven

- Pros: best perf, no runtime tape.
- Cons: requires an LLVM / clang plugin (Enzyme) or external tool; defeats the educational "you can read the implementation" property.

---

## Consequences

### Positive

- The library covers the *named-axis tensor algebra → autograd → small NN training* arc in one teaching artifact.
- A "build your own PyTorch in C++20" notebook becomes feasible end-to-end.
- WebGPU kernels (ADR-0006) and autograd compose: forward kernels produce activations on GPU, backward kernels compute gradients; the tape lives on the host.

### Negative

- Significant code-volume add (tape, Variable wrapper, backward functions for every primitive op, gradient testing harness).
- Performance ceiling for autograd is firmly below libtorch; README must say so.
- Memory-management story (when does the tape get freed?) needs careful documentation.

### Neutral

- Forward-mode and source-to-source remain reachable as future complementary backends.

### Follow-ups

- [ ] Detailed design `docs/detailed-design/03_autograd-tape.md`.
- [ ] Spike: minimal `Variable<Tensor>` + tape with gradient-checking harness against finite differences.
- [ ] Tutorial notebook: implement a small MLP on MNIST using only this library.
- [ ] Decide tape allocator strategy (arena vs. shared_ptr vs. pool).

---

## Compliance / Validation

- Verification: gradient-checking harness compares analytical gradients against numerical finite-difference gradients for every primitive op, within tolerance.
- Frequency: every PR.

---

## More Information

### Related ADRs

- Predecessors: [ADR-0001](0001-pivot-to-educational-named-axis-dsl.md), [ADR-0004](0004-adopt-hybrid-named-axis-api.md), [ADR-0006](0006-adopt-webgpu-as-gpu-backend.md)
- Distribution: [ADR-0008](0008-distribute-as-header-only-with-jupyter-tutorials.md)

### References

- [karpathy/micrograd](https://github.com/karpathy/micrograd) — canonical 100-line autograd
- [tinygrad](https://github.com/tinygrad/tinygrad) — minimal-ops production-grade autograd
- [teenygrad](https://github.com/j4orz/teenygrad) — micrograd → tinygrad bridge for didactic comparison
- ["Recreating PyTorch from Scratch" tutorial](https://medium.com/data-science/recreating-pytorch-from-scratch-with-gpu-support-and-automatic-differentiation-8f565122a3cc)
- [Enzyme.jl / Enzyme C++](https://enzyme.mit.edu/) — reference for the source-to-source path
- Research brief: `docs/reports/2026-05-10_tensor-revival-landscape.md` §5
