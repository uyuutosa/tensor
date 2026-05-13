---
status: Stable
owner: tensor
last-reviewed: 2026-05-14
---

# Glossary maintenance — when to add / when to refresh

> The §12 glossary at [`../arc42/12-glossary/overview.md`](../arc42/12-glossary/overview.md) is one of the three citability disciplines per G-8 ([ADR-0015](../arc42/09-decisions/0015-aspire-to-canonical-reference-quality-not-self-anoint.md)). This guide is the contributor-facing decision tree for "when does this PR need a glossary entry?"

## 1. Triggering events

A glossary entry is required when:

- **A new public symbol lands** in `tensor::core`, `tensor::autograd`, `tensor::tex`, or any backend adapter. Per OC-5 + ADR-0015 §Compliance, every public name traces to a math-literature source. The glossary entry is the trace.
- **A new public Python symbol lands** in `tensor.*`, `tensor.autograd`, or `tensor.tex`. The Python side gets its own glossary entries when the Python name differs from the C++ name (e.g. `DynamicTensorF32` is Python-specific because Python lacks C++'s template parameter notation).
- **A new substrate-specific term enters the vocabulary** — Dawn, WGSL, xeus-cpp, mdspan, etc. New ADRs that introduce substrate terms (like ADR-0016's `webgpu_cpp.h`) trigger a corresponding glossary update.
- **A new bug-class name surfaces in design-guide or arc42 §11 lessons-learned**. Examples: `notebook output gate`, `plotly MathJax v2 trap`, `Sphinx source-tree limit`. Names that recur in the docs need glossary anchors so readers can search for them.

## 2. When NOT to add an entry

- **Private / implementation-detail names** (`_tensor_native`, `BroadcastPlan` if it's not exposed). The glossary mirrors the *public* surface.
- **Generic computer-science terms** (linked list, B-tree, mutex). Link to Wikipedia / cppreference where relevant; don't duplicate definitions the wider community owns.
- **Math-literature primitives** that the project doesn't *own* (matrix, vector, scalar). The glossary names where the project's vocabulary attaches to the math, not the math itself.

## 3. Entry shape

Every glossary entry has:

1. A bold-italic-or-`code`-style heading naming the term.
2. A 1–3 sentence definition.
3. A math-literature anchor where one exists (paper, textbook citation, or another ADR / DD).
4. A cross-reference to the detailed-design or design-guide entry that uses the term.

Reference entry (from `Bifocal / Trifocal / Quadrifocal tensors` added in PR #121):

```markdown
### Bifocal / Trifocal / Quadrifocal tensors

| Tensor          | Index pattern         | DOF | What it constrains          |
| --------------- | ---------------------- | --- | --------------------------- |
| Bifocal F_{ij}  | pix1 × pix2            | 7   | Epipolar geometry…           |
...
Reference: Hartley & Zisserman §§9, 15, 17.

Each is a worked example of "the algebra is the named-axis
contraction" — see `03_multifocal-tensors.ipynb`.
```

## 4. Half-yearly audit

Per [ADR-0015 §Compliance](../arc42/09-decisions/0015-aspire-to-canonical-reference-quality-not-self-anoint.md) bullet 2, the glossary is audited at every half-yearly bibliography audit (next: **2026-11-11**). The audit checks:

- Every public symbol in `python-public-surface.md` + `cpp-tensor-*-surface.md` has a §12 entry.
- Every `see §12` pointer across the docs resolves.
- No glossary entry is dead — every term has at least one inbound cross-reference from a detailed-design or design-guide.

The audit produces a `YYYY-MM-DD_bibliography-audit.md` report under [`../reports/`](../reports/) that lists adds / removals / refreshes.

## 5. Quick-add workflow

When a PR adds a new public symbol:

1. Add the symbol to the relevant `api-contract/<surface>.md` table.
2. Add a paragraph or sub-bullet to `arc42/12-glossary/overview.md` under the appropriate section.
3. Add a `see §12` cross-reference from the detailed-design that introduces the symbol.
4. Run the cross-reference audit script (see [`./cross-reference-discipline.md`](./cross-reference-discipline.md)) to confirm the new entry's links resolve.

Steps 1–3 belong in the same PR. Step 4 catches the common error of pointing at a nonexistent section.

## 6. Anti-patterns

- **Adding an entry without an inbound cross-reference.** A glossary entry no one links to is dead weight.
- **Adding an entry without a math-literature anchor.** If the term is original to this project, the anchor is the ADR or the detailed-design that introduced it; *something* has to ground the name.
- **Editing an entry's body during a PR that doesn't introduce a vocabulary change.** Glossary edits cluster at bibliography-audit time; opportunistic edits in unrelated PRs make `git log -- arc42/12-glossary/overview.md` unreadable.

## 7. Cross-references

- The glossary itself: [`../arc42/12-glossary/overview.md`](../arc42/12-glossary/overview.md).
- [ADR-0015](../arc42/09-decisions/0015-aspire-to-canonical-reference-quality-not-self-anoint.md) — the citability discipline this glossary serves.
- [ADR-0017](../arc42/09-decisions/0017-clarify-reproducibility-envelope.md) — the half-yearly audit cadence.
- [`./cross-reference-discipline.md`](./cross-reference-discipline.md) — the link-audit script that catches glossary cross-ref errors.
