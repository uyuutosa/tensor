"""T2 silent broadcast — einops translation.

einops's string DSL has no concept of "this axis is the feature axis":
the dim names in the pattern are positional aliases for the operand
shape. A bias transposed from (128,) to (128, 1) with axis order
documented but not enforced will broadcast against the wrong axis;
einops's reshape DSL cannot refuse the bug.
"""
import einops
import numpy as np

x = np.zeros((32, 128))               # (batch, feature)
bias = np.zeros((128,))                # the correct shape
bias_buggy = einops.rearrange(bias, "f -> f ()").T   # (1, 128) — wrong axis
y = x + bias_buggy                                     # broadcasts against batch
print(y.shape)
