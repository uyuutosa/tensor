import einx, numpy as np
H = B = 8
upstream = np.zeros((H, B, 64))           # (head, batch, dim)
view = einx.rearrange("b h d -> b h d", upstream)  # NO-OP; writer thought (b,h,d)
print(view.shape)
