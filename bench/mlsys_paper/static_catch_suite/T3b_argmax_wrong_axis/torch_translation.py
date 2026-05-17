import torch
print(torch.argmax(torch.zeros(32, 10), dim=0).shape)
