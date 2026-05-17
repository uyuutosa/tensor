"""T1 transposed contraction — einx translation.

einx generalises the einops grammar with bracket sub-axis grouping but
preserves the same parse-at-runtime, no-host-type-check posture.
With shape-compatible dims (TIME == HEAD), the buggy pattern produces
wrong attention silently.
"""
import einx
import numpy as np

B, H, T, D = 2, 4, 4, 16   # T == H — silent bug
Q = np.random.default_rng(42).standard_normal((B, T, H, D))   # upstream (B,T,H,D)
K = np.random.default_rng(43).standard_normal((B, H, T, D))
scores = einx.dot("b h t d, b h s d -> b h t s", Q, K)
print(scores.shape)
