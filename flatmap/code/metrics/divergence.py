import numpy as np
from voxcell import VoxelData
import sys

orient_x_nrrd = sys.argv[1]
orient_y_nrrd = sys.argv[2]
orient_z_nrrd = sys.argv[3]
mask_nrrd = sys.argv[4]
output_nrrd = sys.argv[5]

nx = VoxelData.load_nrrd(orient_x_nrrd)
ny = VoxelData.load_nrrd(orient_x_nrrd)
nz = VoxelData.load_nrrd(orient_x_nrrd)
mask = VoxelData.load_nrrd(mask_nrrd)

div = np.gradient(nx.raw, axis=0) +\
      np.gradient(ny.raw, axis=1) +\
      np.gradient(nz.raw, axis=2)
div[mask.raw != 1] = np.nan  # keep only interior values

nx.with_data(div.astype(np.float32)).save_nrrd(output_nrrd)
