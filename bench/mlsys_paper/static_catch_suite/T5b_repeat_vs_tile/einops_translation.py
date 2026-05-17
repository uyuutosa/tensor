import einops, numpy as np
base = np.zeros((32, 64))
tiled = einops.repeat(base, "t d -> (n t) d", n=2)
print(tiled.shape)
