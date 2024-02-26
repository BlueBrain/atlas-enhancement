import hexalattice.hexalattice as hex
from voxcell import VoxelData
from scipy.spatial import KDTree
import numpy as np
import math
import sys

flatmap_nrrd = sys.argv[1]
output_nrrd = sys.argv[2]

hex_diam = 0.07
if len(sys.argv) > 3:
    hex_diam = float(sys.argv[3])

# rule of thumb
hex_n = math.floor(math.sqrt(2) / hex_diam)
if len(sys.argv) > 4:
    hex_n = int(sys.argv[4])

# load flatmap
fmap = VoxelData.load_nrrd(flatmap_nrrd)
vshape = fmap.shape[0:3]
msk = fmap.raw[:,:,:,0] > -1

# generate hexagonal grid
hexgrid = hex.create_hex_grid(hex_n,hex_n,hex_diam)[0] + (0.5,0.5)

# nearest hex for each flat coordinate
kdt = KDTree(hexgrid)
fpos = fmap.raw[msk]
nn = kdt.query(fpos, workers=-1)

# should not happen with our rule of thumb
if not np.all(nn[0] < hex_diam):
    raise ValueError('hex_n = {} was too small, please provide a larger value'.format(hex_n))

# get sequential hex ids
ix = nn[1]
uix = np.unique(ix)
map_ix = {x: i for i, x in enumerate(uix)}
ix_seq = [map_ix[x] for x in ix]

# store result
res = np.full(vshape, -1, dtype=np.int32)
res[msk] = ix_seq
fmap.with_data(res).save_nrrd(output_nrrd)
