import numpy as np

from voxcell import VoxelData
from sys import argv, exit, stderr
from os import cpu_count

from tqdm import tqdm
from collections import defaultdict
import scipy.ndimage as nd

import flatmap_util as fmutil

argc = len(argv)
if argc < 3:
    print('Usage: {} <flatmap> <flat xlen> [<ncpu>]'.format(argv[0]),file = stderr)
    exit()

flatmap_file = argv[1]
fxlen = int(argv[2])

ncpu = cpu_count() // 2
if argc > 3:
    ncpu = int(argv[3])

# load flat map
fmap_vd = VoxelData.load_nrrd(flatmap_file)
fmap = fmap_vd.raw
assert(fmap.dtype == np.float32)  # we need float flatmap
vshape = fmap_vd.shape[0:3]

# discretize flatmap
fx, fy = fmutil.get_discrete_flat_coordinates(fmap, fxlen)

# labeled volume
labels = fx + fxlen * fy  # ravel all pixel locations
w = np.where(labels >= 0)
wno = np.where(labels < 0)
lab = labels[w]

# assign voxel indices to pre-images
voxels = defaultdict(list)
for i in tqdm(range(len(w[0])), desc="Getting pre-images"):
    voxels[lab[i]].append((w[0][i],w[1][i],w[2][i]))

label_unique = np.array(list(voxels.keys()))  # used flat pixels

del fx, fy, fmap, w, labels, lab  # FREE

# setup result images
res_x = np.zeros(vshape, dtype=np.uint16)
res_y = np.zeros(vshape, dtype=np.uint16)

# set results
for i in tqdm(range(len(label_unique)), desc="Setting values"):
    lab = label_unique[i]
    vox = voxels[lab]
    idx = tuple(np.transpose(vox))
    res_x[idx] = lab % fxlen
    res_y[idx] = lab // fxlen

# write result images
fmap_vd.with_data(res_x).save_nrrd('lift_x_{}.nrrd'.format(fxlen))
fmap_vd.with_data(res_y).save_nrrd('lift_y_{}.nrrd'.format(fxlen))
