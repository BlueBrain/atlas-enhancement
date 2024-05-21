from voxcell import VoxelData
import numpy as np
import nrrd
import sys

input_nrrd = sys.argv[1]
output_base = sys.argv[2]

vd = VoxelData.load_nrrd(input_nrrd)
nrrd.write('{}.nhdr'.format(output_base), vd.raw, detached_header='{}.bin.gz'.format(output_base))
