#!/usr/bin/env bun
// Bun handles .ts extension + top-level await natively, so we just import the
// TypeScript entry directly. The package declares `engines.bun >= 1.1`, and
// `bun add -g @uyuutosa/pentaglyph` guarantees bun is on the user's PATH.
await import("../src/index.ts");
