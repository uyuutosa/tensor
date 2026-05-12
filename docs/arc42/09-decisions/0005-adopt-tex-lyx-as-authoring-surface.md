---
status: Accepted
owner: tensor
last-reviewed: 2026-05-10
---

# ADR-0005: Adopt TeX / LyX as a first-class authoring surface for tensor expressions

| Metadata  | Value                                                          |
| --------- | -------------------------------------------------------------- |
| Status    | **Accepted**                                                   |
| Type      | ADR (Type 5 / MADR v3.0 / arc42 §9)                            |
| Date      | 2026-05-10                                                     |
| Deciders  | uyuutosa                                                       |
| Consulted | —                                                              |
| Informed  | future contributors                                            |
| Ticket    | —                                                              |

---

## Context and Problem Statement

The original aspiration of `tensor` — captured in the [author's 2016 blog post on convolutions-as-tensor-inner-products](http://qiita.com/uyuutosa/items/12e87f4695bd151b1d74) and the existing README — is "write code the way you write the math". A tensor expression like `a_i + b_j → c_{ij}` should look the same in code as on paper.

In 2026 there are two practical bridges between mathematical notation and C++ tensor code:

- **Output direction (code → LaTeX)**: pretty-print tensor expressions as LaTeX in Jupyter notebooks. This is essentially free with [xeus-cling](https://github.com/jupyter-xeus/xeus-cling) and a `_repr_latex_`-equivalent display hook.
- **Input direction (LaTeX/LyX → C++ expression)**: parse a subset of LaTeX (or a LyX document fragment) into a tensor expression at compile time or via a preprocessor / runtime parser. No mainstream C++ tensor library does this.

The educational positioning (ADR-0001) and the hybrid named-axis API (ADR-0004) both align naturally with a TeX-style authoring surface. The question is *how* to integrate.

---

## Decision Drivers

- **DD-1**: Pedagogical narrative. "Code that reads like the formula" is the original project pitch; a TeX bridge makes it literal.
- **DD-2**: Notebook ergonomics (per ADR-0008). Jupyter + xeus-cling + LaTeX display is a single-screen demo.
- **DD-3**: Implementation cost. A full LaTeX parser is a project unto itself; we want the smallest sufficient subset.
- **DD-4**: Round-trip integrity. A LaTeX expression rendered from a tensor expression should re-parse back to the same expression.

---

## Considered Options

1. **Output-only**: render tensor expressions as LaTeX; no input bridge.
2. **Output + small embedded DSL**: ship a `consteval` parser for a tightly scoped LaTeX subset (Einstein-notation contractions, sums, indexed arithmetic) consumed via a `_tex` user-defined literal, plus output rendering.
3. **Output + full LaTeX preprocessor**: external tool that translates `.tex` files to C++ source.
4. **LyX integration**: ship a LyX module that exports to the same DSL fed to (2).

---

## Decision Outcome

**Chosen option: 2 + 4 (incremental)** — start with output rendering and a `consteval` LaTeX-subset parser; add LyX export as a thin layer on top once the DSL stabilizes.

The `consteval` parser stays inside C++ (no external preprocessor build step), is bounded to Einstein-notation tensor expressions (subscripts/superscripts, sums, indexed arithmetic), and emits the same `LabelTag<...>` types as the NTTP path of ADR-0004. A user writes:

```cpp
auto c = R"(c_{ij} = a_i b_j)"_tex;        // compile-time parsed
auto d = tensor::tex("c_{ij} = a_i b_j");  // runtime variant
```

LyX integration becomes a separate deliverable: a LyX *Module file* that exports a tensor-document to a `.cpp` snippet using the same DSL. The full LaTeX preprocessor (option 3) is rejected as scope creep — we are not building a LaTeX compiler.

### Y-statement summary

> In the context of **bridging mathematical notation and the rewritten `tensor` library's C++ API**, facing **the choice of how to make tensor code read like the math**, we decided for **a `consteval` LaTeX-subset parser exposed via a `_tex` UDL plus output LaTeX rendering, with an optional LyX module layered on top**, to achieve **literal "code reads like the formula" semantics for the educational pitch**, accepting **the LaTeX surface is a tightly scoped subset, not full LaTeX**.

---

## Pros and Cons of the Options

### Option 1: Output-only

- Pros: minimal scope; near-free with xeus-cling.
- Cons: betrays the project pitch — only half the bridge.

### Option 2: Output + `consteval` LaTeX-subset parser (chosen)

- Pros: realises the original aspiration; stays inside the C++ build; pairs cleanly with ADR-0004's NTTP path; demonstrable in a notebook.
- Cons: requires designing and validating a LaTeX subset; `consteval` parsers are powerful but error messages are notoriously brittle.

### Option 3: Output + full LaTeX preprocessor

- Pros: maximum expressiveness.
- Cons: massive scope; reproduces existing TeX → C++ projects; pulls a LaTeX runtime into the build.

### Option 4: LyX integration (chosen as second phase)

- Pros: WYSIWYG math authoring, native to the user's stated dream; LyX has a well-documented [Module / ERT mechanism](https://wiki.lyx.org/LyX/Modules).
- Cons: secondary audience (LyX users only); benefits hinge on the DSL of (2) being stable first.

---

## Consequences

### Positive

- The educational narrative is "the formula *is* the program". This becomes the front-page demo.
- Notebooks render tensor expressions in proper LaTeX; cell input can paste in formulas from papers.
- LyX users get a first-class export path eventually, which is rare-enough to be marketable as a teaching feature.

### Negative

- The LaTeX subset must be specified, documented, and frozen — adding to the doc surface.
- `consteval` parser error messages are notoriously hard to get right; significant effort goes into "you mistyped a subscript here" UX.
- LyX is niche; investment beyond a thin module is hard to justify.

### Neutral

- The runtime variant `tensor::tex(...)` covers REPL / string-input use cases without compile-time guarantees.

### Follow-ups

- [ ] Specify the LaTeX subset (`docs/api-contract/tex-subset.md`): subscripts, superscripts, `\sum`, `\cdot`, fractions, parens, `=`, named tensors `\mathbf{a}`.
- [ ] Implement `consteval` parser with `static_assert` error messages.
- [ ] Implement runtime `tensor::tex(...)` parser sharing the same grammar.
- [ ] Implement `_repr_latex_`-equivalent for xeus-cling display hook.
- [ ] Phase 2: LyX Module file that exports tensor-bearing documents.

---

## Compliance / Validation

- Verification: round-trip property test — for every tensor expression in the test corpus, `parse(render(e)) == e`.
- Frequency: every PR.

---

## More Information

### Related ADRs

- Predecessors: [ADR-0001](0001-pivot-to-educational-named-axis-dsl.md), [ADR-0004](0004-adopt-hybrid-named-axis-api.md)
- Distribution: [ADR-0008](0008-distribute-as-header-only-with-jupyter-tutorials.md)

### References

- [xeus-cling — C++ Jupyter kernel](https://github.com/jupyter-xeus/xeus-cling)
- [LyX Modules wiki](https://wiki.lyx.org/LyX/Modules)
- [SymPy `latex()` and parsing](https://docs.sympy.org/latest/modules/parsing.html#module-sympy.parsing.latex) — Python precedent for round-trip TeX ↔ math
- [unicodemath / itex2MML](https://github.com/davidcarlisle/web-xslt) — alternative input bridges, considered out of scope
- Original 2016 blog post: [Qiita / convolutions-as-inner-products](http://qiita.com/uyuutosa/items/12e87f4695bd151b1d74)
