import numpy as np
from sys import argv

argc = len(argv)
flat_image = argv[1]
fxlen = int(argv[2])

data = np.fromfile(flat_image, dtype=np.float32)

assert(np.min(data) >= 0.0)

def flatmap_gini_curve(data):
    pixel_counts = sorted(data)
    cs = np.cumsum(np.flipud(pixel_counts)).astype(float) / np.sum(pixel_counts)
    return np.vstack((np.linspace(0, 1, len(cs)),cs)).T

def gini_coefficient(data):
    gc = flatmap_gini_curve(data)
    A = gc[:,0]
    B = gc[:,1]
    return np.sum(np.diff(A) * (B[:-1] + B[1:]) / 2.0)

print('{} {}'.format(fxlen, gini_coefficient(data)))
