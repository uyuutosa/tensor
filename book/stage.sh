#!/usr/bin/env bash
# Stage external content under book/ as symlinks so Sphinx's source tree
# (rooted at book/) can resolve _toc.yml paths into ../tutorials/,
# ../python/, ../docs/.
#
# Without this, Sphinx silently skips every notebook + every report
# chapter (etoc.ref "nonexisting document" warnings) — the deploy still
# succeeds but only book/{intro,architecture,roadmap}.html ship.
#
# Idempotent: re-running on an existing symlink is a no-op via `ln -sfn`.
# The three staged paths are .gitignore'd so they don't get committed.
#
# Run before `jupyter-book build book` locally and in CI.

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${REPO_ROOT}/book"

ln -sfn ../tutorials tutorials
ln -sfn ../python    python
ln -sfn ../docs      docs

echo "Staged: book/{tutorials, python, docs} -> ../<same>"
