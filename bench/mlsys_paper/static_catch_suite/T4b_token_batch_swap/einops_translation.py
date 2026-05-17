import einops, numpy as np
T = B = 32
upstream = np.zeros((T, B, 256))
view = einops.rearrange(upstream, "b t d -> b t d")
print(view.shape)
