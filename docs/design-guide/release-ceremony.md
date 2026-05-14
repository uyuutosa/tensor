---
status: Stable
owner: tensor
last-reviewed: 2026-05-13
---

# Release ceremony — Git Flow specifics as actually executed

> The Git Flow general rule lives in [`.claude/rules/version-control.md`](../../.claude/rules/version-control.md). This document captures the **specifics that recur every release** and the **maintainer-only steps** that no automation handles. Updated after each release cut.

## 1. Trigger

A phase closes (per [`book/roadmap.md`](../../book/roadmap.md)) and the maintainer wants to cut a public tag. As of 2026-05-13 the project has cut `0.1.0` (Phase 4) and prepared `0.2.0` (Phase 6) with `0.3.0` (Phase 6.5) staged.

## 2. The eight-step sequence

```text
1. From develop:
     git checkout develop && git pull
     git checkout -b release/<semver>

2. Version bumps — use the helper to keep all 5 files in lockstep:
     bash tools/release.sh <semver>
   The helper updates:
   - vcpkg.json                          "version-string"
   - CMakeLists.txt                       project(... VERSION ...)
   - pyproject.toml                       project version + 2 extras pins
   - python/extras/tensor-named-axis-eigen/pyproject.toml   version + base pin
   - python/extras/tensor-named-axis-webgpu/pyproject.toml  version + base pin
   (Mitigates R-P6.5.1 lockstep-drift risk.)

3. CHANGELOG promotion:
   - Promote the [Unreleased] block to [<semver>] - <YYYY-MM-DD>.
   - Add a summary header in the prior-release format
     (look at the [0.1.0] / [0.2.0] entries — three short paragraphs +
      a bulleted highlight list).
   - Open a fresh [Unreleased] section with "(empty — work towards
     <next-semver> accumulates here.)".

4. Commit + push the release branch:
     git add CHANGELOG.md CMakeLists.txt pyproject.toml vcpkg.json
     git commit -m "release: <semver>"
     git push -u origin release/<semver>

5. Open the PR as DRAFT to main:
     gh pr create --base main --head release/<semver> --draft \
         --title "release: <semver> — <one-line scope>" \
         --body "[from RELEASE_PR_BODY.md template]"
   Draft because the maintainer-only PyPI / GitHub Release setup
   gates the actual merge.

6. Maintainer-only side work (does not block PR open, gates merge):
   - PyPI: register the new distribution if needed (e.g. `tensor-named-axis-eigen`
     for Phase 6.5).
   - PyPI: configure the trusted publisher policy
     (Settings → Publishing → Add → GitHub publisher;
      workflow `cibuildwheel.yml`; environment `pypi`).
   - GitHub Releases: draft the release notes off the CHANGELOG entry
     (do not publish yet; tag triggers publish on merge).

7. Convert PR draft → ready, merge into main with a **merge commit**
   (not squash — preserves the release boundary in
    `git log --first-parent`). Then tag:
     git checkout main && git pull
     git tag -a <semver> -m "tensor <semver> — <one-line scope>"
     git push origin <semver>
   Tag push triggers `.github/workflows/cibuildwheel.yml`:
   builds wheels for the CPython matrix, publishes via OIDC.

8. Back-merge release/<semver> → develop:
     gh pr create --base develop --head release/<semver> \
         --title "release: back-merge <semver> to develop"
   Merge with a merge commit (no squash). Delete the release branch.
```

## 3. Reference releases (specifics that worked)

### `0.1.0` (Phase 4, 2026-05-12)

- PRs #91 (release → main) + #92 (back-merge → develop).
- One-line scope: "Phase 1 + 1.5 + 2 + 2.5 + 3 close, public tag".
- Maintainer steps that surfaced: enabling GitHub Pages required adding the `develop` + `main` branches to `custom_branch_policies` via `gh api -X PUT`.
- No PyPI step (no Python SDK yet).

### `0.2.0` (Phase 6, 2026-05-13)

- PR #115 (draft, release → main) — held draft pending PyPI setup.
- One-line scope: "Phase 6 — Python SDK via nanobind".
- Maintainer steps to do before merge: register `tensor-named-axis` on PyPI; configure trusted publisher; convert PR draft → ready; merge; tag; back-merge.
- HF Space deploy is optional follow-up (`./huggingface/space/deploy.sh <user>` after `hf auth login`).

### `0.3.0` (Phase 6.5, planned)

- Per [ADR-0019](../arc42/09-decisions/0019-phase-6-5-runtime-backend-selection-via-extras.md) + [Phase 6.5 impl-plan](../impl-plans/2026-05-13_phase-6-5-set-backend.md).
- Three PyPI distributions to register before merge: `tensor-named-axis-eigen`, `tensor-named-axis-webgpu` (the base already exists from `0.2.0`).
- Lockstep version bumps across `pyproject.toml`s (base + 2 companions) — a `release.sh` helper is the Phase 6.5 follow-up that prevents drift (R-P6.5.1).

## 4. Hard rules (do not skip)

- **Merge commits, never squash**, for both release → main and back-merge → develop. Squash erases `git-subtree-dir:` metadata required by the `libs/pentaglyph-docs/` subtree (`.claude/rules/version-control.md` §Hard rules #8).
- **Tag on `main` only**, never on develop or feature branches (`.claude/rules/version-control.md` §Hard rules #7).
- **Draft PR until PyPI setup is complete** — opening as ready risks an accidental merge that fires the tag-push workflow against an unconfigured trusted-publisher policy. The publish step will fail loudly but you'll still have a tag on main that has to be deleted.
- **Don't bump versions on develop directly**. Always cut a `release/<semver>` branch first; the version files belong to the release branch until back-merge.
- **The back-merge PR title is `release: back-merge <semver> to develop`** — this exact prefix lets `git log --first-parent --grep '^release:'` recover both directions of every release cycle. Variants like `release/0.2.0` → `develop` or `back-merge: 0.2.0` break the grep. The PR #92 (back-merge of `0.1.0`) is the reference implementation.
- **`develop` must be 100% CI-green at release-cut time.** If it isn't, the prerequisite is one or more `feature/fix-*` PRs that get develop to green first. PR #113 (CI flakiness fix) was the precondition for cutting `release/0.2.0` (PR #115).

## 5. Soft rules (the maintainer's preferences)

- One PR for the release, even when multiple files change (CHANGELOG + 3 version files = 4 files but 1 logical change).
- The release PR body should list every step the maintainer will do (PyPI register, trusted-publisher, draft→ready, merge, tag, back-merge). PR #115's body is the current template.
- Back-merge title is `release: back-merge <semver> to develop` — the prefix `release:` keeps `git log --first-parent --grep '^release:'` clean.

## 6. Cross-references

- [`.claude/rules/version-control.md`](../../.claude/rules/version-control.md) — the Git Flow rule this guide specialises.
- [`book/roadmap.md`](../../book/roadmap.md) — phase status that drives "is it time to cut?".
- [`CHANGELOG.md`](../../CHANGELOG.md) — the file the ceremony promotes from `[Unreleased]` to `[<semver>]`.
- [`.github/workflows/cibuildwheel.yml`](../../.github/workflows/cibuildwheel.yml) — the workflow tag-push triggers.
- Reference PRs to imitate: [PR #91](https://github.com/uyuutosa/tensor/pull/91) (`0.1.0`), [PR #115](https://github.com/uyuutosa/tensor/pull/115) (`0.2.0`).
