---
status: Stable
owner: tensor
last-reviewed: 2026-05-14
---

# Python getting started — your first named-axis tensor in 10 minutes

> **Diátaxis quadrant**: Tutorial. The reader is a *learner*; the goal is to **produce a feeling of competence**, not to be exhaustive. We will write working code and explain it line by line. If you want a reference, see [`../reference/python-package-overview.md`](../reference/python-package-overview.md). If you want a how-to for a specific task, see [`../how-to/`](../how-to/).

Estimated time: 10 minutes (5 of install + 5 of code-along).

## What you'll have at the end

A Python script that creates a 5×2 named-axis tensor, adds another tensor to it across one named axis (Einstein-broadcast style), and prints the result. You'll understand:

- How `Axis` + `DynamicShape` describe a tensor's structure.
- Why `a + b` does Einstein-broadcast when the operands have disjoint axis labels.
- How to interop with NumPy.

## Step 1 — install

```bash
pip install tensor-named-axis
```

That's it. The wheel is precompiled; no C++ toolchain on your machine is required.

Verify the install:

```python
>>> import tensor
>>> tensor.hello()
'hello from tensor::core'
>>> tensor.__version__
'0.2.0'   # or whatever shipped
```

## Step 2 — first tensor

```python
import tensor

# A rank-1 tensor with 5 elements, named axis "i".
a = tensor.DynamicTensor(
    tensor.DynamicShape([tensor.Axis("i", 5)]),
    [1.0, 2.0, 3.0, 4.0, 5.0],
)
print(a)
```

You should see an ASCII-box rendering of the values labelled with the axis name `i`.

**What just happened?**

- `tensor.Axis("i", 5)` is the label–extent pair: this axis is called `"i"` and has 5 elements.
- `tensor.DynamicShape([...])` is the ordered list of axes. Rank = number of axes.
- `tensor.DynamicTensor(shape, values)` constructs the tensor; `values` is a flat row-major list.

If you change `5` to `4` in the shape but keep 5 elements in the value list, the constructor raises — the size check is the first line of defence.

## Step 3 — Einstein-broadcast addition

Make a second tensor with a *different* axis name:

```python
b = tensor.DynamicTensor(
    tensor.DynamicShape([tensor.Axis("j", 2)]),
    [10.0, 20.0],
)
print(b)

c = a + b
print(c)
print("rank:", c.shape.rank())   # → 2
```

The result `c` is **rank 2** — a 5×2 tensor — because the operands had **disjoint axis labels** (`i` vs `j`). This is named-axis broadcast: each `a_i + b_j` cell is computed.

Compare with **same-label** addition (rank-preserving):

```python
a2 = tensor.DynamicTensor(
    tensor.DynamicShape([tensor.Axis("i", 5)]),
    [10.0, 20.0, 30.0, 40.0, 50.0],
)
print(a + a2)   # still rank 1, element-wise
```

This is the headline feature: **the axis labels decide whether you get outer-broadcast or element-wise**.

## Step 4 — NumPy interop

```python
import numpy as np
import tensor

arr = np.arange(12, dtype=np.float64).reshape(3, 4)
t = tensor.from_numpy(arr, labels=["i", "j"])
print(t)

back = t.numpy()
print(np.allclose(arr, back))   # → True
```

`from_numpy` copies the NumPy array's data and attaches the supplied axis labels. `.numpy()` produces a fresh ndarray with the same data.

## Step 5 — the Einstein-sum contraction

```python
A = tensor.from_numpy(np.array([[1, 2], [3, 4]], dtype=np.float64), labels=["i", "j"])
B = tensor.from_numpy(np.array([[5, 6], [7, 8]], dtype=np.float64), labels=["j", "k"])
C = tensor.contract(A, B)
print(C)
print("rank:", C.shape.rank())  # → 2 (i, k)
```

`tensor.contract(A, B)` sums over all axes that appear in **both** operands — here, the shared `j` axis. The result has the remaining labels (`i` from A, `k` from B). This is `np.einsum("ij,jk->ik", A_np, B_np)` written with names instead of letters.

## Step 6 — where to go next

| If you want to…                                          | Read                                                                                         |
| -------------------------------------------------------- | -------------------------------------------------------------------------------------------- |
| See the same tensor types from C++                       | [`../tutorials/cpp-named-axis-basics.md`](./cpp-named-axis-basics.md)                       |
| Train a small model with autograd                        | [`python/notebooks/01_python-autograd.ipynb`](../../../python/notebooks/01_python-autograd.ipynb) on Colab |
| Write your formulas in LaTeX                              | [`python/notebooks/02_python-tex.ipynb`](../../../python/notebooks/02_python-tex.ipynb)     |
| Pick which tensor type to use                             | [`../how-to/named-tensor-types.md`](../how-to/named-tensor-types.md)                         |
| Understand *why* named axes are the headline             | [`../explanation/why-named-axes.md`](../explanation/why-named-axes.md)                       |
| See the full public Python surface                        | [`../reference/python-package-overview.md`](../reference/python-package-overview.md)         |

## What you did NOT learn here (on purpose)

- The Hexagonal-lite architecture under the hood. See [`../explanation/hexagonal-lite-rationale.md`](../explanation/hexagonal-lite-rationale.md).
- The full surface (only a small subset was used). See the reference.
- The C++ side. See the C++ tutorial above + the [Jupyter Book tutorials](https://uyuutosa.github.io/tensor/).

This tutorial intentionally lies-by-omission, per Diátaxis tutorial discipline — the goal was to make you feel competent, not exhaustively informed.
