import torch
x = torch.zeros(32, 128, names=("batch", "feature"))
bias_T = torch.zeros(128, 1, names=("feature", "broadcast"))
# rename(None) drops names; named-tensor protection bypassed silently.
y = x.rename(None) + bias_T.T.rename(None)
print(y.shape)
