# DO NOT EDIT BELOW THIS LINE
FLATVIEW_DATA_NRRD_FILE = $(notdir $(FLATVIEW_DATA_INPUT_FILE))
FLATVIEW_DATA_PNG_FILE = $(subst .nrrd,.png,$(FLATVIEW_DATA_NRRD_FILE))

ifeq (1,$(BOTH))
override INPUTS := input/$(FLATMAP_BOTH_NRRD_FILE)\
	input/$(FLATVIEW_DATA_NRRD_FILE)

override OUTPUTS := output/$(subst .png,_both.png,$(FLATVIEW_DATA_PNG_FILE))
else
override INPUTS := input/$(FLATMAP_NRRD_FILE)\
	input/$(FLATVIEW_DATA_NRRD_FILE)

override OUTPUTS := output/$(FLATVIEW_DATA_PNG_FILE)
endif

ifeq (1,$(SPLIT))
override INPUTS := $(INPUTS)\
	input/$(LAYERS_FILE)

override OUTPUTS := $(subst .png,_split,$(OUTPUTS))
endif

override USER_PARAMETERS := FLATPLOT_DATA_EXTRA

override USER_BINARIES := FLATPLOT_BIN
