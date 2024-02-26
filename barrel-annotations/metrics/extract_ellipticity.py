"""
Fit barrels to ellipsoids and computes their properties based on voxel data and barrel positions.
This scripts filters and extracts specific barrel positions before mapping 3D positions to 2D positions on a flatmap. Subsequently, it calculates the properties of confidence ellipses for each barrel. Computed ellipsoid properties include computes the ratio of the major axis to the minor axis for each ellipsoid, it's radii and the angle of the major axis.

References:
https://people.richland.edu/james/lecture/m170/tbl-chi.html
https://www.visiondummy.com/2014/04/draw-error-ellipse-representing-covariance-matrix
"""
import numpy as np
import pandas as pd
import voxcell as vc
from tqdm import tqdm


def confidence_ellipse_properties(x, y, chi=9.21):
    """
    Calculate properties of the covariance confidence ellipse for 2D data.

    Args:
        x, y: array-like, shape (n, ), input data.

        chi: Chi-square probability for confidence interval (default: 9.21 for 99% confidence). float, optional

    Returns:
        angle_rad: Angle of the major axis in radians.
        mean_x: Mean of x values.
        mean_y: Mean of y values.
        eigenvalue_x: Eigenvalue corresponding to the major axis.
        eigenvalue_y: Eigenvalue corresponding to the minor axis.
        radius_x: Radius of the major axis.
        radius_y: Radius of the minor axis.
    """

    mean_x = np.mean(x)
    mean_y = np.mean(y)

    cov = np.cov(x, y)
    eigenvalues, eigenvectors = np.linalg.eigh(cov)
    radius_x = 2 * np.sqrt(chi * eigenvalues[0])
    radius_y = 2 * np.sqrt(chi * eigenvalues[1])

    angle_rad = np.arctan2(eigenvectors[0, 1], eigenvectors[1, 1])

    return angle_rad, mean_x, mean_y, eigenvalues[0], eigenvalues[1], radius_x, radius_y


flatmap = vc.VoxelData.load_nrrd(
    "/gpfs/bbp.cscs.ch/project/proj100/atlas_work/mouse/atlas-enhancement/data/flatmap_float.nrrd"
)
positions = pd.read_feather(
    "/gpfs/bbp.cscs.ch/project/proj100/atlas_work/mouse/atlas-enhancement/data/annotated_barrels.feather"
)
barrels = positions.barrel.unique()

barrels_positions4 = positions[positions.layer == "4"]
barrels_positions4 = barrels_positions4[barrels_positions4.hemisphere == "left"]

flatix = flatmap.positions_to_indices(barrels_positions4[list("xyz")].values)
flatreg = flatmap.raw[tuple(flatix.T)]
barrels_positions4["x_flat"] = flatreg[:, 0]
barrels_positions4["y_flat"] = flatreg[:, 1]

ellipsoids = []
for barrel, df in tqdm(barrels_positions4.groupby("barrel")):
    values = confidence_ellipse_properties(df.x_flat, df.y_flat)
    result = pd.DataFrame(
        values,
        index=[
            "angle_rad",
            "center_x",
            "center_y",
            "eigenvalue_x",
            "eigenvalue_y",
            "x_radius",
            "y_radius",
        ],
    ).T
    result["barrel"] = barrel
    ellipsoids.append(result)

ellipsoids = pd.concat(ellipsoids)


ellipsoids["ratio"] = ellipsoids.apply(
    lambda row: np.max([row["y_radius"], row["x_radius"]])
    / np.min([row["y_radius"], row["x_radius"]]),
    axis=1,
)

ellipsoids.reset_index(drop=True, inplace=True)
ellipsoids.to_feather("data/ellipsoids.feather")
