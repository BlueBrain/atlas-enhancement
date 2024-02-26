# path to executable
FLATPLOT_BIN := $(PYTHON3) $(SOURCE_CODE_ROOT)/utils/flatplot.py

# DO NOT EDIT BELOW THIS LINE
STEPS := flatview_3d_data\
		 flatview_annotations\
		 flatview_morphology\
		 streamline_derived_depth\
		 streamline_derived_thickness\
		 volume_decomposition

DEFAULT_STEPS := flatview_annotations

## file names
FLATVIEW_ANNOTATIONS_FILE := flatview_annotations.png
VOLUME_DECOMPOSITION_NRRD_FILE := hexgrid.nrrd
STREAMLINE_DERIVED_DEPTH_FILE := depth.nrrd
STREAMLINE_DERIVED_THICKNESS_FILE := thickness.nrrd
