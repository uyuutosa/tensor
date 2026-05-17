import einx, numpy as np
T = B = 32
upstream = np.zeros((T, B, 256))
print(einx.rearrange("b t d -> b t d", upstream).shape)
