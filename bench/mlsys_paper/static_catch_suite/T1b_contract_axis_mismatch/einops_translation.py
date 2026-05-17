"""T1b — einops matmul on labels that line up positionally but mean different things."""
import einops, numpy as np
A = np.zeros((3, 4))   # writer thinks (i, k)
B = np.zeros((4, 5))   # writer thinks (k, j)
out = einops.einsum(A, B, "i k, k j -> i j")
print(out.shape)
