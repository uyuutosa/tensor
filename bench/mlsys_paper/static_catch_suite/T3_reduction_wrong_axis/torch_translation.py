import torch
logits = torch.zeros(16, 10, names=("batch", "classes"))
# Buggy dim index: writer meant dim="classes" but typed dim=0.
loss_per_class = logits.rename(None).sum(dim=0)  # collapses batch
print(loss_per_class.shape)
