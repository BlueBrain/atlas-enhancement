"""
This script processes voxel data and computes various statistics of the shape and size of the barrels and barrel columns. The computed statistical measures include volumes and surface areas. One of the critical steps involves calculating the convex hull for each identified barrel. The convex hull provides insights into the spatial extent and shape of the structures, enabling further analysis of their geometric properties. 

"""
import math
import blueetl
import numpy as np
import pandas as pd
import voxcell as vc
from scipy.spatial import ConvexHull, distance


def simplices_to_sorted_positions(simplices):
    """
    Calculate the angle of each point with respect to the center and sort the simplices by angle.

    Args:
    - simplices (numpy.ndarray): Array containing the coordinates of points forming the simplices.

    Returns:
    - pandas.DataFrame: DataFrame containing sorted coordinates of simplices by angle.
    """
    x = simplices[:, 0]
    y = simplices[:, 1]

    angle = np.arctan2(y - y.mean(), x - x.mean())
    df = pd.DataFrame([x, y, angle], index=["x", "y", "angle"])
    sorted_simplices = df.T.sort_values(by="angle").drop("angle", axis=1, inplace=False)

    return pd.concat([sorted_simplices, sorted_simplices.iloc[0:1]], ignore_index=True)


flatmap = vc.VoxelData.load_nrrd("../data/flatmap_float.nrrd")

positions = pd.read_feather("../data/annotated_barrels.feather")
voxel_size = 10**3 * 1e-9

barrels_positions = positions[positions.hemisphere == "left"]
barrels_positions4 = barrels_positions[barrels_positions.layer == "4"]
flatix = flatmap.positions_to_indices(barrels_positions4[list("xyz")].values)
flatreg = flatmap.raw[tuple(flatix.T)]

barrels_positions4["x_flat"] = flatreg[:, 0]
barrels_positions4["y_flat"] = flatreg[:, 1]


barrels_stats = {}
for bname, df in barrels_positions.groupby("barrel"):
    barrels_stats[bname] = df.shape[0]

barrels_stats = pd.DataFrame.from_dict(
    barrels_stats, orient="index", columns=["voxel_count"]
)
barrels_stats["volume"] = barrels_stats["voxel_count"] * 25**3

barrels_stats4 = {}
for bname, df in barrels_positions4.groupby("barrel"):
    barrels_stats4[bname] = df.shape[0]

barrels_stats4 = pd.DataFrame.from_dict(
    barrels_stats4, orient="index", columns=["voxel_count"]
)

# Calculate the convex hull of each barrel in L4
centers = []
simplices = []
hull_stats = []
simplices_positions = []

for bname, df in barrels_positions4.groupby("barrel"):
    temp = pd.DataFrame(df[["x_flat", "y_flat"]].mean(), index=["x_flat", "y_flat"]).T
    temp["barrel"] = bname
    centers.append(temp)

    hull = ConvexHull(df[["x_flat", "y_flat"]].values)
    hull_stats.append(
        pd.DataFrame(
            [hull.volume, hull.area], index=["area", "perimiter"], columns=[bname]
        ).T
    )

    simplex_ = pd.DataFrame(hull.simplices, columns=["x", "y"])
    simplex_["barrel"] = bname
    simplices.append(simplex_)

    simplex_positions = df[["x_flat", "y_flat"]].iloc[
        np.unique(hull.simplices.flatten())
    ]
    simplex_positions["barrel"] = bname
    simplices_positions.append(simplex_positions)

centers = pd.concat(centers).reset_index(drop=True)
simplices = pd.concat(simplices).reset_index(drop=True)
hull_stats = pd.concat(hull_stats)
simplices_positions = pd.concat(simplices_positions).reset_index(drop=True)

# Sort simplices
simplices_distances = []
for bname, df in simplices_positions.groupby("barrel"):
    center_point = centers.etl.q(barrel=bname)[["x_flat", "y_flat"]].values[0]
    df["distance"] = df.apply(
        lambda row: distance.euclidean((row["x_flat"], row["y_flat"]), center_point),
        axis=1,
    )
    simplices_distances.append(df)
simplices_distances = pd.concat(simplices_distances).reset_index(drop=True)


simplices_positions_sorted = []
for barrel, barrel_simplices in simplices_positions.groupby("barrel"):
    sorted_simplices = simplices_to_sorted_positions(
        barrel_simplices[["x_flat", "y_flat"]].values
    )
    sorted_simplices["barrel"] = barrel

    simplices_positions_sorted.append(sorted_simplices)
simplices_positions_sorted = pd.concat(simplices_positions_sorted)


# Calulate the statistics in metric not just voxels
all_stats = pd.merge(
    hull_stats[["area"]],
    simplices_distances.groupby("barrel")
    .max("distance")[["distance"]]
    .rename(columns={"distance": "max_radius"}),
    left_index=True,
    right_index=True,
)

all_stats = pd.merge(
    all_stats,
    barrels_stats4.rename(columns={"voxel_count": "voxel_count_layer4"}),
    left_index=True,
    right_index=True,
)

all_stats = pd.merge(
    all_stats,
    barrels_stats.rename(columns={"voxel_count": "voxel_count_column"})[
        ["voxel_count_column"]
    ],
    left_index=True,
    right_index=True,
)

all_stats["volume column [mm^3]"] = all_stats["voxel_count_column"] * voxel_size
all_stats["volume barrel [mm^3]"] = all_stats["voxel_count_layer4"] * voxel_size
all_stats["surface area [mm^2]"] = all_stats["area"] * 5.16521e7 * 1e-6


# Save data
simplices_positions_sorted.reset_index(drop=True).to_feather(
    "data/simplices-positions.feather"
)
all_stats.reset_index(drop=False).rename(columns={"index": "barrel"}).to_feather(
    "data/statistics-volume-area.feather"
)
