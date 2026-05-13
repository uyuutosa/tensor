---
status: Stable
owner: tensor
last-reviewed: 2026-05-14
---

# "The formula is the program" — what the slogan means

> **Diátaxis quadrant**: Explanation. The reader wants to understand the *idea*, not learn a syntax or solve a task. For hands-on, see [`../tutorials/cpp-named-axis-basics.md`](../tutorials/cpp-named-axis-basics.md) §Step 5. For the decision record, see [ADR-0005](../../arc42/09-decisions/0005-adopt-tex-lyx-as-authoring-surface.md).

## The slogan

Across the project — README, ADR-0005, the C++ tutorial 01, the Python notebook 02 — one phrase recurs: ***the formula is the program***. This document unpacks what that means and why it shapes the library.

## What it means literally

In `tensor`, you can write a tensor expression as a LaTeX string and have the compiler / runtime parse it into the same expression graph you'd build with C++ operators. The two are interchangeable representations of the same computation.

In C++ (compile-time UDL):

```cpp
auto expr = R"(c_{ij} = a_i b_j)"_tex;
Evaluator<double> ev;
ev.bind("a", a).bind("b", b);
auto c = ev.evaluate(expr);
```

In Python (`tensor.tex`):

```python
import tensor.tex as tex
expr = tex.parse(r"c_{ij} = a_i b_j")
ev = tex.Evaluator()
ev.bind("a", a)
ev.bind("b", b)
c = ev.evaluate(expr)
```

The expression `c_{ij} = a_i b_j` is *literally LaTeX* — paste it into a `\[ ... \]` environment in a math paper and it typesets identically. It's also a real parse tree with the same `Expression` AST type the in-source `_tex` UDL produces.

## Why this matters (the deeper idea)

Three things become possible when the formula and the program are the same artifact:

### 1. Citation surface for textbook authors

When a paper or textbook references `c_{ij} = a_i b_j`, a reader who knows `tensor` can transcribe the LaTeX *as written* and have working code. No "translate the math to NumPy" step. The math-literature anchor and the implementation share their syntax — they're the same file.

This is what [ADR-0015](../../arc42/09-decisions/0015-aspire-to-canonical-reference-quality-not-self-anoint.md) means by "citability discipline": public names map to math names, and now expressions in those names map to math expressions. Every public surface is one transcription away from a citation.

### 2. LyX integration — write in LyX, get code

`lyx-export/` exports a LyX document containing tensor formulas to a `.cpp` source file that compiles. The headline slogan goes the other direction too: write the math in your typesetting environment, and your typesetting *is* your source. ADR-0005 anchors this.

The implication is that `tensor` is a substrate for *authoring* and *executing* tensor computations in the same document. The 2016 maintainer's [original blog post](https://qiita.com/uyuutosa/items/12e87f4695bd151b1d74) hinted at this; ten years later the C++20 substrate makes it tractable.

### 3. No second DSL

Most tensor libraries with an Einstein-notation entry surface (`np.einsum`, `tf.einsum`, `jax.numpy.einsum`) have a *separate* string DSL with its own quirks. `'ij,jk->ik'` is positional letters, not the math the paper writes. The disagreement between the source string and the textbook prose is constant cognitive tax.

`tensor.tex` parses the textbook prose itself. There's no second DSL; the math *is* the DSL.

## What's actually implemented

| Feature                                                                                   | C++ surface                              | Python surface                                 |
| ----------------------------------------------------------------------------------------- | ---------------------------------------- | ---------------------------------------------- |
| Parse LaTeX subset → AST                                                                   | `R"(...)"_tex` (compile time) + `tex::parse(s)` (runtime) | `tensor.tex.parse(s)`                          |
| Render AST → canonical LaTeX                                                               | `tex::to_latex(expr)`                    | `tensor.tex.to_latex(expr)`                    |
| Bind named tensors to AST leaves + evaluate                                                | `tex::Evaluator<T>::bind` + `.evaluate`  | `tensor.tex.Evaluator().bind / .evaluate`      |
| Round-trip property `parse(to_latex(e)) == e`                                              | yes, in `tests/test_tex_parser.cpp`      | yes, in `python/tests/test_tex.py`             |

Supported AST nodes: `IndexedVar` (`a`, `a_i`, `c_{ij}`), `BinOp` (`+ - * /` + juxtaposition for implicit multiplication), `Sum` (`\sum_i {body}`), `Equation` (`lhs = rhs` with shape-annotated LHS), `Group` (`{...}`).

Parser expansion (`\prod`, `\delta_{ij}`, `\nabla`) is tracked as future work in [`../../detailed-design/tensor-tex.md` §8](../../detailed-design/tensor-tex.md).

## What this is NOT

- **Not a full LaTeX renderer**. `tensor::tex` parses a *subset* of LaTeX — the parts that express tensor algebra. `\sin`, `\cos`, `\int`, `\frac`, etc. are not in the supported corpus (yet).
- **Not a paper-to-executable compiler**. You still need to bind real tensors with `.bind` for the formula to produce a value. The parse is pure structure; the values come from C++ / Python side.
- **Not a CAS** (computer algebra system). There's no symbolic simplification; the parser produces the AST you wrote, not a normalised form. (Round-trip canonicalisation rewrites spacing and operator order but doesn't simplify.)

## Where the slogan fails (honest)

The `_tex` UDL is a *teaching* surface. It costs:

- **Compile-time UDL parse errors are LONG** — when you write a malformed formula, the compiler spits out hundreds of lines of template-instantiation noise before getting to the actual problem. The runtime `tex::parse(s)` form has cleaner error messages but loses the compile-time validation.
- **Performance is not the point**. The AST evaluator walks a tree; a hand-written C++ expression with the same operators is faster. Production users are pointed at the C++ surface; `_tex` is for citability, not for speed.
- **Discovery is harder than you'd think**. The first time a reader sees `R"(c_{ij} = a_i b_j)"_tex`, it doesn't *look* like an executable — it looks like a comment with quotes around it. The README + intro notebook lead with this so the surprise lands gracefully.

## Where to go next

- The C++ side, hands-on: [`../tutorials/cpp-named-axis-basics.md`](../tutorials/cpp-named-axis-basics.md) §Step 5.
- The Python side, hands-on: [`python/notebooks/02_python-tex.ipynb`](../../../python/notebooks/02_python-tex.ipynb).
- The decision: [ADR-0005](../../arc42/09-decisions/0005-adopt-tex-lyx-as-authoring-surface.md).
- The implementation HOW: [`../../detailed-design/tensor-tex.md`](../../detailed-design/tensor-tex.md).
- The maintainer's 2016 driver: <https://qiita.com/uyuutosa/items/12e87f4695bd151b1d74>.
