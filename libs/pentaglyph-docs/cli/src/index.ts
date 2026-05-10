#!/usr/bin/env bun
/**
 * pentaglyph — CLI scaffolder for the pentaglyph-docs kit.
 *
 * @remarks
 * Copies the `template/` directory of the pentaglyph-docs repository into a target
 * project, optionally trimming sections by `--include` and rewriting AI instructions
 * for the chosen `--ai` target.
 */
import { parseArgs } from "./lib/args.ts";
import { runInit } from "./commands/init.ts";
import { runAdd } from "./commands/add.ts";
import { printHelp, printVersion } from "./lib/help.ts";

const argv = process.argv.slice(2);

if (argv.length === 0 || argv[0] === "-h" || argv[0] === "--help") {
  printHelp();
  process.exit(0);
}

if (argv[0] === "-v" || argv[0] === "--version") {
  await printVersion();
  process.exit(0);
}

const [command, ...rest] = argv;

try {
  switch (command) {
    case "init": {
      const opts = parseArgs(rest);
      await runInit(opts);
      break;
    }
    case "add": {
      const opts = parseArgs(rest);
      await runAdd(opts);
      break;
    }
    default:
      console.error(`Unknown command: ${command}`);
      printHelp();
      process.exit(1);
  }
} catch (err) {
  const message = err instanceof Error ? err.message : String(err);
  console.error(`pentaglyph: ${message}`);
  process.exit(1);
}
