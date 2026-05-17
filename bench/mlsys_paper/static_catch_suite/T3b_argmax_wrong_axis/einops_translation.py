import einops, numpy as np
logits = np.zeros((32, 10))
result = einops.reduce(logits, "b c -> c", "max")
print(result.shape)
