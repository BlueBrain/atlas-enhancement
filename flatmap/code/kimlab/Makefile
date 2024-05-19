ifndef ATLAS_ENHANCEMENT_ROOT
$(error Please set ATLAS_ENHANCEMENT_ROOT to the location of atlas-enhancement)
endif

ifndef USER_DATA_ROOT
$(error Please set USER_DATA_ROOT to the location of user data directory)
endif

PYTHON := python3
CAT := cat
SED := sed

NPIX := 256

flatmap_kimlab_both.nrrd: flatmap_kimlab_$(NPIX).nrrd
	$(PYTHON) $(ATLAS_ENHANCEMENT_ROOT)/flatmap/code/utils/mirror_hemi.py $< $(USER_DATA_ROOT)/hemispheres.nrrd $@ $(NPIX)

flatmap_kimlab_$(NPIX).nrrd: flatmap_kimlab.nrrd
	$(PYTHON) $(ATLAS_ENHANCEMENT_ROOT)/flatmap/code/utils/discretize_flatmap.py $< $(NPIX) $@

flatmap_kimlab.nrrd: ftxflat.nrrd ftyflat.nrrd
	$(PYTHON) kimlab_flatmap.py $^ $@

define recipe_transpose_flip
#from voxcell import VoxelData
#import numpy as np
#dat = VoxelData.load_nrrd('$<')
#tdat = dat.raw.transpose()
#ftdat = np.flip(tdat, 2)
#dat.with_data(ftdat).save_nrrd('ft$<')
endef

ftxflat.nrrd ftyflat.nrrd ftmask.nrrd: ft%.nrrd: %.nrrd
	$(CAT) <<EOF | $(SED) 's/^#//' | $(PYTHON)
	$(recipe_transpose_flip)
	EOF

xflat.nrrd yflat.nrrd mask.nrrd: %.nrrd: %.nii.gz
	# manual processing of NIfTI files in xgrid3d
	## remove 1-voxel padding
	## expand image to full size (both hemispheres)
	## save as NRRD

mask.nii.gz xflat.nii.gz yflat.nii.gz: flat_map_data_10um_v2.mat
	MATLAB extract_flatmap.m
