import numpy as np

from voxcell import VoxelData
from os import cpu_count
import sys

from tqdm import tqdm

import flatmap_util as fmutil

import best_line_fit as blf
from trsqrpyr import normalized_volume_r_fun, linear_volume_increase_fun
from lmfit import Parameters, Model


flatmap_nrrd = sys.argv[1]
pixel_res = int(sys.argv[2])

orient_x_nrrd = sys.argv[3]
orient_y_nrrd = sys.argv[4]
orient_z_nrrd = sys.argv[5]

output_nrrd = sys.argv[6]

ncpu = cpu_count() // 2
if len(sys.argv) > 7:
    ncpu = int(argv[7])

# load flat map
fmap_vd = VoxelData.load_nrrd(flatmap_nrrd)
vshape = fmap_vd.shape[0:3]

points = fmutil.get_preimages_pts(fmap_vd, pixel_res)
del fmap_vd  # FREE

label_unique = np.array(list(points.keys()))  # used flat pixels

# unravel used pixel locations
ix = tuple([np.array([x % pixel_res for x in label_unique]),
            np.array([x // pixel_res for x in label_unique])])

# load orientation vectors
orient_vd = [
        VoxelData.load_nrrd(orient_x_nrrd),
        VoxelData.load_nrrd(orient_y_nrrd),
        VoxelData.load_nrrd(orient_z_nrrd) ]

assert(orient_vd[0].shape == orient_vd[1].shape)
assert(orient_vd[1].shape == orient_vd[2].shape)
assert(orient_vd[2].shape == vshape)


# get orientation vector
def get_orientation_at(position):
    return np.array([
        orient_vd[0].lookup(position),
        orient_vd[1].lookup(position),
        orient_vd[2].lookup(position)])


# goodness-of-fit measure
def squared_cosine(a, b):
    u = np.dot(a, b)
    return u * u / (np.dot(a, a) * np.dot(b, b))


def getline(pts, label='DEBUG'):
    try:
        # best fit line
        line = blf.best_line_fit(pts)
        # dot product of line direction and orientation at centroid
        ## under the convention that orientation points away from depth:
        ## negative heading means depth increases along line
        ## positive heading means depth diminishes along line
        heading = line.direction.scalar_projection(get_orientation_at(line.point))
        # flip line direction to point towards increasing depth
        line.direction = line.direction if heading < 0 else -line.direction
        # np.savetxt('vec_{}.txt'.format(label),list(line.point) + list(line.direction))  # DEBUG
    except ValueError:
        line = None
    return line


# best line fits to pre-images
def get_r0_r1_K(pts, line, label='DEBUG'):
    popt = None
    perr = None
    # np.savetxt('pts_{}.txt'.format(label),pts)                  # DEBUG
    try:
        if line is None:
            raise ValueError("Cannot compute best fit line")

        # get radial distances and heights
        rh = np.array([(line.distance_point(x), blf.position_along_line(line, x)) for x in pts])

        # radial distances
        r = rh[:,0]
        # np.savetxt('r_{}.txt'.format(label),r)                  # DEBUG
        # rhist, rbins = np.histogram(r, 36, density=True)        # DEBUG
        # np.savetxt('rpdf_{}.txt'.format(label),np.vstack((rbins[:-1],rhist)).T)  # DEBUG
        ## normalized CDF
        rx, rcounts = np.unique(r, return_counts=True)
        rcdf = np.cumsum(rcounts)
        rcdf = rcdf / rcdf[-1]
        ## too few points in CDF
        rfit_dof = len(rcdf) - 2  # 2 parameters (r0, r1)
        if rfit_dof < 1:
            raise ValueError("Not enough fit DOF for radial distribution")
        # np.savetxt('rcdf_{}.txt'.format(label),np.vstack((rx,rcdf)).T)  # DEBUG

        # vertical distances
        h = rh[:,1]
        h = h - min(h)  # shift to all positive along the line
        # np.savetxt('h_{}.txt'.format(label),h)                  # DEBUG
        # hhist, hbins = np.histogram(h, 36, density=True)        # DEBUG
        # np.savetxt('hpdf_{}.txt'.format(label),np.vstack((hbins[:-1],hhist)).T)  # DEBUG
        ## normalized CDF
        hx, hcounts = np.unique(h, return_counts=True)
        hcdf = np.cumsum(hcounts)
        hcdf = hcdf / hcdf[-1]
        ## too few points in CDF
        hfit_dof = len(hcdf) - 2  # 2 parameters (delta, H)
        if hfit_dof < 1:
            raise ValueError("Not enough fit DOF for depthwise distribution")
        # np.savetxt('hcdf_{}.txt'.format(label),np.vstack((hx,hcdf)).T)  # DEBUG

        # normalized radial volume distribution of square frustum
        rmodel = Model(normalized_volume_r_fun)
        rparams = Parameters()
        rparams.add('r0', value=10, min=0.0, vary=True)
        rparams.add('delta', value=20, min=1e-8, vary=True)
        rparams.add('r1', expr="r0 + delta")

        # fit model
        rres = rmodel.fit(rcdf, rparams, r=rx, calc_covar=False)
        if rres.success:
            r0 = rres.best_values['r0']
            r1 = rres.best_values['r1']
            # squared cosine between data and best_fit
            rerr = squared_cosine(rcdf, rres.best_fit)
        else:
            raise ValueError("Failed fit for radial distribution")

        # normalized depth volume distribution of square frustum
        hmodel = Model(linear_volume_increase_fun)
        hparams = Parameters()
        hparams.add('a', value=1, min=0.0, vary=True)
        hparams.add('b', value=1, vary=True)
        hparams.add('H', value=hx[-1], vary=False)

        # fit model
        hres = hmodel.fit(hcdf, hparams, h=hx, calc_covar=False)
        if hres.success:
            K = 2.0 * hres.best_values['b']  # slope of linear density fit
            # squared cosine between data and best_fit
            herr = squared_cosine(hcdf, hres.best_fit)
        else:
            raise ValueError("Failed fit for depthwise distribution")

        popt = (r0, r1, K)
        perr = (rerr, herr)

    except ValueError:
        pass  # points not concurrent or not enough fit DOF or failed fit

    popt = popt if popt is not None else (np.nan, np.nan, np.nan)  # default params (sentinel)
    perr = perr if perr is not None else (np.nan, np.nan)          # default error  (sentinel)

    return (popt, perr)


# compute
desc = "Computing best-fit lines"
if ncpu == 1:  # fit parameters serially
    lines = [getline(pts) for pts in tqdm(points, desc=desc)]
else:          # fit parameters in parallel
    from joblib import Parallel, delayed
    from tqdm_joblib_bridge import tqdm_joblib
    with tqdm_joblib(tqdm(desc=desc, total=len(label_unique))) as progress_bar:
        with Parallel(n_jobs=ncpu) as parallel:
            lines = parallel(delayed(getline)(points[x]) for x in label_unique)

del orient_vd  # FREE

desc = "Computing r0, r1, K"
if ncpu == 1:  # fit parameters serially
    result = [get_r0_r1_K(pts) for pts in tqdm(points, desc=desc)]
    # DEBUG
    # n = 656 # 430
    # for x in label_unique[range(n,n + 1)]:
    #     pts = pos[np.where(labels == x)]
    #     res = get_r0_r1_K(pts, x)
    #     print(res)
    # exit(1)
else:          # fit parameters in parallel
    from joblib import Parallel, delayed
    from tqdm_joblib_bridge import tqdm_joblib
    with tqdm_joblib(tqdm(desc=desc, total=len(label_unique))) as progress_bar:
        with Parallel(n_jobs=ncpu) as parallel:
            result = parallel(delayed(get_r0_r1_K)(points[x], line) for x, line in zip(label_unique, lines))

del points, lines  # FREE

# setup result images
fit_K = np.full((pixel_res, pixel_res), np.nan, dtype=np.float32)
fit_r0 = np.full((pixel_res, pixel_res), np.nan, dtype=np.float32)
fit_r1 = np.full((pixel_res, pixel_res), np.nan, dtype=np.float32)
err_r = np.full((pixel_res, pixel_res), np.nan, dtype=np.float32)
err_h = np.full((pixel_res, pixel_res), np.nan, dtype=np.float32)

# extract result parameters ((r0, r1, K), (rerr, herr))
fit_r0[ix] = [x[0][0] for x in result]
fit_r1[ix] = [x[0][1] for x in result]
fit_K[ix] = [x[0][2] for x in result]
err_r[ix] = [x[1][0] for x in result]
err_h[ix] = [x[1][1] for x in result]

# mean of fit radii
rmean = np.full((pixel_res, pixel_res), np.nan, dtype=np.float32)
rmean[ix] = (fit_r0[ix] + fit_r1[ix]) / 2.0

# save results
res = np.stack((rmean, fit_K, fit_r0, fit_r1, err_r, err_h))
res_vd = VoxelData(res, (1,1,1))
res_vd.save_nrrd(output_nrrd)
