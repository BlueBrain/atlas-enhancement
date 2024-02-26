# AUTHOR: Sirio Bolaños Puchet <sirio.bolanospuchet@epfl.ch>
# CREATION DATE: 2024-02-15
# LAST MODIFICATION: 2024-02-15

_PROGNAME = 'plot_heatmap'

import logging

# setup logger
LOGGER = logging.getLogger(_PROGNAME)
logging.basicConfig(level=logging.INFO)

# parse arguments early
if __name__ == "__main__":
    import argparse
    import sys

    parser = argparse.ArgumentParser(_PROGNAME)
    parser.add_argument("input_nrrd",
            help="Path to input NRRD")
    parser.add_argument("output_prefix",
            help="Path prefix for output PNG image")
    parser.add_argument("-q", "--minval", type=float, default=None,
            help="Minimum value in colormap range")
    parser.add_argument("-Q", "--maxval", type=float, default=None,
            help="Maximum value in colormap range")
    parser.add_argument("-c", "--colormap", default='cet:gray',
            help="Color map")
    parser.add_argument("-B", "--bgcolor", default=None,
            help="Image background color (used for NaNs)")

    args = parser.parse_args()


import numpy as np
import datashader as ds
import datashader.transfer_functions as tf
import colorcet


def plot_heatmap(array, flatpix, cmap=colorcet.gray, span=None):
    import xarray as xr

    # correct orientation
    array = np.rot90(np.flipud(array), -1)

    cvs = ds.Canvas(plot_width=flatpix, plot_height=flatpix)
    xs = ys = np.linspace(0,1,flatpix)
    x = xr.DataArray(array,coords=[('y',ys),('x',xs)])
    agg = cvs.raster(x)

    return tf.shade(agg, how='linear', cmap=cmap, span=span), agg


if __name__ == "__main__":
    import voxcell as vc
    from parse_cmap import parse_cmap

    cmap = parse_cmap(args.colormap)

    LOGGER.info('Loading input data "{}"'.format(args.input_nrrd))
    vd = vc.VoxelData.load_nrrd(args.input_nrrd)
    flatpix = vd.shape[0]
    array = vd.raw

    minval = np.nanmin(array) if args.minval is None else args.minval
    maxval = np.nanmax(array) if args.maxval is None else args.maxval
    span = [minval, maxval]

    LOGGER.info('Generating and saving image ({}x{})'.format(flatpix,flatpix))
    img, agg = plot_heatmap(array,flatpix,cmap,span)
    LOGGER.info('Min: {} Max: {}'.format(np.nanmin(agg.values), np.nanmax(agg.values)))
    ds.utils.export_image(img, args.output_prefix, background=args.bgcolor, _return=False)
