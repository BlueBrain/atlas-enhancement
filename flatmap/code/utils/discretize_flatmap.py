import flatmap_util as fmutil
import numpy as np
import sys

input_nrrd = sys.argv[1]
pixel_res = int(sys.argv[2])
output_nrrd = sys.argv[3]

# smallest needed data type
if pixel_res > 2 ** 31 - 1:
    dtype = np.dtype('i8')
elif pixel_res > 2 ** 15 - 1:
    dtype = np.dtype('i4')
elif pixel_res > 2 ** 7 - 1:
    dtype = np.dtype('i2')
else:
    dtype = np.dtype('i1')

fmap, fmask = fmutil.load_flatmap(input_nrrd)
fx_d, fy_d = fmutil.get_discrete_flat_coordinates(fmap.raw, pixel_res, fmask)
fmap_d = np.stack((fx_d, fy_d), axis=-1)

assert(np.all((fmap_d[:,:,:,0] > -1) == fmask))
assert(np.all((fmap_d[:,:,:,1] > -1) == fmask))

fmap.with_data(fmap_d.astype(dtype)).save_nrrd(output_nrrd)
