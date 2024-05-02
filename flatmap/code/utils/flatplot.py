#!/usr/bin/env python3
# AUTHOR: Sirio Bolaños Puchet <sirio.bolanospuchet@epfl.ch>
# CREATION DATE: 2023-05-30
# LAST MODIFICATION: 2024-05-02

_PROGNAME = 'flatplot'

import logging

# setup logger
LOGGER = logging.getLogger(_PROGNAME)
logging.basicConfig(level=logging.INFO)

# parse arguments early
if __name__ == "__main__":
    import argparse
    import sys

    parser = argparse.ArgumentParser(_PROGNAME)
    parser.add_argument("flatmap",
            help="Path to flatmap NRRD")
    parser.add_argument("dataset",
            help="Path to input dataset (NRRD or XYZ)")
    parser.add_argument("output_prefix",
            help="Path prefix for output PNG image")
    parser.add_argument("-c", "--colormap", default='cet:gray',
            help="Color map")
    parser.add_argument("-l", "--layers", default=None,
            help="Path to layers annotation NRRD")
    parser.add_argument("-m", "--mask", default=None,
            help="Path to mask NRRD")
    parser.add_argument("-p", "--flatpix", type=int, default=256,
            help="Number of pixels in square flat map")
    parser.add_argument("-r", "--reduction", default='max',
            help="Reduction function for pixel values")
    parser.add_argument("-s", "--split", default=False, action="store_true",
            help="Split flat views by layer")
    parser.add_argument("-L", "--only-layer", type=int, default=None,
            help="Plot only this layer")
    parser.add_argument("--how", default='linear',
            help="How to map from the value scale to the colormap")
    parser.add_argument("--regions", default=False, action="store_true",
            help="Plot flat view of region annotations (mode reduction)")
    parser.add_argument("--regions-background", type=float, default=-1,
            help="Background value in regions NNRD input")
    parser.add_argument("--dots", default=False, action="store_true",
            help="Plot flat dots (no reduction)")
    parser.add_argument("--reflect", default=False, action="store_true",
            help="Reflect dots (hemisphere swap)")
    parser.add_argument("--autospan", default=False, action="store_true",
            help="Autoscale value axis (instead of 0 to 1)")
    parser.add_argument("--dual", default=False, action="store_true",
            help="Dual-hemisphere plot")
    parser.add_argument("--spread", type=int, default=None,
            help="Point spread when plotting dots")

    args = parser.parse_args()

    if args.split and args.layers is None:
        LOGGER.error('Layer annotation is required for split flat views')
        sys.exit(1)


import numpy as np
import datashader as ds
import datashader.transfer_functions as tf
import flatmap_util as fmutil
import colorcet


def flatplot(flatpos, flatval, cmap=colorcet.gray, flatpix=256, reduction=ds.max, how='linear',
             span=(0, 1), dual=False):
    import pandas as pd
    plot_width = 2 * flatpix if dual else flatpix
    x_range = (0,2) if dual else (0,1)
    cvs = ds.Canvas(plot_width=plot_width, plot_height=flatpix, x_range=x_range, y_range=(0,1))
    df = pd.DataFrame(flatpos, columns=('x','y'))
    df['val'] = flatval  # set column while keeping data type
    agg = cvs.points(df,'x','y',reduction('val'))
    return tf.shade(agg, how=how, cmap=cmap, span=span), agg


def flatplot_dots(flatpos, flatval, cmap='#000000', flatpix=256, how='eq_hist', spread=None,
                  dual=False):
    import pandas as pd
    x_range = (0,2) if dual else (0,1)
    cvs = ds.Canvas(plot_width=flatpix, plot_height=flatpix, x_range=x_range, y_range=(0,1))
    df = pd.DataFrame(flatpos, columns=('x','y'))
    agg = cvs.points(df,'x','y')
    if spread is not None:
        return tf.shade(tf.spread(agg, px=spread), cmap=cmap, how=how), agg
    else:
        return tf.shade(agg, cmap=cmap, how=how), agg


def get_mode(vals):
    x, c = np.unique(vals, return_counts=True)
    return x[np.argmax(c)]


def flatplot_regions(flatpos, flatval, cmap=colorcet.glasbey, flatpix=256, valbkg=-1):
    from collections import defaultdict
    import xarray as xr
    try:
        from joblib import parallel_config, Parallel, delayed
        from multiprocessing import cpu_count
        has_parallel = True
    except ModuleNotFoundError:
        has_parallel = False
    # mask data
    cmsk = (flatval != valbkg)
    flatpos = flatpos[cmsk]
    flatval = flatval[cmsk]
    # rasterize
    intpos = fmutil.discretize(flatpos, flatpix)
    idat = np.c_[ intpos, flatval ]
    # compute mode of values per pixel
    d = defaultdict(list)
    for x, y, z in idat:
        d[(x,y)].append(z)
    if has_parallel:
        with parallel_config(backend='threading', n_jobs=cpu_count()):
            result = Parallel()(delayed(get_mode)(v) for v in d.values())
    else:
        result = [get_mode(v) for v in d.values()]
    dmode = {k: result[i] for i, k in enumerate(d.keys())}
    # setup raster
    arr = np.full((flatpix,flatpix), np.nan, dtype='float32')
    for k,v in dmode.items():
        arr[k] = v
    # correct orientation
    arr = np.rot90(np.flipud(arr), -1)
    # plot
    cvs = ds.Canvas(plot_width=flatpix, plot_height=flatpix)
    xs = ys = np.linspace(0, 1, flatpix)
    x = xr.DataArray(arr,coords=[('y', ys), ('x', xs)])
    agg = cvs.raster(x)
    return tf.shade(agg, how='linear', cmap=cmap, span=(0, len(cmap) - 1)), agg


def str_to_reduction(s):
    if s in ['max']:
        return ds.max
    elif s in ['min']:
        return ds.min
    elif s in ['avg','mean']:
        return ds.mean
    elif s in ['sum']:
        return ds.sum
    elif s in ['mode']:
        return ds.mode
    elif s in ['std','sd']:
        return ds.std
    elif s in ['var']:
        return ds.var
    elif s in ['count']:
        return ds.count
    else:
        raise ValueError('Unknown reduction {}'.format(s))


if __name__ == "__main__":
    from voxcell import VoxelData
    from parse_cmap import parse_cmap
    import numpy as np

    cmap = parse_cmap(args.colormap)

    kwargs = {}
    if args.autospan:
        kwargs.update({'span': None})
    if args.regions:
        func = flatplot_regions
        kwargs.update({'valbkg': args.regions_background})
    elif args.dots:
        func = flatplot_dots
        kwargs.update({'spread': args.spread})
        kwargs.update({'dual': args.dual})
    else:
        func = flatplot
        kwargs.update({'reduction': str_to_reduction(args.reduction)})
        kwargs.update({'how': args.how})
        kwargs.update({'dual': args.dual})

    LOGGER.info('Loading flat map "{}"'.format(args.flatmap))
    fmap, fmap_valid = fmutil.load_flatmap(args.flatmap)

    if args.dots:
        LOGGER.info('Loading dots "{}"'.format(args.dataset))
        dots = np.loadtxt(args.dataset)
        dots = dots.reshape((-1, 3))

        if args.reflect:
            zmax = fmap.bbox[1, 2]
            dots[:, 2] = zmax - dots[:, 2]

        flatpos = fmutil.lookup(dots, fmap)
        flatval = None
    else:
        LOGGER.info('Loading dataset "{}"'.format(args.dataset))
        dat = VoxelData.load_nrrd(args.dataset)
        assert(dat.shape == fmap.shape)

        data_valid = True
        if args.mask is not None:
            LOGGER.info('Loading data mask "{}"'.format(args.mask))
            dmsk = VoxelData.load_nrrd(args.mask)
            assert(dmsk.shape == dat.shape)
            data_valid = (dmsk.raw != 0)
            del dmsk

        lay = None
        if args.split or args.only_layer is not None:
            LOGGER.info('Loading layer annotation "{}"'.format(args.layers))
            lay = VoxelData.load_nrrd(args.layers)
            assert(lay.shape == dat.shape)
            ldict = {x: x for x in np.unique(lay.raw)[1:]}  # skip background first value (= 0)
            if args.only_layer is not None:
                if args.only_layer not in ldict:
                    raise ValueError("requested layer {} does not exist".format(args.only_layer))
                ldict = {args.only_layer: ldict[args.only_layer]}

        LOGGER.info('Masking data')
        mask = np.where(fmap_valid & data_valid)
        flatpos = fmap.raw[mask]
        flatval = dat.raw[mask]

    if lay is not None:
        LOGGER.info('Masking layer annotation')
        layval = lay.raw[mask]
        for k, v in ldict.items():
            LOGGER.info('Generating and saving image for Layer {} ({}x{})'.format(v, args.flatpix, args.flatpix))
            lmask = np.where(layval == k)
            img, agg = func(flatpos[lmask], flatval[lmask], cmap, args.flatpix, **kwargs)
            LOGGER.info('Min: {} Max: {}'.format(np.nanmin(agg.values), np.nanmax(agg.values)))
            ds.utils.export_image(img, '{}_Layer{}'.format(args.output_prefix, v), background=None, _return=False)
    else:
        LOGGER.info('Generating and saving image ({}x{})'.format(args.flatpix, args.flatpix))
        img, agg = func(flatpos, flatval, cmap, args.flatpix, **kwargs)
        LOGGER.info('Min: {} Max: {}'.format(np.nanmin(agg.values), np.nanmax(agg.values)))
        ds.utils.export_image(img, args.output_prefix, background=None, _return=False)
