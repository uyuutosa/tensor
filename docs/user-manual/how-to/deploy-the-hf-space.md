---
status: Stable
owner: tensor
last-reviewed: 2026-05-14
---

# How to deploy the HuggingFace Space

> **Diátaxis quadrant**: How-to. Task-oriented; the reader is the *maintainer* (or someone with the maintainer's PyPI / HF auth). For the conceptual frame, see [`huggingface/space/README.md`](../../../huggingface/space/README.md). For the script: [`huggingface/space/deploy.sh`](../../../huggingface/space/deploy.sh).

The HuggingFace Space at `huggingface.co/spaces/<user>/tensor-named-axis-demo` is a free hosted Gradio demo of the Python SDK. This guide is the runbook for getting the in-tree scaffold pushed to a live Space.

## Prerequisites

- An HF account (free tier works for CPU-basic hardware).
- The `huggingface_hub` Python package (`hf` CLI), version 0.20+ (`pip install -U huggingface_hub`).
- Git installed locally.
- The maintainer's authentication — write-scoped token from <https://huggingface.co/settings/tokens>.

## Step 1 — authenticate

```bash
hf auth login
```

The CLI prompts for a token. Paste a token with **write** scope. When asked "Add token as git credential? (Y/n)", choose **Y** so the subsequent `git push` to the Space repo works without re-auth.

Verify:

```bash
hf auth whoami
# → your-hf-username
```

## Step 2 — run the deploy script

```bash
./huggingface/space/deploy.sh <hf-username-or-org> [<space-name>]
```

Examples:

```bash
./huggingface/space/deploy.sh uyuutosa                 # uses default slug "tensor-named-axis-demo"
./huggingface/space/deploy.sh uyuutosa my-fork-demo    # custom slug
```

What it does (per the script):

1. `hf repos create <user>/<slug> --type space --space-sdk gradio --exist-ok` — creates the Space (no-op if it already exists).
2. `git clone https://huggingface.co/spaces/<user>/<slug>` into a tmp directory.
3. Copies the four Space files (`README.md`, `app.py`, `requirements.txt`, `packages.txt`) into the cloned repo. (`DEPLOY.md` and `deploy.sh` are intentionally NOT pushed — they're tooling for this repo, not the Space.)
4. Commits + pushes to the Space's `main` branch.
5. Prints the live Space URL.

First push triggers a ~3–5 minute build:

- `packages.txt` installs apt deps (`build-essential`, `cmake`, `ninja-build`) — the C++20 toolchain.
- `requirements.txt` pip-installs `tensor` from upstream Git (`git+https://github.com/uyuutosa/tensor.git@develop`), which triggers a scikit-build-core / nanobind compile against the C++ headers.
- Gradio launches `app.py` on the Space's CPU runner.

You can watch the build in the Space's "App" tab → "Logs".

## Step 3 — verify the live Space

Browse to `https://huggingface.co/spaces/<user>/<slug>`. You should see the three Gradio tabs:

1. **TeX evaluator** — type `c_{ij} = a_i b_j`, set values for `a` and `b`, click Evaluate.
2. **Named-axis broadcast** — element-wise vs outer-broadcast demo.
3. **Autograd** — small SGD training loop on a `DynamicVariable`.

If any tab errors, check the build logs first; the most common cause is a nanobind / scikit-build-core version drift between HF Spaces' Python and the upstream `develop`.

## Updating after a release

After a `tensor` release (e.g. `0.2.0` → PyPI), update the Space to pull from PyPI instead of upstream Git:

```python
# huggingface/space/requirements.txt
tensor-named-axis>=0.2.0
gradio==4.44.0
numpy>=1.20
```

After Phase 6.5, pin the extras:

```python
# Use GPU on a paid HF Spaces hardware tier:
tensor-named-axis[webgpu]>=0.3.0
gradio==4.44.0
numpy>=1.20
```

When `requirements.txt` collapses to `pip install tensor-named-axis`, the `packages.txt` (apt deps for the C++ build) becomes unnecessary — delete it. Cold-start drops from ~5 min to ~10 s.

Run `./huggingface/space/deploy.sh <user>` after the edit; the script re-syncs the four files and pushes.

## Maintainer hand-off

Once the Space is live, surface the URL:

1. Edit [`README.md`](../../../README.md) §Documentation — replace the placeholder "`huggingface.co/spaces/<maintainer>/tensor-named-axis-demo` (link will be added here once the Space is published)" with the live URL.
2. Add the same URL to `huggingface/space/README.md`'s header.
3. Cross-link from `docs/INDEX.md` if it isn't already there.

## Troubleshooting

| Symptom                                                                   | Likely cause / fix                                                                              |
| ------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------- |
| `hf repos create` fails with `403 Forbidden`                              | Token doesn't have write scope. Regenerate at <https://huggingface.co/settings/tokens>.        |
| `git push` fails after `hf auth login`                                    | The credential helper missed. Run `git config --global credential.helper store` once.           |
| Build hangs at "Building wheel for tensor-named-axis"                     | scikit-build-core's CMake invocation is slow; ~3–5 min cold-start is normal. Watch the logs.    |
| Gradio app launches but `tensor.set_backend("webgpu")` errors at runtime  | HF Spaces' free CPU tier doesn't have a GPU. Use a paid hardware tier, or stick with reference. |
| `requirements.txt` install fails on `tensor-named-axis>=X.Y.Z`             | The release hasn't propagated to PyPI's CDN yet. Wait 5–10 min, retry.                          |

## Cross-references

- The Space scaffold: [`huggingface/space/`](../../../huggingface/space/).
- The deploy script: [`huggingface/space/deploy.sh`](../../../huggingface/space/deploy.sh).
- The Space's `DEPLOY.md` (which this how-to supersedes): [`huggingface/space/DEPLOY.md`](../../../huggingface/space/DEPLOY.md).
- Phase 6 retrospective (the Space scaffold's introducing context): [`../../reports/2026-05-13_phase-6-python-sdk-retrospective.md`](../../reports/2026-05-13_phase-6-python-sdk-retrospective.md).
- arc42 §7 §4c — deployment view that names this surface: [`../../arc42/07-deployment/overview.md`](../../arc42/07-deployment/overview.md).
