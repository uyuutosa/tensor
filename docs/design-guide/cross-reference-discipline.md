---
status: Stable
owner: tensor
last-reviewed: 2026-05-14
---

# Cross-reference discipline

> Links between docs are the connective tissue of the documentation system. When they rot, navigation breaks and reviewers can't trust pointers. This guide is the convention for *what kind* of link to use and *how to verify* the links a PR introduces resolve.

## 1. Link style

| Context                              | Style                                                                                | Why                                                                  |
| ------------------------------------ | ------------------------------------------------------------------------------------ | -------------------------------------------------------------------- |
| Within `docs/`                        | Relative: `[ADR-0019](../09-decisions/0019-...md)`                                  | Survives file moves; works in any Markdown viewer.                   |
| From `docs/` to a sibling top-level   | Relative: `[CONTRIBUTING.md](../../CONTRIBUTING.md)`                                | Same survival property.                                              |
| To upstream code                      | Repo-root-relative shown as path: `` `include/tensor/core/dynamic_tensor.hpp` ``    | Code paths don't need URL clickability — readers will grep / cd.    |
| External resources                    | Full URL: `<https://nanobind.readthedocs.io/>`                                       | Self-explanatory; resilient.                                         |
| To a specific PR                      | `[PR #117](https://github.com/uyuutosa/tensor/pull/117)` or just `(PR #117)`        | PR text is searchable; explicit links for PRs the reader will click. |
| To a heading inside the same file     | `[§4 Decision Outcome](#4-decision-outcome)`                                          | GitHub auto-anchors headings.                                        |

**Forbidden**:

- `https://github.com/uyuutosa/tensor/blob/main/docs/...` — uses absolute GitHub URLs to in-repo files. Breaks on forks, breaks on offline reads, doesn't follow file moves.
- Bare filename references (`README.md` without a path). Ambiguous when multiple READMEs exist.
- `./` for same-directory references. Use the filename directly (`[foo](./foo.md)` → `[foo](foo.md)`). Bare filename is the GitHub convention.

## 2. Link audit on every doc PR

The cycle-19 script from the 20-cycle PDCA log:

```python
# tools/check-docs-links.py (planned; for now, paste this inline)
import os, re, glob, sys

def audit(files):
    broken = []
    for f in files:
        if not os.path.exists(f): continue
        src = open(f).read()
        for m in re.finditer(
            r'\[[^\]]+\]\((\.\.?/[^)#\s]+(?:\.md|\.cff|\.py|\.cpp|\.ipynb|\.yml|/))(?:#[^)\s]*)?\)', src
        ):
            href = m.group(1).rstrip('/')
            full = os.path.normpath(os.path.join(os.path.dirname(f), href))
            if not os.path.exists(full):
                broken.append((f, href, full))
    return broken

if __name__ == "__main__":
    files = glob.glob('docs/**/*.md', recursive=True)
    broken = audit(files)
    if not broken:
        print('OK no broken relative links')
        sys.exit(0)
    print(f'BROKEN ({len(broken)}):')
    for src, href, full in broken:
        print(f'  {src} -> {href}  (resolves to {full})')
    sys.exit(1)
```

Run before opening any doc PR. The script is intentionally simple — it catches the 90% case (typos, file moves, missing files) without trying to parse fragments, code-block contents, or external URLs.

## 3. Common breakage patterns

| Pattern                                                          | Example breakage                                                       | Fix                                                                       |
| ---------------------------------------------------------------- | ---------------------------------------------------------------------- | ------------------------------------------------------------------------- |
| File renamed, links not updated                                   | `[old name](./old-file.md)` → 404                                       | grep + `sed` across the docs tree                                          |
| Directory restructured                                            | `../arc42/09-decisions/0015-...` → wrong number of `..`                | Audit script catches; fix with relative-path arithmetic                    |
| Markdown anchor casing drift                                      | `[§4 Foo](#4-Foo)` → fails; GitHub auto-anchor is `#4-foo`              | Anchors are always lowercase + hyphens                                     |
| Code path drift                                                   | `[backend.hpp](../../include/tensor/core/backend/reference.hpp)` → file moved | grep for the path; update everywhere                                       |
| External link domain change                                       | `https://example.com/old-path/` → 404                                  | Quarterly external-link sweep (manual; not yet automated)                  |

## 4. Anchor pinning rules

When a `## §N` heading is renamed or removed, every `[…](file.md#section-anchor)` reference to it breaks silently — `git status` shows no error. Conventions to mitigate:

- **Don't rename headings casually.** If a section heading needs to change, the same PR updates every inbound anchor.
- **Use numbered headings** for stable navigation. `## 4. Decision Outcome` is more stable than `## The Decision`.
- **Anchor by topic, not by ID.** `(#decision-outcome)` outlives `(#4-decision-outcome)` when section numbers shift, but section numbers are more readable. Use the numbered form unless reorderings are likely.

## 5. INDEX maintenance

The `docs/INDEX.md` "Index" section is the project-level cross-reference hub. Every new durable doc (under `arc42/`, `detailed-design/`, `design-guide/`, `api-contract/`, `user-manual/`) gets a link from INDEX in the same PR that adds it. The cycle-1 + cycle-18 from the 20-cycle PDCA log encodes this discipline; the half-yearly audit re-verifies.

## 6. When in doubt

- Run the audit script.
- Open the rendered Markdown in GitHub's preview (or local `jupyter-book build book` for the published site).
- Click the links. The mechanical check above catches most errors; clicking catches the rest (wrong anchor, wrong fragment, link-text-doesn't-match).

## 7. Cross-references

- [`./glossary-maintenance.md`](./glossary-maintenance.md) — the related discipline for vocabulary anchors.
- [`../WORKFLOW.md` §6 hard rule 7](../WORKFLOW.md) — "Repo-root-relative links" is the upstream version of §1 of this file.
- [`docs/INDEX.md`](../INDEX.md) — the project-level index this discipline maintains.
- Half-yearly audit cadence: [ADR-0015 §Compliance](../arc42/09-decisions/0015-aspire-to-canonical-reference-quality-not-self-anoint.md).
