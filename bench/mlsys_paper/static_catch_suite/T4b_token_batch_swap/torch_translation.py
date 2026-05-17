import torch
T = B = 32
print(torch.zeros(T, B, 256).view(B, T, 256).shape)
