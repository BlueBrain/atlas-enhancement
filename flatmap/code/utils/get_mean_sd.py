from voxcell import VoxelData
import numpy as np
import sys

file_nrrd = sys.argv[1]
pixel_res = int(sys.argv[2])

data = VoxelData.load_nrrd(file_nrrd)
vals = data.raw[~np.isnan(data.raw)]  # valid values are not NaN

mean = np.mean(vals)
std = np.std(vals)

print('{} {} {}'.format(pixel_res, mean, std))
