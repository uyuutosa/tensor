#!/usr/bin/env bash
# Deploy `huggingface/space/` to a HuggingFace Space.
#
# Prerequisites:
#   - `hf` (or legacy `huggingface-cli`) installed:
#       pip install -U huggingface_hub
#   - Logged in:
#       hf auth login    # paste a token with `write` scope from
#                        # https://huggingface.co/settings/tokens
#
# Usage:
#   ./huggingface/space/deploy.sh <hf-username-or-org> [<space-name>]
#
# Defaults:
#   space-name = tensor-named-axis-demo
#
# What it does:
#   1. Creates the Space (idempotent — re-running on an existing Space is a no-op).
#   2. Clones the Space repo to a temp directory.
#   3. Copies this directory's contents (sans deploy.sh + DEPLOY.md) into it.
#   4. Commits + pushes to the Space's `main` branch.
#   5. Prints the live Space URL.

set -euo pipefail

if [[ $# -lt 1 ]]; then
  echo "usage: $0 <hf-username-or-org> [<space-name>]" >&2
  exit 2
fi

HF_USER="$1"
SPACE_NAME="${2:-tensor-named-axis-demo}"
REPO_ID="${HF_USER}/${SPACE_NAME}"
SPACE_URL="https://huggingface.co/spaces/${REPO_ID}"

THIS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "==> Creating Space ${REPO_ID} (--type space --space-sdk gradio --exist-ok)"
hf repos create "${REPO_ID}" --type space --space-sdk gradio --exist-ok

TMP="$(mktemp -d)"
trap 'rm -rf "${TMP}"' EXIT

echo "==> Cloning ${SPACE_URL} into ${TMP}"
git clone "${SPACE_URL}" "${TMP}/space"

echo "==> Copying Space contents (sans deploy.sh / DEPLOY.md)"
cp "${THIS_DIR}/README.md"        "${TMP}/space/README.md"
cp "${THIS_DIR}/app.py"           "${TMP}/space/app.py"
cp "${THIS_DIR}/requirements.txt" "${TMP}/space/requirements.txt"
cp "${THIS_DIR}/packages.txt"     "${TMP}/space/packages.txt"

echo "==> Committing + pushing"
cd "${TMP}/space"
git add README.md app.py requirements.txt packages.txt
if git diff --cached --quiet; then
  echo "    nothing to commit (Space already up to date)"
else
  git commit -m "deploy: sync from upstream tensor repo $(date -u +%Y-%m-%dT%H:%M:%SZ)"
  git push origin main
fi

echo ""
echo "==> Done. Space: ${SPACE_URL}"
echo "    First build will take ~3-5 min (apt installs + nanobind C++ build)."
echo "    Watch progress in the Space's 'App' tab → 'Logs'."
