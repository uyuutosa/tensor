---
status: Stable
owner: <placeholder>
last-reviewed: 2026-05-09
---

# Version Control — branching, commits, and PR flow

This is the **explanation** that sits behind
[`.claude/rules/version-control.md`](../../.claude/rules/version-control.md).
The rule file is the operational checklist; this file explains *why* and
gives the project a place to record deviations.

## Default model: Git Flow

pentaglyph ships with **Git Flow** (Vincent Driessen, 2010) as the default
branching strategy. The full canonical model is at
<https://nvie.com/posts/a-successful-git-branching-model/> — read it
once before extending the rule.

A one-paragraph reminder of the topology:

- `main` and `develop` are long-lived. `main` only moves on tagged
  releases / hotfixes; `develop` is the ongoing integration line.
- `feature/*` and `bugfix/*` branch from `develop`, merge back to
  `develop`.
- `release/*` branches from `develop` when a version is being prepared,
  receives bug fixes only, and merges to **both** `main` (tagged) and
  `develop` (back-merge).
- `hotfix/*` branches from `main`, fixes a production-only bug, and
  merges to **both** `main` (tagged) and `develop`.

The trick of Git Flow is the dual back-merge: every `release/*` and
`hotfix/*` lands twice so production fixes never get lost on the
integration line.

## Why Git Flow rather than trunk-based

Three load-bearing reasons:

### 1. Legible release trail without tooling

`main` changes only via a release or hotfix merge. The commit log on
`main`, filtered by `--first-parent`, is the deployment history.
Regulators, auditors, and post-mortem authors can read it without the
team setting up release-tracking tooling. Trunk-based makes every commit
a candidate for production, which is fine when you have feature flags,
canary deploys, and instant rollback — but those cost engineering hours
that small / regulated teams often cannot afford up-front.

### 2. Integration safety net

`develop` lets two or more in-flight features stabilise together before
they reach `main`. Without it, every feature branch must be production-
ready in isolation, which forces feature flags or behind-the-scenes
toggles into work that could just live on an integration branch. The
overhead of `develop` (one extra merge per release) is lower than the
overhead of building a feature-flag system from scratch.

### 3. Stakeholder expectations in regulated domains

Medical, financial, and public-sector consumers often expect a release
branch trail. Git Flow gives them what they expect. Trunk-based does
not, and the team ends up reinventing release branches under a different
name on demand. Better to start with the model the auditor is asking
for.

## When Git Flow is wrong

Switch off the default when one of the following is true and document
the reason in this file:

- **Mature continuous deployment.** Feature flags, automated canary, and
  instant rollback are already in production. Trunk-based is strictly
  cheaper.
- **No release concept.** The product ships continuously without
  versioned releases (some SaaS, some libraries with rolling releases).
  GitHub Flow fits better.
- **Library mono-repo.** Multiple packages with independent versioning
  rarely need a single `develop`.

If you switch, replace the rule and the README references in this
project's `.claude/rules/version-control.md` so AI agents follow the new
model.

## Project extension points (per-project required)

Pentaglyph only ships the model. Each project must add the platform-
specific operational details to either this file or its sibling rule:

| Item | Examples |
|---|---|
| PR review platform | GitHub PRs / GitLab MRs / AWS CodeCommit / Bitbucket / Azure Repos |
| Exact CLI commands to open + merge a PR | `gh pr create … --base develop`, `aws codecommit merge-pull-request-by-three-way …` |
| Branch protection setup | Required reviewers, required CI checks, signed commits |
| Subtree / submodule push paths | When `lib/<name>/` is a subtree, where does it sync upstream? |
| CI hook configuration | Pre-commit, pre-push, server-side checks |
| Release tagging | Manual `git tag -a v…` vs automated by CI |
| Mirror remotes | Whether the repo is mirrored to a second remote, and the sync direction |

The rule file (`.claude/rules/version-control.md`) is the operational
checklist; this design-guide file is where deviations and rationale live.
Keep code-of-record decisions in ADRs under `docs/arc42/09-decisions/`.

## Forbidden anti-patterns

These hold across every branching model, not just Git Flow:

- **Force-pushing to a long-lived branch.** Even with permissions, never.
  History on `main` / `develop` is shared and rewriting it is unrecoverable
  for collaborators.
- **`git add .` / `git add -A`.** They sweep up `.env`, credentials, and
  build artefacts. Stage explicit paths and verify with `git diff --cached`
  before committing.
- **`git commit --no-verify`.** Pre-commit hooks exist for reasons.
  Address the underlying failure rather than skipping the gate. The
  one-off exception is when the user explicitly asks for it.
- **Squash-merging subtree commits.** The squash erases `git-subtree-dir:`
  metadata that future `git subtree push` requires. Use merge commits for
  any PR that contains a subtree pull.

## Templates and Conventional Commits

- Commit subject format: `<type>(<scope>?): <imperative subject>` per
  <https://www.conventionalcommits.org/>.
- Type vocabulary: `feat`, `fix`, `refactor`, `docs`, `test`, `chore`,
  `ci`, `perf`, `style`, `build`, `revert`.
- Body explains *why*, not *what*. The diff already shows what.
- Co-authors are credited via `Co-Authored-By:` trailers when relevant.

## Related

- [`.claude/rules/version-control.md`](../../.claude/rules/version-control.md)
  — operational rule loaded automatically by AI agents.
- [`docs/STRATEGY.md` §9](../STRATEGY.md) — where this file is referenced
  from the five-standard kit overview.
- [`docs/WORKFLOW.md`](../WORKFLOW.md) — documentation lifecycle (separate
  from code lifecycle).
