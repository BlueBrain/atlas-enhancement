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
    parser.add_argument("-z", "--zoom", type=int, default=1,
            help="Zoom image")

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


# customize datashader image export
def export_image(img, filename, fmt=".png", _return=True, export_path=".", background="", scaling=1):
    """Given a datashader Image object, saves it to a disk file in the requested format"""

    from datashader.transfer_functions import set_background
    from PIL import Image
    import os

    if not os.path.exists(export_path):
        os.mkdir(export_path)

    if background:
        img = set_background(img, background)

    img_pil = img.to_pil()
    if scaling > 1:
        img_pil = img_pil.resize(np.array(img_pil.size) * scaling, Image.Resampling.NEAREST)
    img_pil.save(os.path.join(export_path, filename + fmt))
    return img if _return else None


if __name__ == "__main__":
    from voxcell import VoxelData
    from parse_cmap import parse_cmap

    cmap = parse_cmap(args.colormap)

    LOGGER.info('Loading input data "{}"'.format(args.input_nrrd))
    vd = VoxelData.load_nrrd(args.input_nrrd)
    flatpix = vd.shape[0]
    array = vd.raw

    minval = np.nanmin(array) if args.minval is None else args.minval
    maxval = np.nanmax(array) if args.maxval is None else args.maxval
    span = [minval, maxval]

    LOGGER.info('Generating and saving image ({}x{}) (zoom x{})'.format(flatpix, flatpix, args.zoom))
    img, agg = plot_heatmap(array,flatpix,cmap,span)
    LOGGER.info('Min: {} Max: {}'.format(np.nanmin(agg.values), np.nanmax(agg.values)))
    export_image(img, args.output_prefix, background=args.bgcolor, _return=False, scaling=args.zoom)
