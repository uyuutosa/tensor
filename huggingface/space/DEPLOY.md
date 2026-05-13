# Deploying this directory as a HuggingFace Space

This directory is a stand-alone Space project. The repo embeds it at
`huggingface/space/` so the source-of-truth lives next to the SDK that
the Space exhibits — the canonical deployment is to a separate Git remote
on `huggingface.co`.

## One-time setup (maintainer)

1. Create a Space on <https://huggingface.co/new-space> with:
   - SDK: **Gradio**
   - Hardware: **CPU basic** (free tier)
   - Visibility: **Public**
2. Clone the empty Space repo:
   ```bash
   git clone https://huggingface.co/spaces/<user>/tensor-named-axis-demo
   cd tensor-named-axis-demo
   ```
3. Copy the contents of this directory (`huggingface/space/*`) into the
   Space repo root, commit, and push to the `main` branch of the Space:
   ```bash
   cp -r /path/to/tensor/huggingface/space/. .
   git add README.md app.py requirements.txt packages.txt
   git commit -m "init: tensor named-axis demo"
   git push
   ```
4. HuggingFace will build the container (~3–5 minutes on first deploy:
   apt-installs the C++ toolchain via `packages.txt`, then pip-installs
   `tensor` from upstream Git which triggers the scikit-build-core /
   nanobind build).

## Updating

After each release of `tensor`, bump the pinned ref in `requirements.txt`
(e.g. `@v0.2.0`) and push. The Space will rebuild automatically.

Once `tensor-named-axis` is published to PyPI, switch the line to
`tensor-named-axis>=0.2.0` — that drops the C++ rebuild from cold-start
and removes the `packages.txt` dependency.

## Local smoke test before pushing

```bash
cd huggingface/space
python -m pip install -r requirements.txt
python app.py
# → browser opens to http://127.0.0.1:7860
```
