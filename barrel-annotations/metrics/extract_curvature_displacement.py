"""
Computes streamline curvature based on the spatial displacement of the central streamline of the barrel column. 

The deflection is measured at at average minimal and maximal depth of the streamline. Then maximal deflection is reported.
Intuitively, we measure by how much does the curved barrel deflects from a straight line. The process involves sampling sections of the streamline from top, middle, and bottom regions, computing their mean direction and centroid, and then measuring the distance to the line based on the L4 barrel.
"""
import blueetl
import numpy as np
import pandas as pd
import voxcell as vc
from utils import indices_to_mask
from tqdm import tqdm


def distance_to_infinite_line(point1, point2, third_point):
    """Calculates the distance from a point to an infinite line defined by two points.

    Args:
        point1: A numpy array representing the coordinates of the first point on the line.
        point2: A numpy array representing the coordinates of the second point on the line.
        third_point: A numpy array representing the coordinates of the third point.

    Returns:
        foot_of_perpendicular: A numpy array representing the coordinates of the foot of the perpendicular
            from the third point to the infinite line.
        distance: The distance between the third point and the foot of the perpendicular.
    """
    line_vector = point2 - point1
    point1_to_third = third_point - point1

    projection = np.dot(point1_to_third, line_vector) / np.dot(line_vector, line_vector)
    projection_vector = projection * line_vector

    foot_of_perpendicular = point1 + projection_vector

    distance = np.linalg.norm(foot_of_perpendicular - third_point)
    return foot_of_perpendicular, distance


def extract_streamline_displacement(streamline, depth):
    """Extracts displacement of a streamline within a given depth.

    Args:
        streamline (pd.DataFrame): DataFrame containing streamline coordinates.
        depth (VoxelData): Depth object providing depth information.

    Returns:
        float: Maximum displacement between the streamline and the depth boundaries.

    Raises:
        ValueError: If the input arguments are not valid.

    Notes:
        - The function calculates the displacement of a streamline based on its depth.
        - It samples average bottom and top values to determine displacement.
        - Displacement is measured with top and bottom of the streamline.
        - It selects the larger displacement between top and bottom boundaries.
    """
    indices = depth.positions_to_indices(streamline[["x", "y", "z"]].values)
    indices_frame = pd.DataFrame(indices, columns=["x", "y", "z"])
    unique_indices = np.unique(indices.reshape(-1, 3), axis=0)

    depth_values = depth.raw[indices_to_mask(unique_indices, depth).astype(bool)]
    depth_mask = pd.DataFrame(
        np.array(
            [
                unique_indices[:, 0],
                unique_indices[:, 1],
                unique_indices[:, 2],
                depth_values,
            ]
        ).T,
        columns=["x", "y", "z", "depth"],
    )

    temp = pd.merge(indices_frame, depth_mask, on=["x", "y", "z"], how="left")
    streamline["depth"] = temp.depth.values

    samplesize = np.floor(streamline.shape[0] * 0.05).astype(int)
    mean_bottom = np.mean(streamline[:samplesize][["x", "y", "z"]], axis=0)
    mean_top = np.mean(streamline[-samplesize:][["x", "y", "z"]], axis=0)

    streamline_layer4 = streamline.etl.q(
        depth={
            "lt": depth_borders.etl.q(barrel=barrel, layer="4").depth_mean.values[0],
            "gt": depth_borders.etl.q(barrel=barrel, layer="2/3").depth_mean.values[0],
        }
    )

    foot_of_perpendicular_top, distance_top = distance_to_infinite_line(
        streamline_layer4.iloc[0][list("xyz")].values,
        streamline_layer4.iloc[-1][list("xyz")].values,
        mean_top.values,
    )
    foot_of_perpendicular_bottom, distance_bottom = distance_to_infinite_line(
        streamline_layer4.iloc[0][list("xyz")].values,
        streamline_layer4.iloc[-1][list("xyz")].values,
        mean_bottom.values,
    )

    return max(distance_bottom, distance_top)


streamlinedf = pd.read_feather("../data/central-streamlines.feather")
depth = vc.VoxelData.load_nrrd("../data/depth.nrrd")
depth_borders = pd.read_feather(
    "/gpfs/bbp.cscs.ch/project/proj100/atlas_work/mouse/atlas-enhancement/metrics/data/depth.feather"
)

displacements = {}
for barrel, streamline in tqdm(streamlinedf.groupby("barrel")):
    displacements[barrel] = extract_streamline_displacement(streamline, depth)

displacements = (
    pd.DataFrame()
    .from_dict(displacements, orient="index")
    .reset_index()
    .rename(columns={0: "displacement", "index": "barrel"})
)
displacements.to_feather("data/streamline_displacements_from_barrel.feather")
