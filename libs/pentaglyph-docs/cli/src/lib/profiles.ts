import type { Profile, Section } from "./types.ts";

/** Sections that are ALWAYS included regardless of profile. */
const ALWAYS: readonly Section[] = ["templates"] as const;

/** Profile → ordered list of sections to scaffold. */
export const PROFILE_SECTIONS: Record<Profile, readonly Section[]> = {
  /**
   * Minimal — just the workflow + arc42 + ADRs + templates.
   * Use for: small libraries, single-team projects, internal tools.
   */
  minimal: [...ALWAYS, "arc42"],

  /**
   * Standard — everything in minimal, plus C4, detailed-design, api-contract,
   * and the volatile Layer B set.
   * Use for: most product teams.
   */
  standard: [
    ...ALWAYS,
    "arc42",
    "diagrams",
    "detailed-design",
    "api-contract",
    "design-guide",
    "impl-plans",
    "postmortems",
    "reports",
  ],

  /**
   * Full — every section, including user-manual (Diátaxis quadrants),
   * task-list, and cost-estimates.
   * Use for: customer-facing products with end-user docs.
   */
  full: [
    ...ALWAYS,
    "arc42",
    "diagrams",
    "detailed-design",
    "api-contract",
    "design-guide",
    "impl-plans",
    "task-list",
    "postmortems",
    "reports",
    "cost-estimates",
    "user-manual",
  ],
};

/**
 * Files that are ALWAYS installed at `docs/` root regardless of section selection.
 *
 * These are the entry points and source-of-truth files; without them the kit
 * does not function.
 */
export const ROOT_FILES: readonly string[] = [
  "INDEX.md",
  "STRATEGY.md",
  "WORKFLOW.md",
  "AI_INSTRUCTIONS.md",
] as const;
