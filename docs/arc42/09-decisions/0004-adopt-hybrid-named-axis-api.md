---
status: Accepted
owner: tensor
last-reviewed: 2026-05-10
---

# ADR-0004: Adopt a hybrid named-axis API (runtime axis identity + NTTP compile-time fast path)

| Metadata  | Value                                                          |
| --------- | -------------------------------------------------------------- |
| Status    | **Accepted**                                                   |
| Type      | ADR (Type 5 / MADR v3.0 / arc42 §9)                            |
| Date      | 2026-05-10                                                     |
| Deciders  | uyuutosa                                                       |
| Consulted | Claude (research brief §2)                                     |
| Informed  | future contributors                                            |
| Ticket    | —                                                              |

---

## Context and Problem Statement

The headline differentiator of the `tensor` library, per ADR-0001, is **named-axis tensor algebra**: tensors carry axis labels, and operations on tensors with different labels broadcast like `a_i + b_j → c_{ij}`. The unresolved design question is *where* axis identity lives:

- **Type-level (compile-time)**: axis labels are template parameters / NTTP string literals. The compiler enforces compatibility. This is what e.g. [Tenseur](https://istmarc.github.io/post/2024/10/27/on-designing-tenseur-a-c-tensor-library-with-lazy-evaluation/) and many academic Einstein-notation libraries do.
- **Value-level (runtime)**: axis labels are runtime-checked strings. This is what [PyTorch named tensors](https://docs.pytorch.org/docs/stable/named_tensor.html) does.

Both have known failure modes:

- **Pure type-level** is elegant for fixed-shape kernels but explodes the type system for dynamic / data-dependent shapes (PyTorch named tensors stayed "prototype" for six years partly for this reason).
- **Pure runtime** loses the compile-time error path that makes named axes pedagogically powerful ("the compiler caught your mismatched axes").

We must pick one model — or design a hybrid — before any API surface is committed.

---

## Decision Drivers

- **DD-1**: Pedagogical clarity. Mismatched axes should fail with a useful message; type-level wins this.
- **DD-2**: Ergonomics for dynamic shapes. Notebooks, exploratory work, and TeX-derived expressions (ADR-0005) sometimes carry runtime labels; runtime wins this.
- **DD-3**: Compile-time error quality. Hard for any approach in C++; mitigated by C++20 concepts.
- **DD-4**: Interop with `mdspan`. `mdspan` is index-based; the API must round-trip through it.

---

## Considered Options

1. **Pure type-level** — axis labels as NTTPs; fully compile-time.
2. **Pure runtime** — axis labels as `std::string` / `std::string_view`; checked at runtime.
3. **Hybrid: runtime axis identity + NTTP compile-time fast path** — labels are values, but a `consteval` literal (`"i"_ax` or similar) lifts a label into a type so that compile-time-known operations get static checking, while runtime labels still work.

---

## Decision Outcome

**Chosen option: 3 — Hybrid. Axis identity is a value type; an NTTP-bearing wrapper provides a compile-time fast path.**

Concretely:

- Every tensor carries a `Shape` whose axes are `(label, extent)` pairs. `label` is a `std::string_view` at runtime, with a small-string-optimized internal representation.
- A literal `"i"_ax` (NTTP string-literal user-defined literal in C++20) yields a `LabelTag<"i">` whose type encodes the label. Operations parameterized by `LabelTag` get compile-time axis-mismatch errors.
- Operations parameterized by runtime labels do the equivalent runtime check (and report through `expected<...>` or throw).
- The user opts in to the compile-time path by writing `"i"_ax` instead of `"i"`.

This pattern is similar to the way `std::format` accepts both compile-time format strings (with [P2216 compile-time checks](https://wg21.link/P2216)) and runtime ones — a precedent from the standard library.

### Y-statement summary

> In the context of **designing the named-axis API for the rewritten `tensor` library**, facing **the trade-off between compile-time axis safety and runtime ergonomics**, we decided for **a hybrid: runtime axis identity with an NTTP compile-time fast path opted into via a `_ax` user-defined literal**, to achieve **pedagogical compile-time errors when the user wants them and runtime flexibility when they don't**, accepting **two parallel API entry points that must be kept in sync**.

---

## Pros and Cons of the Options

### Option 1: Pure type-level

- Pros: maximum compile-time safety; elegant for kernel-style code.
- Cons: type-system explosion for dynamic shapes; poor fit for notebooks; PyTorch / JAX experience suggests this is a long-term maintenance trap.

### Option 2: Pure runtime

- Pros: simple, ergonomic, works in dynamic / scripting contexts.
- Cons: surrenders the pedagogical "compiler caught your mistake" moment that justifies named-axis-in-C++ in the first place.

### Option 3: Hybrid (chosen)

- Pros: covers both worlds; users self-select; precedent in `std::format`; matches the educational pitch.
- Cons: two parallel paths; doc burden; small risk of subtly diverging semantics between paths (the implementation must keep them unified).

---

## Consequences

### Positive

- The teaching narrative becomes: "write `a("i") + b("j")` to explore in a notebook; write `a("i"_ax) + b("j"_ax)` to lock the axes at compile time when shipping a kernel."
- Compile-time-ness composes with `consteval` shape arithmetic — pedagogical gold for showing how modern C++ moves work to compile time.
- Runtime path ensures TeX/LyX-derived expressions (ADR-0005) just work without code generation.

### Negative

- The implementation must avoid bifurcating: every operator should have one underlying definition with two thin wrappers.
- Documentation must explain *both* paths and when to use which.

### Neutral

- A future ADR could deprecate the runtime path if compile-time ergonomics improve enough; but PyTorch / JAX history says this would be premature.

### Follow-ups

- [ ] Detailed design doc under `docs/detailed-design/01_named-axis-api.md`.
- [ ] Spike: `consteval` parser for an einsum-style string `"ij,jk->ik"` resolving to `LabelTag<...>`-based contractions.
- [ ] Spike: error-message quality (concepts + `static_assert` with `std::format` messages on C++20).

---

## Compliance / Validation

- Verification: every binary operator on `Tensor` has both type-level and value-level overloads, and a single shared kernel; tests cover both paths.
- Frequency: every PR (CI test).

---

## More Information

### Related ADRs

- Predecessors: [ADR-0001](0001-pivot-to-educational-named-axis-dsl.md), [ADR-0002](0002-rewrite-on-cpp20-baseline-with-mdspan-interop.md)
- TeX/LyX surface (consumes both paths): [ADR-0005](0005-adopt-tex-lyx-as-authoring-surface.md)
- Autograd subsystem: [ADR-0007](0007-adopt-autograd-as-first-class-subsystem.md)

### References

- [PyTorch Named Tensors operator coverage docs](https://docs.pytorch.org/docs/stable/name_inference.html) — failure mode of pure type-level
- [State of Torch Named Tensors issue](https://github.com/pytorch/pytorch/issues/60832)
- [JAX `xmap` deletion discussion](https://github.com/jax-ml/jax/discussions/20312) — failure mode of pure-research named-axis
- [Tenseur design notes](https://istmarc.github.io/post/2024/10/27/on-designing-tenseur-a-c-tensor-library-with-lazy-evaluation/)
- [Einsums C++ library](https://einsums.github.io/Einsums/)
- [Tensor Considered Harmful](https://nlp.seas.harvard.edu/NamedTensor)
- P2216 — `std::format` compile-time checks (precedent for hybrid pattern)
