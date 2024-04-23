from voxcell import VoxelData
import numpy as np
import sys

nrrd_file = sys.argv[1]
output_file = sys.argv[2]

vd = VoxelData.load_nrrd(nrrd_file)
n_notnan = np.sum(~np.isnan(vd.raw))
shape_str = " ".join([str(x) for x in vd.raw.shape])
voxdim_str = " ".join(["{:.8f}".format(x) for x in vd.voxel_dimensions])
origin_str = " ".join(["{:.8f}".format(x) for x in vd.offset])

with open(output_file,"w") as f:
    print(n_notnan, file=f)
    print(shape_str, file=f)
    print(voxdim_str, file=f)
    print(origin_str, file=f)
