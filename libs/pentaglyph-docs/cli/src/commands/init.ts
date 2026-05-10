import { existsSync } from "node:fs";
import { mkdir, copyFile, writeFile, readFile } from "node:fs/promises";
import { dirname, join, resolve } from "node:path";
import type { ParsedArgs } from "../lib/args.ts";
import type { AiTarget, Profile, Section } from "../lib/types.ts";
import { PROFILE_SECTIONS, ROOT_FILES } from "../lib/profiles.ts";
import { copyDir, resolveTemplateDir } from "../lib/fs.ts";

/**
 * Substitute `<placeholder>` with the project name **only inside the YAML
 * front-matter block** at the top of the file.
 *
 * @remarks
 * `<placeholder>` is overloaded in the templates: it stands for the project
 * name in front-matter `owner:` lines (where it should be substituted) AND
 * for "fill-this-in" markers in the body of authoring templates (where it
 * must remain literal so the human author can spot it). Substituting only
 * inside the leading `---...---` block satisfies both.
 *
 * Files without front-matter are returned unchanged.
 */
function substitutePlaceholder(content: string, projectName: string): string {
  const fmMatch = content.match(/^---\n([\s\S]*?)\n---\n/);
  if (!fmMatch) return content;
  const fmEnd = fmMatch[0].length;
  const head = content.slice(0, fmEnd).replaceAll("<placeholder>", projectName);
  const body = content.slice(fmEnd);
  return head + body;
}

export async function runInit(opts: ParsedArgs): Promise<void> {
  const target = opts._[0];
  if (!target) {
    throw new Error(
      "Usage: pentaglyph init <target-dir> [--profile=...] [--ai=...] [--name=...]",
    );
  }

  const targetRoot = resolve(target);
  const targetDocs = join(targetRoot, "docs");

  const profile: Profile = opts.profile ?? "standard";
  const ai: AiTarget = opts.ai ?? "generic";
  const projectName = opts.name ?? "<placeholder>";

  const sections: readonly Section[] = opts.include
    ? (opts.include as Section[])
    : PROFILE_SECTIONS[profile];

  const log = (msg: string) => process.stdout.write(`${msg}\n`);

  log(`pentaglyph init`);
  log(`  target:   ${targetDocs}`);
  log(`  profile:  ${profile}${opts.include ? " (overridden by --include)" : ""}`);
  log(`  sections: ${sections.join(", ")}`);
  log(`  ai:       ${ai}`);
  log(`  name:     ${projectName}`);
  log(`  force:    ${opts.force ?? false}`);
  log(`  dry-run:  ${opts.dryRun ?? false}`);
  log("");

  const templateRoot = await resolveTemplateDir();
  const templateDocs = join(templateRoot, "docs");

  if (!existsSync(templateDocs)) {
    throw new Error(`Template docs not found at ${templateDocs}`);
  }

  if (!opts.dryRun) {
    await mkdir(targetDocs, { recursive: true });
  }

  // Always-installed root files
  for (const f of ROOT_FILES) {
    await installFile(
      join(templateDocs, f),
      join(targetDocs, f),
      { force: !!opts.force, dryRun: !!opts.dryRun, projectName, log },
    );
  }

  // Sections
  for (const section of sections) {
    const src = join(templateDocs, section);
    const dest = join(targetDocs, section);
    if (!existsSync(src)) {
      log(`  warn   section "${section}" not found in template — skipping`);
      continue;
    }
    log(`  add    ${section}/`);
    await copyDir(src, dest, {
      force: !!opts.force,
      dryRun: !!opts.dryRun,
      log,
      transform: (content) => substitutePlaceholder(content, projectName),
    });
  }

  // AI hook
  await installAiHook(templateRoot, targetRoot, ai, opts, log);

  log("");
  log("Done.");
  log(`Read ${targetDocs}/AI_INSTRUCTIONS.md and ${targetDocs}/WORKFLOW.md next.`);
}

interface InstallFileOpts {
  force: boolean;
  dryRun: boolean;
  projectName: string;
  log: (msg: string) => void;
}

async function installFile(
  src: string,
  dest: string,
  opts: InstallFileOpts,
): Promise<void> {
  if (!existsSync(src)) {
    opts.log(`  warn   ${src} missing in template`);
    return;
  }
  if (existsSync(dest) && !opts.force) {
    opts.log(`  skip   ${dest} (exists)`);
    return;
  }
  if (opts.dryRun) {
    opts.log(`  would  ${dest}`);
    return;
  }
  await mkdir(dirname(dest), { recursive: true });
  const content = await readFile(src, "utf-8");
  const replaced = substitutePlaceholder(content, opts.projectName);
  await writeFile(dest, replaced, "utf-8");
  opts.log(`  write  ${dest}`);
}

async function installAiHook(
  templateRoot: string,
  targetRoot: string,
  ai: AiTarget,
  opts: ParsedArgs,
  log: (msg: string) => void,
): Promise<void> {
  if (ai === "generic") {
    log(`  ai     generic → AI_INSTRUCTIONS.md only (no editor hook)`);
    return;
  }

  // Claude target: install full .claude/ tree (rules + agents + skills).
  // The bundled doc-orchestrator + 5 specialist agents + 3 slash commands
  // turn /doc-init into a guided conversational doc builder.
  if (ai === "claude") {
    const src = join(templateRoot, ".claude");
    const dest = join(targetRoot, ".claude");
    if (!existsSync(src)) {
      log(`  warn   Claude template source missing: ${src}`);
      return;
    }
    log(`  ai     claude → .claude/{rules,agents,skills}/ (orchestrator + 5 specialists + 3 commands)`);
    await copyDir(src, dest, {
      force: !!opts.force,
      dryRun: !!opts.dryRun,
      log,
      transform: opts.name
        ? (content) => substitutePlaceholder(content, opts.name!)
        : undefined,
    });
    return;
  }

  // Cursor / Copilot targets: re-use the documentation rule as a project
  // instruction file at the editor's expected path.
  const map: Record<Exclude<AiTarget, "generic" | "claude">, { src: string; dest: string }> = {
    cursor: {
      src: join(templateRoot, ".claude/rules/documentation.md"),
      dest: join(targetRoot, ".cursor/rules/docs.md"),
    },
    copilot: {
      src: join(templateRoot, ".claude/rules/documentation.md"),
      dest: join(targetRoot, ".github/copilot-instructions.md"),
    },
  };

  const entry = map[ai];
  if (!existsSync(entry.src)) {
    log(`  warn   AI hook source missing: ${entry.src}`);
    return;
  }
  if (existsSync(entry.dest) && !opts.force) {
    log(`  skip   ${entry.dest} (exists)`);
    return;
  }
  if (opts.dryRun) {
    log(`  would  ${entry.dest}`);
    return;
  }
  await mkdir(dirname(entry.dest), { recursive: true });
  await copyFile(entry.src, entry.dest);
  log(`  write  ${entry.dest}`);
}
