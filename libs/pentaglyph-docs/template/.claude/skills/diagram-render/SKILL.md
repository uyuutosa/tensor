---
name: diagram-render
description: >
  Render every committed diagram source under `docs/` (Structurizr DSL, PlantUML,
  Mermaid) to SVG and embed the renders in viewer markdown files so the diagrams
  are visible from any repo browser (GitHub, CodeCommit, Bitbucket) without
  client-side rendering. Idempotent — safe to re-run after every diagram source
  change. Replaces the historical "exports/ is gitignored" policy.
disable-model-invocation: false
---

# /diagram-render — render & commit diagrams

Use this skill when you want every diagram under `docs/` to be browseable
without a local toolchain. The skill walks the diagram source tree, resolves
the right renderer for each notation, produces SVGs alongside the source,
and updates a `views.md` viewer that embeds the SVGs.

## What it does

1. **Discovery** — walks `docs/diagrams/` (and any other directory the project
   designates) and groups files by notation:
   - `*.dsl` → Structurizr DSL (each `views {}` view becomes one SVG)
   - `*.puml`, `*.plantuml` → PlantUML
   - `*.mmd`, `*.mermaid` → Mermaid
   - Inline ` ```mermaid ` blocks inside `*.md` are left in-place — repo
     browsers that render Markdown also render Mermaid blocks.
2. **Toolchain resolution** — for each notation, resolves the CLI via the
   precedence below. Missing CLIs produce a structured install hint, not a
   silent skip.
   - Structurizr → `structurizr-cli` (Java JAR or `structurizr/cli` Docker image)
   - PlantUML → `plantuml` (Java) or `plantuml.jar` discovered in `$PATH`
     or `~/.local/share/plantuml/`
   - Mermaid → `mmdc` (Node `@mermaid-js/mermaid-cli`)
3. **Render** — produces `<source-dir>/exports/<view-name>.svg`. The
   `exports/` directory is **committed**, not gitignored. Each SVG carries a
   header comment with the source file path and a content hash so drift is
   detectable.
4. **Embed** — generates / refreshes `<source-dir>/views.md` with a section per
   view containing:
   - The view title and one-line description
   - `![view-name](./exports/view-name.svg)`
   - A "regenerate via `/diagram-render`" footer line
5. **Drift check** — re-hashes each SVG against the source and prints a
   table of (`view-name`, `source-hash`, `svg-hash`, `status`). A non-zero
   drift count is a warning the caller can choose to surface or block on.

## When to use

- After editing any diagram source — **before committing**
- In CI as a freshness gate (the skill prints a non-zero exit code if the SVG
  hash does not match the source hash)
- When picking up an unfamiliar repo and you want to see the architecture
  before reading the DSL

## When NOT to use

- You are mid-edit and exploring shape options — render manually with the
  CLI directly, run the skill once you settle on a shape
- Inline Mermaid in narrative Markdown (rule of thumb: free-flowing
  explanation use inline blocks; canonical architecture diagrams use a
  source file under `docs/diagrams/`)

## Tooling install (one-time per machine)

The skill detects missing tools and prints these install steps. They are
captured here so the skill output is short and the canonical install
recipe is one place.

### Structurizr CLI

```bash
# Java >= 11 required (already available on most dev images)
cd ~/.local/share && \
  curl -L -o structurizr-cli.zip https://github.com/structurizr/cli/releases/latest/download/structurizr-cli.zip && \
  unzip structurizr-cli.zip -d structurizr-cli && \
  ln -sf "$(pwd)/structurizr-cli/structurizr.sh" ~/.local/bin/structurizr-cli
# verify
structurizr-cli version
```

Docker alternative (no Java install):

```bash
docker pull structurizr/cli
alias structurizr-cli='docker run --rm -v "$PWD:/usr/local/structurizr" structurizr/cli'
```

### PlantUML

```bash
# Debian / Ubuntu — Graphviz is the SVG-rendering back-end
sudo apt-get update && sudo apt-get install -y plantuml graphviz
# verify
plantuml -version
```

Java alternative if `apt` is unavailable (e.g. macOS):

```bash
brew install plantuml graphviz
```

### Mermaid CLI

```bash
# Node >= 18 required
npm install -g @mermaid-js/mermaid-cli
# Headless Chromium is downloaded on first invocation; allow ~150 MB
mmdc --version
```

## Operating contract

### Inputs

- `--root <path>` — root to scan, default `docs/`
- `--target <path>` — single file or single directory to render (overrides
  root scan; useful for fast iteration on one diagram)
- `--check` — render to a temp dir, compare hashes, report drift, exit
  non-zero on drift; **do not write SVG / views.md**. Used in CI.
- `--no-views` — skip the viewer-markdown step (just render SVGs)

### Outputs

For every input source `<dir>/<name>.<ext>`:

- `<dir>/exports/<view-or-name>.svg` (one SVG per view for Structurizr; one
  SVG per source file for PlantUML / Mermaid)
- `<dir>/views.md` (regenerated; deterministic) — embeds every SVG under
  `<dir>` and its sub-tree

### Idempotency

The skill is fully idempotent:

- Same input ⇒ same SVG bytes (renderer determinism allowing — see Caveats)
- Re-running with no source change is a no-op (drift check passes; no file
  writes)
- Re-running after a source change updates only the affected SVGs and the
  affected `views.md` sections

### Determinism caveats

- Structurizr CLI's PlantUML export does not guarantee byte-identical output
  across versions. The skill records the renderer version in the SVG header
  comment so version-driven churn is auditable.
- PlantUML SVG output includes a generation timestamp by default. The
  skill passes `-Sruntimeenv=true` and post-processes the SVG to strip the
  timestamp comment, so timestamp-only diffs do not pollute commits.
- Mermaid CLI is the most deterministic of the three (the renderer hashes
  inputs internally) and requires no post-processing.

## Project hard rules (paired with this skill)

When a project adopts this skill, two `docs/` policy lines flip:

| Before                              | After                                         |
| ----------------------------------- | --------------------------------------------- |
| `exports/ is gitignored`            | `exports/ is committed; regenerate via /diagram-render` |
| `Image renders are gitignored.`     | `Image renders are committed; the skill is the canonical regenerator.` |

The flip is required because **CodeCommit and many corporate Git web UIs
do not render Mermaid in markdown**, so the only way to make an
architecture diagram visible from the repo browser is to commit a
self-contained SVG. The cost (one SVG per view in git history) is small;
the benefit (every reviewer can see the architecture without local
tooling) is large.

## Workflow integration

| Trigger | What runs |
|---|---|
| After editing a `.dsl` / `.puml` / `.mmd` | `/diagram-render` once locally before commit |
| Pre-commit hook (recommended) | `/diagram-render --check` (fails on drift; commit must include up-to-date SVGs) |
| Pre-PR CI (recommended) | `/diagram-render --check` |
| Post-merge to `main` | optional `/diagram-render` to regenerate canonical SVGs (only useful if you do NOT enforce drift in PR) |

## Output schema

A successful run prints one summary line per source plus a final table:

```
[discover] docs/diagrams/c4/workspace.dsl  (Structurizr; 2 views)
[render]   docs/diagrams/c4/exports/L1-SystemContext.svg  ← workspace.dsl
[render]   docs/diagrams/c4/exports/L2-Container.svg       ← workspace.dsl
[embed]    docs/diagrams/c4/views.md (refreshed)

| view              | source-hash | svg-hash    | status   |
| ----------------- | ----------- | ----------- | -------- |
| L1-SystemContext  | a3f9…       | a3f9…       | up-to-date |
| L2-Container      | bc1d…       | bc1d…       | up-to-date |
```

## Cross-references

- C4 Model — <https://c4model.com>
- Structurizr CLI — <https://docs.structurizr.com/cli>
- PlantUML — <https://plantuml.com>
- Mermaid — <https://mermaid.js.org>
- Project docs rule pairing this skill — `.claude/rules/documentation.md` rule #5
