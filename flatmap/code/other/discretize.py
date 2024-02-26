import voxcell as vc
import numpy as np

pixelSize = 10

vd = vc.VoxelData.load_nrrd('flatmap_centered_200um.nrrd')

x = vd.raw[:,:,:,0]
y = vd.raw[:,:,:,1]

xnan = np.isnan(x)
ynan = np.isnan(y)

xmin = np.min(x[~xnan])
ymin = np.min(y[~ynan])

x = np.floor((x - xmin) / pixelSize)
y = np.floor((y - ymin) / pixelSize)

x[xnan] = -1
y[ynan] = -1

vd.raw[:,:,:,0] = x
vd.raw[:,:,:,1] = y

vd.with_data(np.int16(vd.raw)).save_nrrd('flatmap_centered_200um_discrete_10um.nrrd')
