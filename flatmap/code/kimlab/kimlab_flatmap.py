# Run this script after...
## Extracting NIfTI datasets from 'flat_map_data_10um_v2.mat' with 'extract_flatmap.m'
## Removing one-voxel padding all around in xgrid3d (or similar)
## Expanding X to cover both hemispheres in xgrid3d (or similar)
## Converting to NRRD in xgrid3d (or similar)
## Transposing and Z flipping in Python (see Makefile)
import numpy as np
from voxcell import VoxelData
import sys

xflat_nrrd = sys.argv[1]
yflat_nrrd = sys.argv[2]
outfile = sys.argv[3]

fx = VoxelData.load_nrrd(xflat_nrrd)
fy = VoxelData.load_nrrd(yflat_nrrd)

# Set range to [0,1]
ffx = fx.raw.astype(np.float32)
ffy = fy.raw.astype(np.float32)
ffx = ffx - 1  # start at 0
ffy = ffy - 1  # start at 0
norm = np.max([np.max(ffx), np.max(ffy)])
ffx[ffx > -1] = ffx[ffx > -1] / norm
ffy[ffy > -1] = ffy[ffy > -1] / norm
np.all((ffx > -1) == (ffy > -1))

# Save flatmap
fmap = np.zeros(list(fx.shape) + [2],dtype=np.float32)
## swap X and Y
fmap[:,:,:,0] = ffy
fmap[:,:,:,1] = ffx
fx.voxel_dimensions = [10,10,10]  # 10 um
fx.with_data(fmap).save_nrrd(outfile)
