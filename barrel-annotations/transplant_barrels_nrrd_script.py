import argparse
import numpy as np
from voxcell import VoxelData

def transplant_annotations(data_path, annotation_path, resolution):
    """
    Transplant pregenerated barrel annotations into an annotation file.
    :param data_path:
    :param annotation_path:
    :param resolution:
    :return:
    """
    print('Transplanting barrel annotations into annotation file...')
    # Define paths based on provided data_path and resolution
    annotation_barrels_path = f"{data_path}/annotation_barrels.nrrd"
    annotation_path = f"{annotation_path}/annotation_{resolution}.nrrd"
    output_path = f"{data_path}/annotation_barrels_{resolution}.nrrd"

    # Load the necessary nrrd files
    barrel = VoxelData.load_nrrd(annotation_barrels_path)
    annotation = VoxelData.load_nrrd(annotation_path)

    # Extract the raw data from the annotation files
    annotation_raw = annotation.raw
    barrel_raw = barrel.raw

    # Identify the indices where barrel annotation is present
    indices = np.argwhere(barrel_raw > 0)

    # Create a mask of the same shape as the annotation file
    mask = np.zeros(annotation.shape, dtype=bool)

    # Set the mask to True where the barrel indices are present
    for indx in indices:
        mask[tuple(indx)] = True

    # Update the annotation data where the mask is True
    annotation_raw[mask] = barrel_raw[mask]

    # Create a new VoxelData object for the output
    barrel_nrrd = VoxelData(
        annotation_raw, annotation.voxel_dimensions, offset=annotation.offset
    )

    # Save the updated annotation data to the output path
    print('Saving updated annotation data to output path...')
    barrel_nrrd.save_nrrd(output_path)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Transplant pregenerated barrel annotations into an annotation file.')
    parser.add_argument('data_path', type=str, help='Path to new data files')
    parser.add_argument('annotation_path', type=str, help='Path to the original Allen annotation file')
    parser.add_argument('resolution', type=str, help='Resolution identifier to determine which annotation file to use')

    args = parser.parse_args()
    transplant_annotations(args.data_path, args.annotation_path, args.resolution)