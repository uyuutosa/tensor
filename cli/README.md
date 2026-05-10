# pentaglyph (CLI)

Scaffold a documentation tree based on arc42 + C4 + MADR + Diátaxis with one workflow.

## Install

> Currently published as the scoped name **`@uyuutosa/pentaglyph`** (0.0.x series).
> The unscoped `pentaglyph` name is reserved for the 0.1.0 stable release. See [`PUBLISH.md`](./PUBLISH.md) for the versioning policy.

### Run without install (recommended)

```bash
bunx --bun @uyuutosa/pentaglyph init ./my-project --profile=standard --ai=claude
# or
npx @uyuutosa/pentaglyph init ./my-project --profile=standard --ai=claude
```

### Install globally

```bash
bun add -g @uyuutosa/pentaglyph
# or
npm install -g @uyuutosa/pentaglyph
```

### From source (before npm publish, or for development)

```bash
git clone https://github.com/uyuutosa/pentaglyph-docs.git
cd pentaglyph-docs/cli
bun install
bun run src/index.ts init ../sample --profile=standard --ai=claude
```

## Usage

```text
pentaglyph init <target-dir> [options]
pentaglyph add <section> [target-dir] [options]
pentaglyph --help | --version
```

### `init` — scaffold a new docs/ tree

```bash
pentaglyph init ./my-app --profile=standard --ai=claude --name="My App"
```

This creates `./my-app/docs/` populated with the pentaglyph kit and (if `--ai=claude`) an auto-load rule at `./my-app/.claude/rules/documentation.md`.

### `add` — add a single section to an existing scaffold

```bash
pentaglyph add user-manual ./my-app
```

## Options

| Flag         | Values                                          | Default        | Effect                                                                      |
| ------------ | ----------------------------------------------- | -------------- | --------------------------------------------------------------------------- |
| `--profile`  | `minimal` / `standard` / `full`                 | `standard`     | Which sections to include (see below)                                       |
| `--include`  | comma list of sections                          | (from profile) | Override profile with explicit section list                                 |
| `--ai`       | `claude` / `cursor` / `copilot` / `generic`     | `generic`      | Which editor's auto-load rule to install                                    |
| `--lang`     | `en` / `ja` / `both`                            | `en`           | Language of boilerplate text (templates remain English regardless)          |
| `--name`     | string                                          | placeholder    | Project name written into front-matter `<placeholder>` slots                |
| `--force`    |                                                 | false          | Overwrite existing files                                                    |
| `--dry-run`  |                                                 | false          | Print what would happen, write nothing                                      |

## Profiles

- **minimal** — `templates/` + `arc42/` (12 sections + MADR ADRs). For libraries / single-team projects.
- **standard** — minimal + `diagrams/` + `detailed-design/` + `api-contract/` + `design-guide/` + `impl-plans/` + `postmortems/` + `reports/`. For most product teams.
- **full** — standard + `task-list/` + `cost-estimates/` + `user-manual/` (Diátaxis quadrants). For customer-facing products with end-user docs.

## AI targets

| Target    | Installs                                            |
| --------- | --------------------------------------------------- |
| `claude`  | `.claude/rules/documentation.md` auto-load rule     |
| `cursor`  | `.cursor/rules/docs.md`                             |
| `copilot` | `.github/copilot-instructions.md`                   |
| `generic` | `docs/AI_INSTRUCTIONS.md` only (no editor hook)     |

All AI targets get `docs/AI_INSTRUCTIONS.md` — the editor-specific hook just adds a pointer that auto-loads when the editor opens `docs/**`.

## Sections

| Section            | Layer | Purpose                                                                  |
| ------------------ | ----- | ------------------------------------------------------------------------ |
| `templates`        | A     | Six authoring templates (always installed)                               |
| `arc42`            | A     | arc42 §1–§12 architecture description                                    |
| `diagrams`         | A     | C4 model (Structurizr DSL)                                               |
| `detailed-design`  | A     | Per-module implementation specs                                          |
| `design-guide`     | A     | Operational conventions                                                  |
| `api-contract`     | A     | OpenAPI / GraphQL / MCP / RPC schemas                                    |
| `user-manual`      | A     | Diátaxis quadrants (tutorials / how-to / reference / explanation)        |
| `impl-plans`       | B     | Dated implementation plans                                               |
| `task-list`        | B     | Sprint-scoped task breakdowns                                            |
| `postmortems`      | B     | Incident retrospectives                                                  |
| `reports`          | B     | One-shot research / evaluation reports                                   |
| `cost-estimates`   | B     | Cost projections                                                         |

## Develop

```bash
bun install
bun run dev init /tmp/test-app --profile=standard --ai=claude
bun run typecheck
bun run sync-template      # copy ../template/ into cli/template/
bun run smoke              # one-shot end-to-end smoke test
```

## Publishing

See [`PUBLISH.md`](./PUBLISH.md) for the publish flow, versioning policy, and the eventual move to the unscoped `pentaglyph` name.

## License

MIT.
