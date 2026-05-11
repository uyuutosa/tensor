#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
#
# install_lyx_converter.sh — append the Tensor C++ converter lines to
# the user's LyX preferences file. Idempotent: running twice does not
# duplicate the entries.
#
# See LYX_PLUGIN.md for the manual installation instructions; this
# script automates the converter half. The custom Module is still a
# copy-and-reconfigure step.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PY_PATH="${SCRIPT_DIR}/lyx_to_tex.py"

# Detect the per-user LyX preferences path by OS. Default to the modern
# 2.4 directory layout; older installs may need manual adjustment.
case "$(uname -s)" in
    Linux*)
        PREF_FILE="${HOME}/.lyx/preferences"
        ;;
    Darwin*)
        PREF_FILE="${HOME}/Library/Application Support/LyX-2.4/preferences"
        ;;
    MINGW*|MSYS*|CYGWIN*)
        PREF_FILE="${APPDATA}/LyX2.4/preferences"
        ;;
    *)
        echo "error: unsupported OS '$(uname -s)' — append the two lines from LYX_PLUGIN.md by hand." >&2
        exit 2
        ;;
esac

mkdir -p "$(dirname "${PREF_FILE}")"
touch "${PREF_FILE}"

MARKER='# tensor::tex Tensor C++ converter (added by install_lyx_converter.sh)'
if grep -Fq "${MARKER}" "${PREF_FILE}"; then
    echo "Already installed in ${PREF_FILE}; no changes."
    exit 0
fi

cat >>"${PREF_FILE}" <<EOF

${MARKER}
\\format "tensor-cpp" "cpp" "Tensor C++ (_tex UDL)" "" "" "" "document"
\\converter "lyx" "tensor-cpp" "python3 ${PY_PATH} \$\$i --output \$\$o" ""
EOF

echo "Installed Tensor C++ converter into ${PREF_FILE}."
echo "Restart LyX. File → Export → Tensor C++ (_tex UDL) now exports via ${PY_PATH}."
