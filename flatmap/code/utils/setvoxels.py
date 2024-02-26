from voxcell import VoxelData
import numpy as np
import sys

geom_nrrd = sys.argv[1]
data_txt = sys.argv[2]
output_nrrd = sys.argv[3]

dtype = np.float32
if len(sys.argv) > 4:
    dtype = np.dtype(sys.argv[4])

bkgval = -1.0
if len(sys.argv) > 5:
    bkgval = float(sys.argv[5])

vd = VoxelData.load_nrrd(geom_nrrd)
dat = np.loadtxt(data_txt, comments=None)

w = dat[:,[0,1,2]]
w = tuple(np.uint16(w).T)

h = np.full_like(vd.raw, bkgval, dtype=dtype)
h[w] = dat[:,3]

print('Setting {} voxels'.format(len(w[0])))

vd.with_data(h).save_nrrd(output_nrrd)
