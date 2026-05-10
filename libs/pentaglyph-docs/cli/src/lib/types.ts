/** Profile selects which sections of the pentaglyph kit to scaffold. */
export type Profile = "minimal" | "standard" | "full";

/**
 * AI target — controls which AI-instruction file gets installed.
 *
 * - `claude`  → installs `.claude/rules/documentation.md` auto-load rule
 * - `cursor`  → installs `.cursor/rules/docs.md`
 * - `copilot` → installs `.github/copilot-instructions.md`
 * - `generic` → installs `AI_INSTRUCTIONS.md` only (no editor-specific hook)
 */
export type AiTarget = "claude" | "cursor" | "copilot" | "generic";

/** Language for boilerplate text in scaffolded files. Templates themselves are English. */
export type Language = "en" | "ja" | "both";

/**
 * Section identifier — corresponds to a top-level directory under `template/docs/`.
 *
 * Used by `--include` to selectively scaffold only the listed sections.
 */
export type Section =
  | "arc42"
  | "diagrams"
  | "detailed-design"
  | "design-guide"
  | "api-contract"
  | "impl-plans"
  | "task-list"
  | "postmortems"
  | "reports"
  | "cost-estimates"
  | "user-manual"
  | "templates";
