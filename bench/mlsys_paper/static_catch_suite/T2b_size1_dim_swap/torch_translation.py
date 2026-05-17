import torch
x = torch.zeros(16, 64); scale = torch.zeros(1, 64)
print((x + scale).shape)
