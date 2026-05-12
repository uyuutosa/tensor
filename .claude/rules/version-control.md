---
paths:
  - "**"
---

# Version Control Rule (auto-loaded globally)

This kit ships **Git Flow** as the default branching model. Downstream
projects that need a different model (trunk-based, GitHub Flow, release-train,
…) override this rule by writing their own
[`docs/design-guide/version-control.md`](../../docs/design-guide/version-control.md)
**and** updating `.claude/rules/version-control.md` with the project-specific
deviation. Until then, the rules below are mandatory.

Reference (canonical): Vincent Driessen, "A successful Git branching model"
<https://nvie.com/posts/a-successful-git-branching-model/>. Re-paraphrasing
the model here is forbidden — read the source. This rule only encodes the
operational decisions pentaglyph makes on top of the model.

## Long-lived branches

| Branch    | Role                                                   | Push target            |
| --------- | ------------------------------------------------------ | ---------------------- |
| `main`    | Production-ready. Tagged releases only. Never broken.  | hotfix / release merge |
| `develop` | Integration line for the next release. CI must pass.   | feature / bugfix merge |

`main` and `develop` are both protected. Direct pushes to either are
forbidden — every change arrives via a pull request that targets one of
the two.

## Short-lived branches

| Prefix      | Branch from | Merge into                     | Naming                                |
| ----------- | ----------- | ------------------------------ | ------------------------------------- |
| `feature/`  | `develop`   | `develop`                      | `feature/<kebab-description>`         |
| `bugfix/`   | `develop`   | `develop`                      | `bugfix/<kebab-description>`          |
| `release/`  | `develop`   | `main` AND `develop` + tag     | `release/<semver>` (e.g. `release/1.4.0`) |
| `hotfix/`   | `main`      | `main` AND `develop` + tag     | `hotfix/<semver>` (e.g. `hotfix/1.3.2`) |

Refactor / chore / docs / test / ci / perf changes use the `feature/`
prefix unless they materially shift behaviour, in which case promote to
`bugfix/` or call out the risk in the PR.

## Required workflows

### New work

```bash
git checkout develop && git pull
git checkout -b feature/<short-description>
# ... commit ...
git push -u origin feature/<short-description>
# Open PR targeting develop. Merge with a merge commit (no fast-forward,
# no squash) so the history shows the integration boundary.
```

### Cutting a release

1. From `develop`: `git checkout -b release/<semver>`.
2. Bump version files / changelog. **No new features** on the release branch
   — bug fixes only.
3. Open PR `release/<semver>` → `main`. Merge with a merge commit and tag
   `<semver>` on `main`.
4. Open PR `release/<semver>` → `develop` (back-merge) so the version bump
   and any release-only fixes return to integration. Merge with a merge
   commit.
5. Delete the release branch.

### Production hotfix

1. From `main`: `git checkout -b hotfix/<semver>`.
2. Fix the bug. Bump patch version.
3. Open PR `hotfix/<semver>` → `main`. Merge + tag.
4. Open PR `hotfix/<semver>` → `develop` (back-merge).
5. Delete the hotfix branch.

## Why Git Flow as default

Three reasons drive the choice:

1. **Predictable release boundaries.** `main` only changes via a release
   or a hotfix merge, so the production deployment trail is one merge
   commit per release. Auditors, regulators, and post-mortem authors find
   that legible without tooling.
2. **Integration safety net.** `develop` lets multiple in-flight features
   stabilise together before they touch production. Trunk-based assumes
   continuous deployment maturity (feature flags, canary, instant rollback)
   that newer / smaller teams typically lack.
3. **Compatibility with downstream consumers.** Many regulated industries
   (medical, financial, public-sector) expect a release-branch trail. Git
   Flow gives them what they expect without extra ceremony.

Pick a different model only when one of those drivers is actively wrong
for the project (e.g. mature CD with feature flags makes trunk-based
strictly better). Document the deviation in
[`docs/design-guide/version-control.md`](../../docs/design-guide/version-control.md)
with a one-paragraph rationale.

## Hard rules

1. **No direct pushes to `main` or `develop`.** Every change arrives via
   a PR. Branch protection on the host should enforce this; the rule
   stands even when the host does not.
2. **No `git push --force` to `main` or `develop`.** Long-lived branches
   are immutable. Force-push is allowed only on personal feature branches
   that no one else is rebasing onto.
3. **Conventional Commits.** Subject lines follow
   <https://www.conventionalcommits.org/>. Type vocabulary: `feat`, `fix`,
   `refactor`, `docs`, `test`, `chore`, `ci`, `perf`, `style`, `build`,
   `revert`. Optional scope in parentheses (`feat(api): …`).
4. **Stage explicit paths.** `git add .` and `git add -A` are forbidden —
   they routinely sweep up `.env`, secrets, and `__pycache__`. Use
   explicit paths and verify with `git diff --cached` before committing.
5. **Never `--no-verify`.** Pre-commit hooks run for a reason. Fix the
   underlying issue rather than skipping. The exception is when the user
   explicitly asks for a one-off skip.
6. **Merge commits over squash for integration PRs.** Feature → develop
   and release → main use merge commits so the integration boundary
   stays visible in `git log --first-parent`.
7. **Tag releases on `main` only.** A version tag never lives on
   `develop` or a feature branch.
8. **Subtree-aware merges.** When the PR contains commits made by
   `git subtree pull`, do not squash — the squash-merge erases the
   `git-subtree-dir:` metadata that future `git subtree push` needs.
   Use a merge commit.

## Project-specific extensions

Each downstream project supplements this rule with at minimum:

- The PR review platform (GitHub / GitLab / CodeCommit / Azure DevOps)
  and the exact CLI commands to create / merge a PR.
- Branch protection setup (required reviewers, required status checks).
- Any subtree / submodule push paths.
- CI hook / pre-commit configuration.
- Whether release tagging is automated or manual.

Those extensions belong in
[`docs/design-guide/version-control.md`](../../docs/design-guide/version-control.md).
This file stays universal.
