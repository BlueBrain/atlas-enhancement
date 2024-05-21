# DO NOT EDIT BELOW THIS LINE
ifeq (1,$(BOTH))
override INPUTS := input/$(ANNOTATIONS_FILE)\
	input/$(FLATMAP_BOTH_NRRD_FILE)\

override OUTPUTS := output/$(FLATVIEW_ANNOTATIONS_BOTH_FILE)
else
override INPUTS := input/$(ANNOTATIONS_FILE)\
	input/$(FLATMAP_NRRD_FILE)\
	input/$(MASK_FILE)

override OUTPUTS := output/$(FLATVIEW_ANNOTATIONS_FILE)
endif

ifeq (1,$(SPLIT))
override INPUTS := $(INPUTS)\
	input/$(LAYERS_FILE)

override OUTPUTS := $(subst .png,_split,$(OUTPUTS))
endif

override USER_PARAMETERS := FLATVIEW_ANNOTATIONS_COLORMAP\
	PIXEL_RESOLUTION

override USER_BINARIES := FLATPLOT_BIN
