import numpy as np
import pandas as pd
import voxcell as vc
from lmfit import Parameters, Model
from tqdm import tqdm


def linear_volume_increase(h, a, b, H):
    """
    Calculate the volume increase using linear density assumption.

    Args:
    - h (array_like): Array of heights.
    - a (float): Coefficient for the linear term.
    - b (float): Coefficient for the quadratic term.
    - H (float): Threshold height where the function switches to a constant value.

    Returns:
    - numpy.ndarray: Array of volume increases corresponding to the heights.
    """
    return np.where(h <= H, a * h + b * h * h, 1.0)


def calc_conicity(depths):
    """
    Calculate the conicality of voxel depths. Here h is an array with the depths of the voxels

    Args:
    - depths (array_like): Array of depths of the voxels.

    Returns:
    - float: Conicality value indicating the slope of linear density fit.
    """
    hx, hcounts = np.unique(depths, return_counts=True)
    hcdf = np.cumsum(hcounts)
    hcdf = hcdf / hcdf[-1]

    hmodel = Model(linear_volume_increase)
    hparams = Parameters()
    hparams.add("a", value=1, min=0.0, vary=True)
    hparams.add("b", value=1, vary=True)
    hparams.add("H", value=hx[-1], vary=False)

    hres = hmodel.fit(hcdf, hparams, h=hx, calc_covar=False)
    if hres.success:
        conicality = 2.0 * hres.best_values["b"]  # slope of linear density fit

    return conicality


depth = vc.VoxelData.load_nrrd("../data/depth.nrrd")
positions = pd.read_feather("../data/annotated_barrels.feather")

connicalitydf = {}
for barrel, pos in tqdm(positions.groupby("barrel")):
    pos["depth"] = pos.apply(
        lambda row: depth.raw[
            tuple(depth.positions_to_indices(row[list("xyz")].values))
        ],
        axis=1,
    )

    connicalitydf[barrel] = calc_conicity(pos.depth.values)


connicalitydf = (
    pd.DataFrame.from_dict(connicalitydf, orient="index")
    .reset_index()
    .rename(columns={"index": "barrel", 0: "conicality"})
)

connicalitydf.to_feather("data/conicality.feather")
