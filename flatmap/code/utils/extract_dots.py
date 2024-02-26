import numpy as np
from voxcell import VoxelData
import sys

input_nrrd = sys.argv[1]
output_xyz = sys.argv[2]

vd = VoxelData.load_nrrd(input_nrrd)

w = np.array(np.where(vd.raw == 1)).T
pos = vd.indices_to_positions(w + 0.5)  # voxel centers
np.savetxt(output_xyz, pos, '%.12f')
