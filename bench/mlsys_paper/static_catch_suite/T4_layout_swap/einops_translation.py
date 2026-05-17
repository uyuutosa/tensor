"""T4 layout swap — einops translation.

Upstream returned (head, batch, dim). Downstream rearranges with a
pattern that assumed (batch, head, dim). When head == batch
coincidentally, the rearranged buffer is shape-correct, content-wrong.
"""
import einops
import numpy as np

H = B = 8         # picked to make the bug silent
D = 64
upstream = np.zeros((H, B, D))                        # (head, batch, dim)
view = einops.rearrange(upstream, "b h d -> b h d")  # NO-OP pattern,
                                                       # but writer thought (b, h, d)
print(view.shape)                                      # (8, 8, 64) — looks right
