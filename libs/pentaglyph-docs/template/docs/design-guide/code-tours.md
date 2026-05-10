---
status: Stable
owner: <placeholder>
last-reviewed: 2026-05-10
---

# Code tours — guided reading paths through the codebase

> **What this file is.** The convention this kit ships for *guided reading paths* — short, ordered walkthroughs that take a new contributor through a feature, a bug fix, or an architectural slice in the order it makes sense to understand it.
>
> **What this file is not.** A user manual ([`../user-manual/`](../user-manual/)). A decision record ([`../arc42/09-decisions/`](../arc42/09-decisions/)). A detailed design spec ([`../detailed-design/`](../detailed-design/)). The Diátaxis framework Pentaglyph adopts for end-user docs is explicitly *not* meant to absorb contributor-onboarding artefacts; see <https://diataxis.fr/start-here/>.

---

## 1. Why a separate convention

Reading a non-trivial change in the order the diff happens to be presented is rarely the order that makes the change *understandable*. The reviewer's productive path is usually:

1. The entry point that the change extends or replaces.
2. The interface contract or schema (if any) that the change implements.
3. The new code, in the call order it executes at runtime.
4. The tests, paired with the code they exercise.
5. The places elsewhere in the codebase that have been touched defensively.

That sequence cannot be expressed in a flat PR description, and it does not belong inside the source files as comments. The industry has converged on **CodeTour** as the most portable representation of a reading path:

> Microsoft's CodeTour project, the *de facto* standard, defines a JSON schema (`.tour` files) for ordered, file-and-line-anchored walkthroughs that any editor or IDE can render. Source: <https://github.com/microsoft/codetour>; schema: <https://github.com/microsoft/codetour/blob/main/schema.json>.

This kit adopts CodeTour's file format directly. We do not invent a parallel schema.

---

## 2. When to write a code tour

Write one when at least two of the following are true:

- The change touches **three or more files** that must be read in a non-obvious order.
- The change crosses a layer boundary documented in [`../arc42/05-building-blocks/`](../arc42/05-building-blocks/).
- A reviewer unfamiliar with this slice of the codebase would need more than ~15 minutes to reconstruct the reading order from the diff alone.
- The change introduces a new abstraction, contract, or mechanism that other code will subsequently depend on.

Do **not** write one for typo fixes, single-file refactors, dependency bumps, or any change small enough to read top-to-bottom of the diff.

---

## 3. Where tours live

CodeTour itself searches three well-known directories. Pick **one** per repository and stay consistent:

| Directory | When to choose it |
| --- | --- |
| `.tours/` | Default. Tours are first-class artefacts of the repo. |
| `.vscode/tours/` | The repo treats tours as IDE configuration rather than documentation. |
| `.github/tours/` | The repo wants tours visible alongside other GitHub-platform metadata (PR templates, issue templates). |

Pentaglyph defaults to `.tours/`. Override the default in [`../STRATEGY.md`](../STRATEGY.md) if the project has a reason to diverge.

---

## 4. Authoring rules

1. **Pin the tour to a commit.** Set the `ref` field on the tour to the commit SHA the tour was authored against. CodeTour will warn the reader when the underlying lines have moved; without `ref` the tour silently rots.
2. **Keep tours short.** Five to twelve steps. A tour that needs more than twelve steps is usually two tours, or is replacing a missing detailed-design document.
3. **Cite, do not re-explain.** Each step's `description` should orient the reader and link out — to the relevant ADR, detailed-design file, or external standard — rather than rephrasing what those documents already say.
4. **One tour per change of substance, not per PR.** A PR may include multiple tours (rare) or none (common). A tour outlives the PR that introduced it; treat tours as durable design artefacts that need updating when the underlying code changes.
5. **Mark the entry tour.** If the repository ships a "start here" tour for new contributors, set `isPrimary: true` on it. Use `nextTour` to chain tours into a recommended sequence.
6. **Tours are reviewable.** Include the `.tour` file in the same PR as the code it describes so reviewers can verify that the reading path matches the change.

---

## 5. Minimal `.tour` example

```json
{
  "$schema": "https://aka.ms/codetour-schema",
  "title": "Adding a new payment provider",
  "ref": "<commit-sha-this-tour-is-pinned-to>",
  "description": "Walks through the entry point, the provider contract, the new adapter, and the integration tests.",
  "steps": [
    {
      "title": "Entry point",
      "file": "src/payments/registry.ts",
      "line": 42,
      "description": "All providers register here. The new `acme` provider is added on line 47."
    },
    {
      "title": "Provider contract",
      "file": "src/payments/provider.ts",
      "line": 1,
      "description": "Implementations must satisfy this interface. See [ADR-0023](../docs/arc42/09-decisions/0023-payment-provider-abstraction.md) for the reasoning."
    },
    {
      "title": "Adapter",
      "file": "src/payments/providers/acme.ts",
      "pattern": "export class AcmeProvider",
      "description": "Implementation. The retry policy on `charge()` is the riskiest part; see test below."
    },
    {
      "title": "Tests",
      "file": "src/payments/providers/acme.test.ts",
      "pattern": "describe\\('charge'",
      "description": "Covers happy path, network error retry, and signature validation. Does *not* cover concurrent refund races yet."
    }
  ]
}
```

Full schema: <https://github.com/microsoft/codetour/blob/main/schema.json>.

---

## 6. Relationship to other artefacts

| Question the reader has | Where they should look |
| --- | --- |
| "What does this product do, end-to-end?" | [`../user-manual/`](../user-manual/) (Diátaxis) |
| "Why was this approach chosen?" | [`../arc42/09-decisions/`](../arc42/09-decisions/) (MADR) |
| "What is the design of this module?" | [`../detailed-design/`](../detailed-design/) |
| "How do I read this change?" | this convention — `.tours/*.tour` |
| "Why am I being asked to verify these specific risks on this PR?" | [`./ai-augmented-pr.md`](./ai-augmented-pr.md) + the PR template |

A tour can — and should — link to all four neighbours. It is the navigation aid that ties them together for a contributor who is new to a slice of the codebase.

---

## References

- Microsoft CodeTour — <https://github.com/microsoft/codetour>
- CodeTour JSON schema — <https://github.com/microsoft/codetour/blob/main/schema.json>
- Diátaxis on its own scope (excludes contributor-facing onboarding) — <https://diataxis.fr/start-here/>
- Knuth, Literate Programming (1984) — the historical antecedent for "code as a guided reading experience" — <https://www-cs-faculty.stanford.edu/~knuth/lp.html>
