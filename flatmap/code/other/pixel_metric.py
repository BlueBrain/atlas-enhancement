import numpy as np
from sys import argv, exit, stderr

argc = len(argv)
file_rmean = argv[1]
fxlen = int(argv[2])

rmean = np.fromfile(file_rmean, dtype=np.float32).reshape((fxlen, fxlen))

vals = rmean[rmean != -1]  # sentinel for bad fit

mean = np.mean(vals)
std = np.std(vals)

print('{} {} {}'.format(fxlen,mean,std))
