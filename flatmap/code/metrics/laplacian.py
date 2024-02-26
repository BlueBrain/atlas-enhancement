import numpy as np
from voxcell import VoxelData
import sys

from scipy.ndimage import laplace

rdepth_nrrd = sys.argv[1]
mask_nrrd = sys.argv[2]
output_nrrd = sys.argv[3]

rdepth = VoxelData.load_nrrd(rdepth_nrrd)
mask = VoxelData.load_nrrd(mask_nrrd)

lapl = laplace(rdepth.raw,mode='nearest')
lapl[mask.raw != 1] = np.nan  # keep only interior values

rdepth.with_data(lapl.astype(np.float32)).save_nrrd(output_nrrd)
