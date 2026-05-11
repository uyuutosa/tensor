# Using `lyx_to_tex` as a LyX plugin

LyX has two extension surfaces; we use both to make the translator feel like a first-class plugin:

1. **Custom converter** — registers a new export format ("Tensor C++") that runs `lyx_to_tex.py` when the user picks **File → Export → Tensor C++**.
2. **Optional Module** — defines a custom inset (`Flex:TensorMath`) so a learner can mark math regions as "tensor algebra" and have them flow through the converter cleanly.

The converter alone is enough for everyday use; the Module is an ergonomic bonus.

---

## 1. Register the converter

LyX stores user preferences in a per-version directory:

| OS      | Path                                              |
| ------- | ------------------------------------------------- |
| Linux   | `~/.lyx/preferences`                              |
| macOS   | `~/Library/Application Support/LyX-2.4/preferences` |
| Windows | `%APPDATA%\LyX2.4\preferences`                    |

Append the two lines below (substituting the absolute path of `lyx_to_tex.py`):

```text
\format "tensor-cpp" "cpp" "Tensor C++ (_tex UDL)" "" "" "" "document"
\converter "lyx" "tensor-cpp" "python3 ABSOLUTE_PATH_TO/lyx_to_tex.py $$i --output $$o" ""
```

Restart LyX. **File → Export → Tensor C++ (_tex UDL)** now appears, and clicking it writes a `.cpp` file whose contents are exactly what `lyx_to_tex.py` prints.

### One-shot helper

```bash
# from the repo root, after `git clone`
./lyx-export/install_lyx_converter.sh
```

The helper detects the OS, appends the two lines to the right preferences file, and prints a "restart LyX" reminder. See the script for what it edits.

---

## 2. (Optional) Install the `Tensor Math` Module

Copy [`tensor-math.module`](./tensor-math.module) into LyX's user layout directory:

| OS      | Path                                              |
| ------- | ------------------------------------------------- |
| Linux   | `~/.lyx/layouts/`                                 |
| macOS   | `~/Library/Application Support/LyX-2.4/layouts/` |
| Windows | `%APPDATA%\LyX2.4\layouts\`                       |

Then in LyX: **Tools → Reconfigure**, restart, **Document → Settings → Modules** → enable "Tensor Math". A new `Flex:TensorMath` inset appears under **Insert → Custom Insets**; type your math in math mode and the converter picks it up.

The Module declaration just labels the inset so a future revision of `lyx_to_tex.py` can prioritise it over plain `Formula` insets; the MVP exporter does not require it.

---

## What ships, what doesn't

The converter and Module ship in this directory; **enabling them is a local LyX configuration step**, not something the repository can do for the user. The `install_lyx_converter.sh` helper automates the converter half; the Module install remains a copy-and-reconfigure.

If a learner doesn't have LyX installed, the translator still works as a CLI tool — see [`README.md`](./README.md) for the standalone usage.
