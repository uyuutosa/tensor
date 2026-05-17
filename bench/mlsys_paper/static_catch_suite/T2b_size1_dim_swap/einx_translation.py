import einx, numpy as np
x = np.zeros((16, 64))
scale = einx.rearrange("c -> 1 c", np.zeros(64))
print((x + scale).shape)
