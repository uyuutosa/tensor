"""T1 transposed contraction — Haliax translation.

Haliax's NamedArray carries axis names but the type system is JAX's
shape-trace, not C++ template instantiation: the mismatch fires at the
first jit/pjit call, not at edit time.

Expected outcome: CT (JAX trace-time). Without jit, the call would
succeed at the tracer level until shapes are concretised — Haliax
catches the mismatch via its first-class NamedArray axes.
"""
from __future__ import annotations

import haliax as hax
import jax
import jax.numpy as jnp
import numpy as np

batch = hax.Axis("batch", 2)
head = hax.Axis("head", 4)
time = hax.Axis("time", 8)
key_time = hax.Axis("key_time", 8)
d = hax.Axis("dim", 16)

# Upstream produced Q in (batch, time, head, dim) layout (bug premise).
Q_data = np.random.default_rng(42).standard_normal((2, 8, 4, 16))
Q = hax.named(jnp.asarray(Q_data), (batch, time, head, d))
K = hax.named(jnp.zeros((2, 4, 8, 16)), (batch, head, key_time, d))


@jax.jit
def attn(Q, K):
    # Programmer intent: contract over `time` from Q with `key_time` from K
    # via the `dim` shared axis. With the wrong Q layout, the named axes
    # do not align as written; Haliax's tracer refuses at jit time.
    return hax.dot(d, Q, K)  # dot-product over `dim`


_ = attn(Q, K)
