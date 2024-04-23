import numpy as np


def load_flatmap(file):
    from voxcell import VoxelData
    vd = VoxelData.load_nrrd(file)

    # mask for mapped values
    fx_msk = vd.raw[:,:,:,0] > -1
    fy_msk = vd.raw[:,:,:,1] > -1
    assert(np.all(fx_msk == fy_msk))  # must coincide

    return vd, fx_msk


def discretize(pos, fxlen):
    fpos = pos.astype(np.float64)
    almost_fxlen = fxlen - 1E-5
    dpos = np.floor(fpos * almost_fxlen).astype(np.int64)
    assert(np.max(dpos) < np.max(fpos) * fxlen)
    assert(np.min(dpos) >= -1)
    return dpos


def get_discrete_flat_coordinates(fmap, fxlen, fmask = None):
    fx = fmap[:,:,:,0]  # flat X coordinate in [0,1]
    fy = fmap[:,:,:,1]  # flat Y coordinate in [0,1]

    # mask for mapped values
    if fmask is None:
        fx_msk = fx > -1
        fy_msk = fy > -1
        assert(np.all(fx_msk == fy_msk))  # must coincide
        fmask = fx_msk

    return discretize(fx[fmask], fxlen), discretize(fy[fmask], fxlen)


def get_preimage_mask_vox(vox):
    nvox = len(vox)
    # handle empty pre-images (holes)
    if nvox == 0:
        return None, 0

    avox = np.array(vox)

    wmin = [np.min(avox[:,0]), np.min(avox[:,1]), np.min(avox[:,2])]
    wmax = [np.max(avox[:,0]), np.max(avox[:,1]), np.max(avox[:,2])]
    lmsk = np.zeros((wmax[0] - wmin[0] + 1,wmax[1] - wmin[1] + 1,wmax[2] - wmin[2] + 1))
    lmsk[tuple([avox[:,0] - wmin[0],avox[:,1] - wmin[1],avox[:,2] - wmin[2]])] = 1  # NOTE: must be 1

    return lmsk, nvox


def get_pixel_labels(fmap, pixel_res):
    fx = fmap[:,:,:,0].astype(np.int64)
    fy = fmap[:,:,:,1].astype(np.int64)

    # labeled volume
    labels = fx + pixel_res * fy  # ravel all pixel locations
    w = np.where(labels >= 0)  # mapped
    assert(len(w[0]) == np.sum(fx > -1))
    lab = labels[w]

    return w, lab


def get_preimages_vox(fmap_vd, pixel_res):
    from collections import defaultdict
    from tqdm import tqdm

    w, lab = get_pixel_labels(fmap_vd.raw, pixel_res)

    # assign voxel indices to pre-images
    voxels = defaultdict(list)
    for i in tqdm(range(len(lab)), desc="Getting pre-images"):
        voxels[lab[i]].append((w[0][i],w[1][i],w[2][i]))

    return voxels


def get_preimages_pts(fmap_vd, pixel_res):
    from collections import defaultdict
    from tqdm import tqdm

    w, lab = get_pixel_labels(fmap_vd.raw, pixel_res)

    # positions of mapped voxels
    pos = fmap_vd.indices_to_positions(np.array(w).T)  # positions in atlas units

    # assigning points to pixel pre-images
    points = defaultdict(list)
    for i in tqdm(range(len(lab)), desc="Getting point pre-images"):
        points[lab[i]].append(tuple(pos[i]))

    return points


def lookup(pos, fmap, keep_oob=False, interp=False):
    if interp:  # slow, has edge artifacts
        from scipy.interpolate import interpn
        mask = fmap.raw[:,:,:,0] > -1  # mapped
        w = np.where(mask)
        bounds = (range(np.min(w[0]), np.max(w[0]) + 1),
                  range(np.min(w[1]), np.max(w[1]) + 1),
                  range(np.min(w[2]), np.max(w[2]) + 1))
        allpos = [bounds[i] * fmap.voxel_dimensions[i] + fmap.offset[i] for i in range(0,3)]
        allval = fmap.raw[bounds[0],:,:][:,bounds[1],:][:,:,bounds[2]]
        res = interpn(allpos, allval, pos, method='linear', bounds_error=False, fill_value=-1)
    else:  # fast, limited to flatmap resolution
        res = fmap.lookup(pos, outer_value=np.array([-1, -1]))

    if not keep_oob:
        res = res[(res[:,0] > -1) & (res[:,1] > -1)]  # remove out of bounds
    return res
