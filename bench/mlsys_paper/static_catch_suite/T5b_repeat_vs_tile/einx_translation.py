import einx, numpy as np
base = np.zeros((32, 64))
print(einx.rearrange("t d -> (n t) d", base, n=2).shape)
