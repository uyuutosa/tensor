# Publishing the pentaglyph CLI to npm

> Audience: maintainers of [`@uyuutosa/pentaglyph`](https://www.npmjs.com/package/@uyuutosa/pentaglyph).
> If you only want to *use* the CLI, see [`README.md`](./README.md).

This document covers the **OIDC trusted-publishing flow** (steady state) and the
fallback **manual publish** (one-off / disaster recovery).

---

## Versioning policy

- **0.0.x** — initial scoped releases. API may break between any two patch versions. The unscoped `pentaglyph` name is **not yet claimed**.
- **0.1.0** — first release once the API has stabilised in real use. At this point we publish unscoped `pentaglyph` and keep `@uyuutosa/pentaglyph` as a forwarding alias.
- **1.0.0** — semver guarantees engage. No breaking changes without a major bump.

The package name change between 0.0.x and 0.1.0+ is deliberate — it signals stability publicly and lets the unscoped name accumulate trust before being claimed.

---

## Steady-state release flow (OIDC trusted publishing — preferred)

> Why OIDC? Classic tokens were retired by npm in November 2025. Granular tokens have a known [bug](https://github.com/npm/cli/issues/8869) preventing 2FA bypass on personal accounts. OIDC trusted publishing is the only fully-tokenless path.

### Prerequisites (one-time setup)

1. **npm account `uyuutosa`** with 2FA enabled.
2. **GitHub Actions trusted publisher** configured in npm:
   - Open <https://www.npmjs.com/package/@uyuutosa/pentaglyph/access>
   - Section **Trusted Publisher** → **Add Trusted Publisher**
   - Provider: **GitHub Actions**
   - Organization or user: `uyuutosa`
   - Repository: `pentaglyph-docs`
   - Workflow filename: `publish.yml`
   - Environment name: leave blank (or set to `production` if you also configure a matching environment in `.github/workflows/publish.yml`)
   - Save
3. **Workflow `publish.yml`** is checked into [`.github/workflows/publish.yml`](../.github/workflows/publish.yml) — already done.

### Per-release steps

```bash
cd cli/

# 1. Bump version in cli/package.json (semver)
#    e.g. 0.0.4 → 0.0.5
$EDITOR package.json

# 2. Commit + push the bump
git add cli/package.json
git commit -m "chore(pentaglyph/cli): release v0.0.5"
git push

# 3. Tag the release (the workflow triggers on this exact pattern)
git tag pentaglyph-cli-v0.0.5
git push origin pentaglyph-cli-v0.0.5

# 4. Watch the workflow
gh run watch --repo uyuutosa/pentaglyph-docs

# 5. Verify
npm view @uyuutosa/pentaglyph version
```

That's the full flow — **no NPM_TOKEN, no 2FA prompt, no WebAuthn**. The
GitHub Actions workflow exchanges its OIDC ID token for a short-lived npm
publish credential. Provenance attestation is generated automatically.

### Manual dispatch (re-publish current version)

If a publish fails partway through and you need to re-run with the same
version (rare — usually you should bump):

```bash
gh workflow run publish.yml \
  --repo uyuutosa/pentaglyph-docs \
  -f confirm_version=0.0.5
```

The workflow's verify step ensures `confirm_version` matches `cli/package.json`.

---

## Fallback: manual publish (when OIDC is unavailable)

Use this only if GitHub Actions is down, npm OIDC is having an outage, or the
trusted publisher hasn't been configured yet (first-publish chicken-and-egg).

### Steps

```bash
cd cli/

# 1. Log in to npm — interactive WebAuthn flow (opens browser, ~30s window)
npm login --auth-type=web

# 2. Verify
npm whoami

# 3. Sync template
bun run sync-template

# 4. Confirm tarball
npm pack --dry-run | tail -10

# 5. Publish (depending on your 2FA settings, may prompt for OTP / WebAuthn)
npm publish --access=public

# 6. Verify
npm view @uyuutosa/pentaglyph version
```

If the WebAuthn flow keeps timing out (~60s URL TTL is short for some setups),
you must use OIDC trusted publishing instead — see the previous section.

---

## Smoke test the published version

In a clean shell:

```bash
cd /tmp && rm -rf publish-test
bunx --bun @uyuutosa/pentaglyph init ./publish-test \
  --profile=standard --ai=claude --name="Publish Test"
ls publish-test/docs
test -f publish-test/.claude/agents/doc-orchestrator.md && echo "✓ agents shipped"
```

If this scaffolds the kit identically to a local `bun run`, the publish is healthy.

---

## Common pitfalls

| Symptom | Cause | Fix |
|---|---|---|
| `403 You must be logged in to publish packages` | Not logged in or wrong account | `npm login --auth-type=web` |
| `402 You must sign up for private packages` | Forgot `--access=public` on a scoped name | Re-run with `--access=public` |
| `EBADENGINE` (during install on user's machine) | User's Node < 20 | Ask user to upgrade Node, or install Bun |
| Published tarball missing `template/` | `prepack` didn't run | Verify `package.json` `scripts.prepack`; verify Bun on PATH |
| `template/` contains stale content | `cli/template/` wasn't re-synced after editing `../template/` | Run `bun run sync-template` before `npm publish` |
| OIDC workflow fails with `403` | Trusted publisher not configured on npm side | Re-check the npm Trusted Publisher settings; the workflow filename must match exactly |
| OIDC workflow fails with `npm version too old` | Older npm bundled with Node | The workflow upgrades npm to latest; if you copy this to another repo, ensure `npm install -g npm@latest` runs before publish |
| WebAuthn URL keeps timing out | URL TTL is ~60s; passkey browser flow exceeds that | Stop fighting it — switch to OIDC trusted publishing per above |
| Granular token "Bypass 2FA" greyed out / requires Organization | npm bug ([#8869](https://github.com/npm/cli/issues/8869)); personal accounts hit this | Don't use granular tokens — use OIDC instead |

---

## Unpublish window

npm allows `npm unpublish @uyuutosa/pentaglyph@0.0.5` **only within 72 hours** of publish, and only if no other package depends on it.

After 72 hours:

- You can `npm deprecate @uyuutosa/pentaglyph@0.0.5 "<reason>"` to discourage use.
- The version number is **permanently retired** — you cannot re-publish `0.0.5`. Bump to `0.0.6` instead.

---

## Future: claiming the unscoped `pentaglyph` name

When the API has been stable across 2–3 patch versions in real use:

1. Verify availability: `curl -s -o /dev/null -w '%{http_code}\n' https://registry.npmjs.org/pentaglyph` (404 means available).
2. Bump `cli/package.json`:
   - `"name": "pentaglyph"` (unscoped)
   - `"version": "0.1.0"`
3. Add `@uyuutosa/pentaglyph` as a deprecated alias:
   - Publish a final `@uyuutosa/pentaglyph@0.0.99` whose `package.json` has nothing but `"deprecated": "Use 'pentaglyph' instead"`.
4. Configure trusted publisher for the new unscoped package too (one-time, same flow as above).
5. Tag `pentaglyph-cli-v0.1.0` → push → workflow publishes both names.

---

## References

- npm publishing — <https://docs.npmjs.com/cli/commands/npm-publish>
- npm trusted publishing (OIDC) — <https://docs.npmjs.com/trusted-publishers/>
- npm trusted publishing GA changelog — <https://github.blog/changelog/2025-07-31-npm-trusted-publishing-with-oidc-is-generally-available/>
- Classic token retirement — <https://github.com/orgs/community/discussions/179562>
- semver — <https://semver.org/>
- npm unpublish policy — <https://docs.npmjs.com/policies/unpublish>
