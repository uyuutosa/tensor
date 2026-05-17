"""T1 transposed contraction — torch.named_tensor translation.

PyTorch's named tensors carry dimension names but the named-tensor
operator coverage is partial; many operations strip names silently.
With TIME == HEAD the shape is compatible and matmul does not
catch the layout swap.
"""
import torch
B, H, T, D = 2, 4, 4, 16
Q = torch.randn(B, T, H, D, names=("batch", "time", "head", "dim"))
K = torch.randn(B, H, T, D, names=("batch", "head", "time", "dim"))
# Buggy einsum-equivalent: the caller wrote it assuming Q is (B,H,T,D).
# matmul on named tensors does NOT enforce name alignment in many
# overloads; we drop names to invoke the unnamed kernel.
scores = (Q.rename(None) @ K.rename(None).transpose(-2, -1)) / (D ** 0.5)
print(scores.shape)
