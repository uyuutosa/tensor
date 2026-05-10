#!/usr/bin/env node
/**
 * Sync ../template/ → ./template/ before pack/publish.
 *
 * Why: the canonical template lives at the repo root (../template/) so it can
 * be subtree-pushed and consumed independently. But `npm publish` only ships
 * what is inside the package directory (`cli/`). This script copies the
 * template into the package right before packing so the published tarball
 * contains everything `pentaglyph init` needs at runtime.
 *
 * Run by `prepack` lifecycle hook (also runs implicitly on `npm publish`).
 */
import { cp, rm, stat } from "node:fs/promises";
import { existsSync } from "node:fs";
import { fileURLToPath } from "node:url";
import { dirname, join, resolve } from "node:path";

const here = dirname(fileURLToPath(import.meta.url));
const cliRoot = resolve(here, "..");
const repoRoot = resolve(cliRoot, "..");
const src = join(repoRoot, "template");
const dest = join(cliRoot, "template");

async function main() {
  if (!existsSync(src)) {
    console.error(`sync-template: source not found at ${src}`);
    console.error(`  Are you running from a checkout that includes ../template/?`);
    process.exit(1);
  }

  const srcStat = await stat(src);
  if (!srcStat.isDirectory()) {
    console.error(`sync-template: ${src} is not a directory`);
    process.exit(1);
  }

  if (existsSync(dest)) {
    await rm(dest, { recursive: true, force: true });
  }

  await cp(src, dest, { recursive: true });
  console.log(`sync-template: copied ${src} → ${dest}`);
}

main().catch((err) => {
  console.error("sync-template failed:", err);
  process.exit(1);
});
