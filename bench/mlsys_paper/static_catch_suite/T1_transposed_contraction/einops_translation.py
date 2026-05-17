"""T1 transposed contraction — einops translation.

Builds the multi-head attention scores. The bug is that the
contraction string lists `q_time` and `key_time` in the wrong
positions relative to how Q and K were laid out upstream.

einops's string-based notation cannot catch this at all: every
operand has a string label, the contraction expresses a valid
formula, and the produced tensor has a valid shape. The wrong
output is silent (Silent).
"""
from __future__ import annotations

import einops
import numpy as np

# TIME == HEAD so the (B,T,H,D) vs (B,H,T,D) layout swap is shape-
# compatible: einops cannot tell the bug from a feature.
BATCH, HEAD, TIME, D = 2, 4, 4, 16
rng = np.random.default_rng(42)

# Upstream produced Q in BTHD layout (a bug in this entry's premise).
Q = rng.standard_normal((BATCH, TIME, HEAD, D))
K = rng.standard_normal((BATCH, HEAD, TIME, D))

# Bug: the einsum string treats Q's axes as if they were BHTD.
scores = einops.einsum(Q, K, "b h t d, b h s d -> b h t s")
attn = scores / np.sqrt(D)
# Softmax over last axis.
attn = np.exp(attn - attn.max(-1, keepdims=True))
attn = attn / attn.sum(-1, keepdims=True)

print(attn.shape)  # (2, 4, 8, 8) — looks right, semantically wrong
