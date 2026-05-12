---
status: Stable
owner: <placeholder>
last-reviewed: 2026-05-04
---

# user-manual — end-user documentation (Diátaxis)

> Authoritative source: <https://diataxis.fr>
>
> This directory follows the **Diátaxis** four-quadrant model. Pick the quadrant by what the *reader* is trying to do, not by the *topic*.

## The four quadrants

| Quadrant                              | Reader's goal                                | Example                                             |
| ------------------------------------- | -------------------------------------------- | --------------------------------------------------- |
| [`tutorials/`](./tutorials/)          | **Learning by doing** (newcomer)             | "Get from zero to a working hello-world"            |
| [`how-to/`](./how-to/)                | **Solving a specific problem** (competent)   | "How do I rotate API keys?"                         |
| [`reference/`](./reference/)          | **Looking something up** (precise, dry)      | "Full CLI flag list"                                |
| [`explanation/`](./explanation/)      | **Understanding** (background reading)       | "Why we chose token-based auth over session cookies"|

Do not invent fifth quadrants. Do not mix two quadrants in one file. If a file feels like both a tutorial and a how-to, split it.

## Authoring rules

1. **Tutorial** — guaranteed-to-succeed first-run experience. Show, do not explain. Hide trade-offs.
2. **How-to** — assumes the reader knows the basics. Solve one problem. Cover the relevant alternatives.
3. **Reference** — austere, complete, structured. No narrative.
4. **Explanation** — discursive. Background, history, "why". No instructions.

## Hard rules

- **English by default** (per [`../STRATEGY.md`](../STRATEGY.md)). Translations live in sibling directories: `user-manual-ja/`, `user-manual-de/`, etc.
- **Front-matter required** on every file (`status`, `owner`, `last-reviewed`).
- **Link from each how-to to the relevant reference page**, and vice versa.

For lifecycle / when to update, see [`../WORKFLOW.md`](../WORKFLOW.md).

## Reference

- Diátaxis — <https://diataxis.fr>
- Diátaxis quadrants explained — <https://diataxis.fr/start-here/>
