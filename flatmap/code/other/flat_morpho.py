import numpy as np
from voxcell import VoxelData
import sys


def flatten_morpho(flatmap, points, reflect=False, interp=False, include_oob=False):
    """
    Get flat coordinates of 3D morphology points
    Parameters
    flatmap : VoxelData object with flatmap
    points  : array of morphology points in atlas space
    reflect : reflect morphology points w.r.t. midline (hemisphere flip)
    interp  : use linear interpolation of flat positions (requires scipy, is slow, has edge artifacts)
    include_oob : output includes points that are out-of-bounds (keeps input length)
    """

    if reflect:
        zmax = flatmap.bbox[1,2]
        points[:,2] = zmax - points[:,2]

    if interp:
        from scipy.interpolate import interpn
        mask = flatmap.raw[:,:,:,0] > -1  # mapped
        w = np.where(mask)
        bounds = (range(np.min(w[0]), np.max(w[0]) + 1),
                  range(np.min(w[1]), np.max(w[1]) + 1),
                  range(np.min(w[2]), np.max(w[2]) + 1))
        allpos = [bounds[i] * flatmap.voxel_dimensions[i] + flatmap.offset[i] for i in range(0,3)]
        allval = flatmap.raw[bounds[0],:,:][:,bounds[1],:][:,:,bounds[2]]
        flatpoints = interpn(allpos, allval, points, method='linear', bounds_error=False, fill_value=-1)
    else:
        flatpoints = flatmap.lookup(points, outer_value=np.array([-1, -1]))

    if not include_oob:
        mask = (flatpoints[:,0] > -1) & (flatpoints[:,1] > -1)
        flatpoints = flatpoints[mask]

    return flatpoints


flatmap_nrrd = sys.argv[1]
flatpos_xyz = sys.argv[2]
output_nrrd = sys.argv[3]

reflect = True
if len(sys.argv) > 4:
    reflect = bool(sys.argv[4])

interp = False
if len(sys.argv) > 5:
    interp = bool(sys.argv[5])

include_oob = True
if len(sys.argv) > 6:
    include_oob = bool(sys.argv[6])

fmap = VoxelData.load_nrrd(flatmap_nrrd)

pos = np.loadtxt(flatpos_xyz)
pos = pos.reshape((-1,3))

fpos = flatten_morpho(fmap, pos, reflect=reflect, interp=interp, include_oob=include_oob)

np.savetxt(output_nrrd,fpos,"%.16f")
