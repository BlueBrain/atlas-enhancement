import numpy as np

from voxcell import VoxelData
import sys

import flatmap_util as fmutil

norm_eps = 1e-12

flatmap_nrrd = sys.argv[1]
orient_x_nrrd = sys.argv[2]
orient_y_nrrd = sys.argv[3]
orient_z_nrrd = sys.argv[4]
mask_nrrd = sys.argv[5]
output_nrrd = sys.argv[6]

# load flat map
fmap_vd, fmsk = fmutil.load_flatmap(flatmap_nrrd)
fmap = fmap_vd.raw
vshape = fmap.shape[0:3]

# load orientation vector
nx_vd = VoxelData.load_nrrd(orient_x_nrrd)
ny_vd = VoxelData.load_nrrd(orient_y_nrrd)
nz_vd = VoxelData.load_nrrd(orient_z_nrrd)

# load mask
mask_vd = VoxelData.load_nrrd(mask_nrrd)
mask_in = mask_vd.raw == 1  # 1 is interior

# get flat coordinates
fx = fmap[:,:,:,0]
fy = fmap[:,:,:,1]

## we use 0 as background value for gradient computation
fx[~fmsk] = 0.0
fy[~fmsk] = 0.0

# compute cartesian gradient of flat coordinates
grad_xflat = np.gradient(fx)
grad_yflat = np.gradient(fy)

dx_x = grad_xflat[0][fmsk]
dx_y = grad_xflat[1][fmsk]
dx_z = grad_xflat[2][fmsk]

dy_x = grad_yflat[0][fmsk]
dy_y = grad_yflat[1][fmsk]
dy_z = grad_yflat[2][fmsk]

# compute cross product of X and Y gradients
dx = np.vstack((dx_x, dx_y, dx_z)).T
dy = np.vstack((dy_x, dy_y, dy_z)).T

cross = np.cross(dx,dy)
norms = np.linalg.norm(cross, axis=1)
fmsk_norms = norms > norm_eps
cross[fmsk_norms] = cross[fmsk_norms] / norms[fmsk_norms, None]  # unit norm

# get orientation vectors
nx = nx_vd.raw[fmsk] * np.sign(nx_vd.voxel_dimensions[0])  # X axis direction
ny = ny_vd.raw[fmsk] * np.sign(ny_vd.voxel_dimensions[1])  # Y axis direction
nz = nz_vd.raw[fmsk] * np.sign(nz_vd.voxel_dimensions[2])  # Z axis direction

orient = np.vstack((nx,ny,nz)).T
norms = np.linalg.norm(orient,axis=1)
fmsk_norms = norms > norm_eps
orient[fmsk_norms] = orient[fmsk_norms] / norms[fmsk_norms,None]  # unit norm

# compute alignment between cross product and orientation vector
ortho = np.einsum('ij,ij->i', orient, cross)  # dot product

# setup result image
ortho3d = np.full(vshape, np.nan, dtype=np.float32)
ortho3d[fmsk] = ortho
ortho3d[~mask_in] = np.nan  # keep only interior values

# save result image
fmap_vd.with_data(ortho3d).save_nrrd(output_nrrd)
