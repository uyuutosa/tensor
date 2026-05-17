import einx, numpy as np
x = np.zeros((32, 128))
bias_T = einx.rearrange("f -> f 1", np.zeros(128))
y = x + bias_T.T   # broadcasts (1,128) against batch — Silent.
print(y.shape)
