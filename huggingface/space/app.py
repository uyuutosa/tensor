"""Gradio app — interactive demo for the `tensor` Python SDK.

Three tabs cover the headline surfaces of Phase 6 (ADR-0018):

1. **TeX evaluator** — `tensor.tex.parse` + `Evaluator` end-to-end.
2. **Named-axis broadcast** — outer / inner / contract visualisation.
3. **Autograd** — `DynamicVariable` reverse-mode example.

The Space's `packages.txt` provisions a C++20 toolchain + CMake; the
`requirements.txt` then pip-installs the SDK from the upstream Git repo,
which triggers a scikit-build-core / nanobind build at container start.
"""

from __future__ import annotations

import io
import json
import traceback

import gradio as gr
import numpy as np

import tensor
import tensor.autograd as ag
import tensor.tex as tex


def _parse_csv_vector(s: str) -> np.ndarray:
    """Tolerant comma/space/newline-separated float vector parser."""
    if not s or not s.strip():
        return np.zeros(0, dtype=np.float64)
    cleaned = s.replace(",", " ").replace("\n", " ").split()
    return np.array([float(x) for x in cleaned], dtype=np.float64)


def run_tex(expr_src: str, a_csv: str, a_label: str, b_csv: str, b_label: str) -> tuple[str, str, str]:
    """Parse a LaTeX expression, bind a and b, evaluate, return result + AST round-trip."""
    try:
        expr = tex.parse(expr_src)
    except Exception as exc:  # parse error → user-facing message, no traceback
        return ("", "", f"parse error: {exc}")

    try:
        canonical = tex.to_latex(expr)
    except Exception as exc:  # to_latex shouldn't fail on a parsed AST but be safe
        canonical = f"<to_latex failed: {exc}>"

    a_vec = _parse_csv_vector(a_csv)
    b_vec = _parse_csv_vector(b_csv)

    ev = tex.Evaluator()
    if a_vec.size and a_label.strip():
        ev.bind("a", tensor.from_numpy(a_vec, [a_label.strip()]))
    if b_vec.size and b_label.strip():
        ev.bind("b", tensor.from_numpy(b_vec, [b_label.strip()]))

    try:
        result = ev.evaluate(expr)
    except Exception as exc:
        return (canonical, "", f"evaluation error: {exc}")

    arr = result.numpy()
    buf = io.StringIO()
    buf.write(f"shape rank: {result.shape.rank()}\n")
    buf.write(f"numpy:\n{np.array2string(arr, precision=4, suppress_small=True)}\n")
    return (canonical, buf.getvalue(), "")


def run_broadcast(a_csv: str, a_label: str, b_csv: str, b_label: str) -> str:
    """Add two rank-1 tensors and report the result. Same labels → element-wise; different → outer-sum."""
    try:
        a_vec = _parse_csv_vector(a_csv)
        b_vec = _parse_csv_vector(b_csv)
        if a_vec.size == 0 or b_vec.size == 0:
            return "both vectors must be non-empty"
        a = tensor.from_numpy(a_vec, [a_label.strip() or "i"])
        b = tensor.from_numpy(b_vec, [b_label.strip() or "j"])
        c = a + b
        buf = io.StringIO()
        buf.write(f"a labels: [{a_label!r}], shape: {a_vec.shape}\n")
        buf.write(f"b labels: [{b_label!r}], shape: {b_vec.shape}\n")
        buf.write(f"c = a + b, rank: {c.shape.rank()}\n")
        buf.write(f"numpy:\n{np.array2string(c.numpy(), precision=4, suppress_small=True)}\n")
        return buf.getvalue()
    except Exception as exc:
        return f"error: {exc}\n\n{traceback.format_exc()}"


def run_autograd(x_csv: str, lr: float, epochs: int) -> str:
    """Tiny scalar-loss demo: minimise sum((x - target)^2) over a rank-1 Variable."""
    try:
        x_init = _parse_csv_vector(x_csv)
        if x_init.size == 0:
            return "x must be non-empty"
        target = np.zeros_like(x_init)  # converge to zero, simplest possible loss surface

        x = ag.DynamicVariable(tensor.from_numpy(x_init.copy(), ["i"]))
        t = ag.DynamicVariable(tensor.from_numpy(target, ["i"]))

        log = []
        for epoch in range(int(epochs)):
            ag.zero_grad(x)
            diff = x - t
            loss = ag.sum_all(diff * diff)
            ag.backward(loss)
            ag.sgd_update(x, lr)
            if epoch == 0 or (epoch + 1) % max(1, int(epochs) // 8) == 0:
                log.append(f"epoch {epoch + 1:3d}: loss = {loss.value()[0]:.6f}")

        log.append(f"\nfinal x: {x.value().numpy()}")
        return "\n".join(log)
    except Exception as exc:
        return f"error: {exc}\n\n{traceback.format_exc()}"


with gr.Blocks(title="tensor — named-axis Einstein evaluator") as app:
    gr.Markdown(
        f"# `tensor` SDK demo\n\n"
        f"Phase 6 Python bindings for the [tensor named-axis differentiable C++ library]"
        f"(https://github.com/uyuutosa/tensor). "
        f"`tensor.__version__ = {tensor.__version__}`.\n\n"
        f"Each tab below talks to the same C++ Domain through nanobind — "
        f"the demo is the production build, not a Pythonic re-implementation."
    )

    with gr.Tab("TeX evaluator"):
        gr.Markdown(
            "Type a LaTeX expression and bind the named tensors `a` and `b`. "
            "Examples:\n\n"
            "- `c_{ij} = a_i b_j` (outer product — `a` with label `i`, `b` with label `j`)\n"
            "- `\\sum_i {a_i b_i}` (inner product — both with label `i`)\n"
            "- `a_i + b_i` (element-wise sum)\n"
            "- `a_i + b_j` (outer sum table)"
        )
        with gr.Row():
            with gr.Column():
                tex_input = gr.Textbox(label="LaTeX expression", value=r"c_{ij} = a_i b_j")
                a_vals = gr.Textbox(label="a values (comma/space-separated)", value="1, 2, 3")
                a_lab = gr.Textbox(label="a label", value="i")
                b_vals = gr.Textbox(label="b values (comma/space-separated)", value="10, 20")
                b_lab = gr.Textbox(label="b label", value="j")
                tex_run = gr.Button("Evaluate", variant="primary")
            with gr.Column():
                tex_canonical = gr.Textbox(label="parse → to_latex (canonical round-trip)", interactive=False)
                tex_output = gr.Textbox(label="result", lines=10, interactive=False)
                tex_error = gr.Textbox(label="error", interactive=False)
        tex_run.click(
            run_tex,
            inputs=[tex_input, a_vals, a_lab, b_vals, b_lab],
            outputs=[tex_canonical, tex_output, tex_error],
        )

    with gr.Tab("Named-axis broadcast"):
        gr.Markdown(
            "Add two rank-1 tensors. Same label → element-wise; different labels → outer sum."
        )
        with gr.Row():
            with gr.Column():
                br_a = gr.Textbox(label="a values", value="1, 2, 3, 4")
                br_a_l = gr.Textbox(label="a label", value="i")
                br_b = gr.Textbox(label="b values", value="10, 20")
                br_b_l = gr.Textbox(label="b label", value="j")
                br_btn = gr.Button("Compute a + b", variant="primary")
            with gr.Column():
                br_out = gr.Textbox(label="result", lines=12, interactive=False)
        br_btn.click(run_broadcast, inputs=[br_a, br_a_l, br_b, br_b_l], outputs=[br_out])

    with gr.Tab("Autograd"):
        gr.Markdown(
            "Minimise `sum((x - 0)^2)` over a rank-1 `DynamicVariable`. "
            "Reverse-mode tape lives in the C++ `tensor::autograd` Domain."
        )
        with gr.Row():
            with gr.Column():
                ag_x = gr.Textbox(label="initial x values", value="3, -2, 1.5, -0.5")
                ag_lr = gr.Slider(label="learning rate", minimum=0.001, maximum=1.0, value=0.1)
                ag_ep = gr.Slider(label="epochs", minimum=1, maximum=200, step=1, value=40)
                ag_btn = gr.Button("Train", variant="primary")
            with gr.Column():
                ag_out = gr.Textbox(label="training log", lines=14, interactive=False)
        ag_btn.click(run_autograd, inputs=[ag_x, ag_lr, ag_ep], outputs=[ag_out])

    gr.Markdown(
        "---\n\n"
        "Source: <https://github.com/uyuutosa/tensor> · "
        "Docs: <https://uyuutosa.github.io/tensor/> · "
        "Built with [nanobind](https://github.com/wjakob/nanobind) + "
        "[scikit-build-core](https://github.com/scikit-build/scikit-build-core)."
    )


if __name__ == "__main__":
    app.launch()
