"""T5 stack vs concat — einops translation.

einops has both `rearrange` (which reshapes) and `repeat`/`pack`/
`unpack` for axis introduction. Mistakenly using `np.concatenate`
with the wrong axis produces a rank-r tensor instead of rank-(r+1);
the bug propagates silently several operators downstream.
"""
import numpy as np

token0 = np.zeros((16, 64))         # (token, dim)
token1 = np.zeros((16, 64))         # (token, dim)
# Wanted: stack into (2, token, dim). Wrote: concat along token-axis.
result = np.concatenate([token0, token1], axis=0)   # (32, 64)
print(result.shape)                                   # rank-2, not rank-3
