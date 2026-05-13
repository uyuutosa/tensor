---
status: Stable
owner: tensor
last-reviewed: 2026-05-14
---

# user-manual — Diátaxis-typed documentation

> Four quadrants of [Diátaxis](https://diataxis.fr) — tutorials, how-to, reference, explanation. Each quadrant serves a different reader stance; mixing them in one file confuses readers. Per [`../WORKFLOW.md` §3](../WORKFLOW.md).

## Quadrants

| Quadrant       | Stance                       | What it does                                                                  | When to write                                                                          |
| -------------- | ---------------------------- | ----------------------------------------------------------------------------- | -------------------------------------------------------------------------------------- |
| `tutorials/`   | learning by doing            | Walk a learner from zero to a working artifact. Prioritise feeling competent. | A new entry surface lands (Python SDK, new backend) and a learner needs onboarding.   |
| `how-to/`      | solving a specific task      | Give a task-oriented recipe. Skip the why; focus on the steps.                | A recurring "how do I…?" question surfaces in PR review.                                |
| `reference/`   | looking up exact information | List symbols / API surfaces / configuration keys. Information-oriented.       | A new public surface lands and users need an index.                                    |
| `explanation/` | understanding the idea       | Discuss design rationale, alternatives considered, broader context.            | A reader asks "why is it like this?" and the answer doesn't fit in an ADR.            |

## Index (this project's entries, 2026-05-14)

### tutorials/

- [`tutorials/python-getting-started.md`](./tutorials/python-getting-started.md) — first named-axis tensor in Python in 10 minutes.
- [`tutorials/cpp-named-axis-basics.md`](./tutorials/cpp-named-axis-basics.md) — first `DynamicTensor` + `_tex` UDL in 15 minutes.

(The notebook corpus at [`tutorials/`](../../tutorials/) and [`python/notebooks/`](../../python/notebooks/) is the *executable* tutorial layer; the files above are the prose introductions that orient the reader for those.)

### how-to/

- [`how-to/named-tensor-types.md`](./how-to/named-tensor-types.md) — pick between `Tensor` / `DynamicTensor` / `TypedTensor`.
- [`how-to/run-notebooks-locally.md`](./how-to/run-notebooks-locally.md) — build the Python SDK from source + execute the notebook corpus.
- [`how-to/use-set-backend.md`](./how-to/use-set-backend.md) — runtime backend selection (Phase 6.5 forward-doc).
- [`how-to/add-a-new-backend.md`](./how-to/add-a-new-backend.md) — implement a fourth `KernelBackend` adapter end-to-end.
- [`how-to/deploy-the-hf-space.md`](./how-to/deploy-the-hf-space.md) — push the Gradio scaffold to HuggingFace Spaces.

### reference/

- [`reference/cpp-namespace-overview.md`](./reference/cpp-namespace-overview.md) — every public C++ namespace + symbol at a glance.
- [`reference/python-package-overview.md`](./reference/python-package-overview.md) — every public Python symbol at a glance.

### explanation/

- [`explanation/why-named-axes.md`](./explanation/why-named-axes.md) — the design rationale for the headline feature.
- [`explanation/hexagonal-lite-rationale.md`](./explanation/hexagonal-lite-rationale.md) — why this architecture, what "lite" means.
- [`explanation/formula-is-the-program-essay.md`](./explanation/formula-is-the-program-essay.md) — what the slogan means and what it doesn't.

## Authoring rules

Per Diátaxis discipline:

1. **One stance per file.** A file that explains AND lists symbols belongs in two quadrants — split it.
2. **Tutorials lie by omission.** Skip the edge cases that don't help the learner reach the headline feeling-of-competence.
3. **How-tos assume the goal.** No "what is X" intros; the reader already knows the goal.
4. **Reference is exhaustive but not pedagogical.** Symbol-by-symbol, no commentary.
5. **Explanation is opinionated.** It's allowed to argue for a position; the position has to be defensible.

## Cross-references

- [`../WORKFLOW.md`](../WORKFLOW.md) §3 — the canonical Diátaxis routing decision tree.
- [`../INDEX.md`](../INDEX.md) — the project-level documentation index that points here.
- Diátaxis canonical source: <https://diataxis.fr>.
