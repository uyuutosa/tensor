# pentaglyph-docs

<p align="center">
  <img src="./assets/hero.png" alt="pentaglyph-docs — five standards (arc42, C4, MADR, Diátaxis, TiSDD) bound by one AI-first workflow" width="100%" />
</p>

> A documentation scaffold built on five industry standards — **arc42** (architecture), **C4** (diagrams), **MADR** (decisions), **Diátaxis** (user docs), and **TiSDD** (service design) — with a single explicit workflow and AI-readable instructions per directory.

The name **pentaglyph** (Greek `penta` "five" + `glyph` "engraved sign") reflects the five standards bundled into one opinionated kit. Renamed from the original `tetragram` (four standards) when TiSDD was adopted as the fifth peer standard; see `cli/PUBLISH.md` for migration notes.

| #  | Standard      | Authoritative source                                  | Local home                          |
| -- | ------------- | ----------------------------------------------------- | ----------------------------------- |
| 1  | **arc42**     | <https://arc42.org/overview/>                         | `template/docs/arc42/`              |
| 2  | **C4 model**  | <https://c4model.com>                                 | `template/docs/diagrams/c4/`        |
| 3  | **MADR v3.0** | <https://adr.github.io/madr/>                         | `template/docs/arc42/09-decisions/` |
| 4  | **Diátaxis**  | <https://diataxis.fr>                                 | `template/docs/user-manual/`        |
| 5  | **TiSDD**     | <https://www.thisisservicedesigndoing.com/methods>    | `template/docs/service-design/`     |

External standards are authoritative. This kit only adds:

1. **Concrete file layout** that maps each standard to a directory.
2. **A single canonical workflow** ([`template/docs/WORKFLOW.md`](./template/docs/WORKFLOW.md)) that tells humans and AI agents *when to write what, where to put it, and what state it goes through*.
3. **Per-directory `README.md`** files with explicit AI instructions so an LLM can place new content correctly with zero project context.
4. **A Bun-based CLI** (`cli/`) that scaffolds a new project's `docs/` from this template with profile / language / AI-target options.

---

## Repo layout

```text
pentaglyph-docs/
├── README.md                 # this file
├── LICENSE                   # MIT
├── template/                 # the doc kit — what gets copied
│   ├── docs/
│   │   ├── INDEX.md              # entry point
│   │   ├── STRATEGY.md           # taxonomy, layers, authoring rules
│   │   ├── WORKFLOW.md           # ★ single source of truth for "what to write when"
│   │   ├── AI_INSTRUCTIONS.md    # entry point for AI agents
│   │   ├── arc42/                # arc42 §1–§12
│   │   ├── diagrams/c4/          # C4 model (Structurizr DSL)
│   │   ├── detailed-design/      # per-module specs
│   │   ├── design-guide/         # operational conventions
│   │   ├── api-contract/         # OpenAPI / MCP-tool schemas
│   │   ├── impl-plans/           # dated implementation plans
│   │   ├── task-list/            # sprint-scoped task breakdowns
│   │   ├── postmortems/          # incident retrospectives
│   │   ├── reports/              # one-shot research reports
│   │   ├── cost-estimates/       # cost projections
│   │   ├── user-manual/          # Diátaxis quadrants
│   │   └── templates/            # 6 authoring templates
│   └── .claude/rules/            # Claude Code auto-load rule
└── cli/                          # Bun CLI scaffolder
    ├── package.json
    ├── tsconfig.json
    └── src/
```

---

## Quick start (CLI)

```bash
# scaffold a new project's docs/
bunx pentaglyph init ./my-project --profile=standard --ai=claude --lang=en
```

See [`cli/README.md`](./cli/README.md) for full CLI usage.

---

## Quick start (manual copy)

```bash
git clone https://github.com/uyuutosa/pentaglyph-docs.git
cp -r pentaglyph-docs/template/docs ./my-project/docs
cp -r pentaglyph-docs/template/.claude ./my-project/.claude
```

Then read [`docs/AI_INSTRUCTIONS.md`](./template/docs/AI_INSTRUCTIONS.md) and [`docs/WORKFLOW.md`](./template/docs/WORKFLOW.md) — those two files contain everything you need.

---

## Why "pentaglyph"?

The five standards in this kit each answer a different question:

| Standard | Question it answers                                       |
| -------- | --------------------------------------------------------- |
| arc42    | *How is the system organised?*                            |
| C4       | *What does it look like at each zoom level?*              |
| MADR     | *Why did we choose this over alternatives?*               |
| Diátaxis | *How do users learn this product?*                        |
| TiSDD    | *How is the **service** experienced end-to-end?*          |

Picking just one is incomplete. Picking all five is opinionated but defensible — and that opinion is what this kit packages. The name `pentaglyph` (Greek `penta` "five" + `glyph` "engraved sign") replaces the earlier `tetragram` (four standards) as of v0.1.0, when TiSDD joined as the fifth peer standard.

---

## License

MIT. See [`LICENSE`](./LICENSE).
