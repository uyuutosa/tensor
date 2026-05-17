import einx, numpy as np
logits = np.zeros((32, 10))
print(einx.max("[b] c", logits).shape)
