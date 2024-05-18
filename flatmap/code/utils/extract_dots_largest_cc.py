import numpy as np
from voxcell import VoxelData
from scipy.ndimage import generate_binary_structure, label
import sys

input_nrrd = sys.argv[1]
output_xyz = sys.argv[2]

vd = VoxelData.load_nrrd(input_nrrd)
ntot = np.sum(vd.raw)

# get connected components
ball = generate_binary_structure(3, 3)
cc = label(vd.raw, ball)

# keep only largest connected component
cc_uq, cc_cnt = np.unique(cc[0][cc[0] > 0], return_counts=True)
cc_imax = np.argmax(cc_cnt)
cc_umax = cc_uq[cc_imax]
cc_msk = (cc[0] == cc_umax)
nlargest = np.sum(cc_msk)
vd.raw[~cc_msk] = 0

print("Have {} connected components, keeping largest one with {} / {} ({:.2f}%) voxels".format(cc[1], nlargest, ntot, (nlargest / ntot) * 100))

w = np.array(np.where(vd.raw == 1)).T
pos = vd.indices_to_positions(w + 0.5)  # add 0.5 for voxel centers
np.savetxt(output_xyz, pos, '%.12f')
