import numpy as np
from sys import argv, exit, stderr

argc = len(argv)
input_file = argv[1]
fxlen = int(argv[2])

data = np.fromfile(input_file, dtype=np.float32).reshape((fxlen, fxlen))

ngoodfit = np.sum(data >= 0.995)

print('{} {}'.format(fxlen,ngoodfit / data.size))
