from voxcell import VoxelData
import flatmap_util as fmutil
import numpy as np
import sys

flatmap_nrrd = sys.argv[1]
hemi_nrrd = sys.argv[2]
output_nrrd = sys.argv[3]

xmax = 1.0
if len(sys.argv) > 4:
    xmax = float(sys.argv[4])

# load
fmap, _ = fmutil.load_flatmap(flatmap_nrrd)
hemi = VoxelData.load_nrrd(hemi_nrrd)

# define masks
msk = (hemi.raw == 1)
msko = (hemi.raw == 2)
w = np.where(msk)
wo = np.where(msko)
mw = np.array(w).T
mwo = np.array(w).T
mwo[:,2] = fmap.shape[2] - 1 - mwo[:,2]

# check exact symmetry
assert(np.sum(msko[tuple(mwo.T)]) == np.sum(msko[wo]))

# mirror data: [] [0,1] -> [1,0] [0,1]
fmap.raw[tuple(mwo.T)] = fmap.raw[tuple(mw.T)]

# shift right X coordinate: [1,0] [0,1] -> [1,0] [1,2]
mwx = np.array(w).T
mwx = np.vstack((mwx.T,np.zeros(mwx.shape[0]))).T.astype(int)
newx = fmap.raw[tuple(mwx.T)]
newx[newx > -1] = newx[newx > -1] + xmax
fmap.raw[tuple(mwx.T)] = newx

# reflect left X coordinate: [1,0] [1,2] -> [0,1] [1,2]
mwxo = np.array(wo).T
mwxo = np.vstack((mwxo.T,np.zeros(mwxo.shape[0]))).T.astype(int)
newxo = fmap.raw[tuple(mwxo.T)]
newxo[newxo > -1] = xmax - newxo[newxo > -1]
fmap.raw[tuple(mwxo.T)] = newxo

# save
fmap.save_nrrd(output_nrrd)
