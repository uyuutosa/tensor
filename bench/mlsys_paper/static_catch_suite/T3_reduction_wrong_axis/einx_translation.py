import einx, numpy as np
logits = np.zeros((16, 10))
loss = einx.sum("b [c] -> b", logits)  # writer thought they collapsed b — Silent (loss shape (16,) not (10,))
print(loss.shape)
