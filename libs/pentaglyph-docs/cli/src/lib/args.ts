/**
 * Lightweight CLI argument parser for pentaglyph.
 *
 * Supports:
 *   --flag=value
 *   --flag value
 *   --boolean-flag
 *   positional arguments (collected as `_`)
 */

import type { Profile, AiTarget, Language } from "./types.ts";

export interface ParsedArgs {
  /** Positional arguments. */
  _: string[];
  /** Profile selecting which sections to include. */
  profile?: Profile;
  /** Comma-separated list of explicit sections to include (overrides profile). */
  include?: string[];
  /** AI target — affects which AI instruction file is generated. */
  ai?: AiTarget;
  /** Language for templates' boilerplate text. */
  lang?: Language;
  /** Project name written into front-matter and INDEX.md placeholders. */
  name?: string;
  /** Overwrite existing files. */
  force?: boolean;
  /** Print actions without writing. */
  dryRun?: boolean;
}

const VALID_PROFILES: ReadonlySet<Profile> = new Set([
  "minimal",
  "standard",
  "full",
] as const);

const VALID_AI: ReadonlySet<AiTarget> = new Set([
  "claude",
  "cursor",
  "copilot",
  "generic",
] as const);

const VALID_LANG: ReadonlySet<Language> = new Set([
  "en",
  "ja",
  "both",
] as const);

export function parseArgs(argv: string[]): ParsedArgs {
  const out: ParsedArgs = { _: [] };

  for (let i = 0; i < argv.length; i++) {
    const token = argv[i]!;

    if (token === "--force") {
      out.force = true;
      continue;
    }
    if (token === "--dry-run") {
      out.dryRun = true;
      continue;
    }

    if (token.startsWith("--")) {
      const eq = token.indexOf("=");
      let key: string;
      let value: string;
      if (eq >= 0) {
        key = token.slice(2, eq);
        value = token.slice(eq + 1);
      } else {
        key = token.slice(2);
        const next = argv[i + 1];
        if (next === undefined || next.startsWith("--")) {
          throw new Error(`Flag --${key} requires a value`);
        }
        value = next;
        i++;
      }
      assignFlag(out, key, value);
      continue;
    }

    out._.push(token);
  }

  return out;
}

function assignFlag(out: ParsedArgs, key: string, value: string): void {
  switch (key) {
    case "profile":
      if (!VALID_PROFILES.has(value as Profile)) {
        throw new Error(
          `--profile must be one of: ${[...VALID_PROFILES].join(", ")}`,
        );
      }
      out.profile = value as Profile;
      return;
    case "include":
      out.include = value.split(",").map((s) => s.trim()).filter(Boolean);
      return;
    case "ai":
      if (!VALID_AI.has(value as AiTarget)) {
        throw new Error(
          `--ai must be one of: ${[...VALID_AI].join(", ")}`,
        );
      }
      out.ai = value as AiTarget;
      return;
    case "lang":
      if (!VALID_LANG.has(value as Language)) {
        throw new Error(
          `--lang must be one of: ${[...VALID_LANG].join(", ")}`,
        );
      }
      out.lang = value as Language;
      return;
    case "name":
      out.name = value;
      return;
    default:
      throw new Error(`Unknown flag --${key}`);
  }
}
