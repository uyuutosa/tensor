import einops, numpy as np
x = np.zeros((16, 64))
scale_wrong = einops.rearrange(np.zeros(64), "c -> 1 c")
y = x + scale_wrong
print(y.shape)
