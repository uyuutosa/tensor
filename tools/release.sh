#!/usr/bin/env bash
# Lockstep version bumper for the tensor release ceremony.
#
# Updates every version-bearing file in one pass so the base
# distribution and its two companion projects (per ADR-0019) stay in
# sync. Mitigates the R-P6.5.1 drift risk.
#
# Usage:
#   tools/release.sh <semver>
#
# Examples:
#   tools/release.sh 0.3.0
#   tools/release.sh 0.3.1
#
# What it touches:
#   - vcpkg.json                  (version-string)
#   - CMakeLists.txt              (project VERSION)
#   - pyproject.toml              (project version + the three extras' lockstep pins)
#   - python/extras/tensor-named-axis-eigen/pyproject.toml   (project version + dep pin)
#   - python/extras/tensor-named-axis-webgpu/pyproject.toml  (project version + dep pin)
#
# What it does NOT touch (manual step):
#   - CHANGELOG.md   — the [Unreleased] → [<semver>] promotion is a
#                      semantic edit (writing the release summary) that
#                      needs human judgement. The script prints the
#                      reminder; the maintainer follows
#                      `docs/design-guide/release-ceremony.md` §2 step 3.

set -euo pipefail

if [[ $# -ne 1 ]]; then
  echo "usage: $0 <semver>" >&2
  echo "example: $0 0.3.0" >&2
  exit 2
fi

NEW_VERSION="$1"

# Strict semver-ish check (allows post-release suffixes like
# `0.3.0.post1` and pre-release `0.3.0rc1`).
if ! [[ "${NEW_VERSION}" =~ ^[0-9]+\.[0-9]+\.[0-9]+([.+~-][[:alnum:].]+)?$ ]]; then
  echo "error: version '${NEW_VERSION}' doesn't look like a semver" >&2
  exit 1
fi

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${REPO_ROOT}"

echo "==> Bumping to ${NEW_VERSION}"

# 1. vcpkg.json
sed -i.bak -E "s/(\"version-string\":[[:space:]]*\")[^\"]+(\")/\1${NEW_VERSION}\2/" vcpkg.json
rm vcpkg.json.bak
echo "    vcpkg.json updated"

# 2. CMakeLists.txt — match only the indented `VERSION X.Y.Z` line
# inside `project(tensor ...)`. Avoids matching the
# `cmake_minimum_required(VERSION 3.25)` line on line 1.
sed -i.bak -E "s/^([[:space:]]+VERSION[[:space:]]+)[0-9][0-9a-zA-Z.+~-]*/\1${NEW_VERSION}/" CMakeLists.txt
rm CMakeLists.txt.bak
echo "    CMakeLists.txt updated"

# 3. Base pyproject.toml
#    - The project's own version
sed -i.bak -E "s/(^version[[:space:]]*=[[:space:]]*\")[^\"]+(\")/\1${NEW_VERSION}\2/" pyproject.toml
#    - The optional-dependencies lockstep pins on the companion projects
sed -i.bak -E "s/(tensor-named-axis-eigen==)[^\"]+(\")/\1${NEW_VERSION}\2/" pyproject.toml
sed -i.bak -E "s/(tensor-named-axis-webgpu==)[^\"]+(\")/\1${NEW_VERSION}\2/" pyproject.toml
rm pyproject.toml.bak
echo "    pyproject.toml updated (project version + 2 extras pins)"

# 4. Companion projects
for companion in tensor-named-axis-eigen tensor-named-axis-webgpu; do
  path="python/extras/${companion}/pyproject.toml"
  #   - Companion's own version
  sed -i.bak -E "s/(^version[[:space:]]*=[[:space:]]*\")[^\"]+(\")/\1${NEW_VERSION}\2/" "${path}"
  #   - The lockstep dep pin on the base distribution
  sed -i.bak -E "s/(tensor-named-axis==)[^\"]+(\")/\1${NEW_VERSION}\2/" "${path}"
  rm "${path}.bak"
  echo "    ${path} updated"
done

# 5. python/tensor/__init__.py — version string in the example block
#    (cosmetic; the runtime __version__ comes from the native module).
#    Left alone here; the native module's attribute is set by the C++
#    side reading pyproject metadata via setuptools-scm-style or the
#    explicit string in src/_tensor_native.cpp. Update that separately
#    if the embedded version string in C++ changes.

echo ""
echo "==> Reminder: CHANGELOG.md needs the [Unreleased] → [${NEW_VERSION}] promotion."
echo "    Follow docs/design-guide/release-ceremony.md §2 step 3."
echo ""
echo "==> Done. Suggested next steps:"
echo "    git diff --stat"
echo "    git add -p vcpkg.json CMakeLists.txt pyproject.toml python/extras/"
echo "    # ...also stage CHANGELOG.md after the manual edit..."
echo "    git commit -m 'release: ${NEW_VERSION}'"
