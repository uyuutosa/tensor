import einx, numpy as np
A = np.zeros((3, 4)); B = np.zeros((4, 5))
print(einx.dot("i k, k j -> i j", A, B).shape)
