import torch
H = B = 8
upstream = torch.zeros(H, B, 64, names=("head", "batch", "dim"))
# .view does not respect names; reshape with the wrong assumed layout.
view = upstream.rename(None).view(B, H, 64)
print(view.shape)
