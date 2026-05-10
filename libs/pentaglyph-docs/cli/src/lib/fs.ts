import { mkdir, readdir, copyFile, stat, readFile, writeFile } from "node:fs/promises";
import { existsSync } from "node:fs";
import { dirname, join, relative, isAbsolute } from "node:path";

/**
 * Format a path for log output.
 *
 * @remarks
 * Prefers a relative path **only when** the target is under the current
 * working directory. Otherwise falls back to the absolute path to avoid
 * unreadable `../../../../` chains when the target lives elsewhere
 * (e.g. `/tmp/...`).
 */
function fmt(path: string): string {
  const rel = relative(process.cwd(), path);
  if (rel === "" || rel.startsWith("..") || isAbsolute(rel)) {
    return path;
  }
  return rel;
}

export interface CopyOptions {
  force: boolean;
  dryRun: boolean;
  /** Optional file-content transformer. Receives content + relative path. */
  transform?: (content: string, relativePath: string) => string;
  /** Optional log sink. */
  log?: (msg: string) => void;
}

/**
 * Recursively copy a directory tree.
 *
 * @remarks
 * - Respects `force` and `dryRun` flags.
 * - Applies `transform` only to text files (utf-8 decodable). Binary files copy verbatim.
 */
export async function copyDir(
  srcDir: string,
  destDir: string,
  opts: CopyOptions,
): Promise<{ copied: number; skipped: number }> {
  let copied = 0;
  let skipped = 0;

  if (!opts.dryRun) {
    await mkdir(destDir, { recursive: true });
  }

  const entries = await readdir(srcDir, { withFileTypes: true });
  for (const entry of entries) {
    const srcPath = join(srcDir, entry.name);
    const destPath = join(destDir, entry.name);

    if (entry.isDirectory()) {
      const sub = await copyDir(srcPath, destPath, opts);
      copied += sub.copied;
      skipped += sub.skipped;
      continue;
    }

    if (existsSync(destPath) && !opts.force) {
      opts.log?.(`  skip   ${fmt(destPath)} (exists)`);
      skipped++;
      continue;
    }

    if (opts.dryRun) {
      opts.log?.(`  would  ${fmt(destPath)}`);
      copied++;
      continue;
    }

    if (opts.transform && isLikelyText(srcPath)) {
      const raw = await readFile(srcPath, "utf-8");
      const transformed = opts.transform(raw, relative(srcDir, srcPath));
      await mkdir(dirname(destPath), { recursive: true });
      await writeFile(destPath, transformed, "utf-8");
    } else {
      await mkdir(dirname(destPath), { recursive: true });
      await copyFile(srcPath, destPath);
    }

    opts.log?.(`  write  ${fmt(destPath)}`);
    copied++;
  }

  return { copied, skipped };
}

/** Is the file likely text (transformable) based on extension? */
function isLikelyText(path: string): boolean {
  const TEXT_EXTS = [".md", ".txt", ".dsl", ".yml", ".yaml", ".json", ".ts", ".tsx", ".js", ".mjs", ".cjs", ".sh"];
  return TEXT_EXTS.some((ext) => path.endsWith(ext));
}

/**
 * Resolve the path of the bundled `template/` directory relative to this file.
 *
 * @remarks
 * Search order (first hit wins):
 * 1. `cli/template/` — bundled by `npm publish` via the `prepack` hook.
 *    This is what end users get.
 * 2. `<repo-root>/template/` — monorepo dev mode where `template/` lives
 *    next to `cli/` and is the canonical source.
 *
 * If neither exists, the user is running an incomplete checkout — surface
 * a helpful error.
 */
export async function resolveTemplateDir(): Promise<string> {
  // cli/src/lib/fs.ts → cli/ is 2 levels up; cli/template lives under cli/
  // cli/dist/index.js → cli/ is 1 level up
  const here = dirname(new URL(import.meta.url).pathname);
  const candidates = [
    join(here, "..", "..", "template"),         // cli/src/lib/fs.ts → cli/template (bundled)
    join(here, "..", "template"),               // cli/dist/index.js → cli/template (bundled)
    join(here, "..", "..", "..", "template"),   // cli/src/lib/fs.ts → repo-root/template (dev)
    join(here, "..", "..", "template"),         // cli/dist/index.js → repo-root/template (dev)
  ];
  for (const c of candidates) {
    try {
      const s = await stat(c);
      if (s.isDirectory()) return c;
    } catch {
      // try next
    }
  }
  throw new Error(
    "Could not locate the bundled template/ directory. " +
      "Reinstall pentaglyph or pass --template <path>.",
  );
}
