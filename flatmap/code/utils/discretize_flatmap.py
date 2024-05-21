import flatmap_util as fmutil
import numpy as np
import sys

input_nrrd = sys.argv[1]
pixel_res = int(sys.argv[2])
output_nrrd = sys.argv[3]

fmap, fmask = fmutil.load_flatmap(input_nrrd)
fmap_d = fmutil.discretize_flatmap(fmap, fmask, pixel_res)
fmap.with_data(fmap_d).save_nrrd(output_nrrd)
