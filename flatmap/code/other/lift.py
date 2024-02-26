import numpy as np

from voxcell import VoxelData
from sys import argv, exit, stderr
from os import cpu_count

from tqdm import tqdm
from collections import defaultdict
import scipy.ndimage as nd

import flatmap_util as fmutil

argc = len(argv)
if argc < 6:
    print('Usage: {} <flatmap> <flat xlen> <data> <bkgval> <output> [<ncpu>]'.format(argv[0]),file = stderr)
    exit()

flatmap_file = argv[1]
fxlen = int(argv[2])

data_file = argv[3]
bkgval = float(argv[4])

out_file = argv[5]

ncpu = cpu_count() // 2
if argc > 6:
    ncpu = int(argv[6])

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

# unravel used pixel locations
ix = tuple([np.array([x % fxlen for x in label_unique]),\
            np.array([x // fxlen for x in label_unique])])

# load data
dat = np.fromfile(data_file, dtype=np.float32).reshape((fxlen,fxlen))
wdat = dat[ix]

del fx, fy, fmap, w, labels, lab  # FREE

# setup result image
res = np.full(vshape, bkgval, dtype=np.float32)

# set result
for i in tqdm(range(len(label_unique)), desc="Setting values"):
    vox = voxels[label_unique[i]]
    res[tuple(np.transpose(vox))] = wdat[i]

# write result image
fmap_vd.with_data(res).save_nrrd(out_file)
