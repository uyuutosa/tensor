import torch
token0 = torch.zeros(16, 64, names=("token", "dim"))
token1 = torch.zeros(16, 64, names=("token", "dim"))
result = torch.cat([token0, token1], dim=0)   # named cat: produces (32, 64)
print(result.shape)
