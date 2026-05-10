---
status: Stable
owner: <placeholder>
last-reviewed: 2026-05-04
---

# api-contract — public symbol contracts

> **Use Template 0** ([`../templates/0_default.md`](../templates/0_default.md)) or just the OpenAPI / schema file directly.

This directory holds machine-readable schemas (OpenAPI, GraphQL SDL, MCP tool descriptions, RPC IDLs) and their human-readable summaries.

## Suggested layout

```text
api-contract/
├── README.md
├── <module>.md                  # human-readable summary linking to the schema file
├── openapi/
│   └── <module>.yaml            # OpenAPI 3.1 spec
├── graphql/
│   └── <module>.graphql         # GraphQL SDL
└── mcp/
    └── <module>.json            # MCP tool description
```

## Rules

1. **Schema is the source of truth.** Markdown summaries link to the schema file; never duplicate the schema in prose.
2. **Cross-link from the corresponding `../detailed-design/<module>.md` §4.3 (API specification)**.
3. **Generated artifacts** (e.g. TypeScript types from OpenAPI) live in `src/`, not here. This directory holds only the *contract*, not the *generated client*.

For lifecycle / when to update, see [`../WORKFLOW.md`](../WORKFLOW.md).
