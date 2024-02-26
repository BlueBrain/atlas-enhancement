import json
import logging
import pandas as pd
import somatosensory_barrels
from voxcell import VoxelData


L = logging.getLogger(__name__)
logging.basicConfig(level=logging.INFO)


def split_barrel_columns(  # pylint: disable=too-many-arguments
    annotation_path,
    hierarchy_path,
    barrels_path,
    output_hierarchy_path,
    output_annotation_path,
):
    """Introduce the barrel columns to SSp-bfd (primary somatosensory barrel cortex)
    in the mouse annotations in place. Positions of the voxels need to be specified by a DataFrame.

    The `hierarchy` dict and the `annotation` are modified in-place.
    All of the barrels present in the DF are introduced based on their
    x, y, z voxel positions.

    New children are added to the layer 4 of Somatosensory barrel cortex
    in Isocortex.
    """

    L.info("Loading files ...")
    barrel_positions = pd.read_feather(barrels_path)
    annotation = VoxelData.load_nrrd(annotation_path)

    with open(hierarchy_path, "r", encoding="utf-8") as h_file:
        hierarchy = json.load(h_file)

    L.info("Introduce the barrel columns to SSp-bfd...")
    somatosensory_barrels.split_barrels(hierarchy, annotation, barrel_positions)

    L.info("Saving modified hierarchy and annotation files ...")
    with open(output_hierarchy_path, "w", encoding="utf-8") as out:
        json.dump(hierarchy, out, indent=1, separators=(",", ": "))
    annotation.save_nrrd(output_annotation_path)


if __name__ == "__main__":
    split_barrel_columns(
        annotation_path="data/annotation_10.nrrd",
        hierarchy_path="data/1.json",
        barrels_path="data/annotated_barrels.feather",
        output_hierarchy_path="data/hierarchy.json",
        output_annotation_path="data/annotation_barrels_10.nrrd",
    )
