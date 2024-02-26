# required software
JQ := jq
MAGICK := magick

# DO NOT EDIT BELOW THIS LINE
FLATVIEW_MORPHO_PNG_FILE := $(subst .json,.png,$(FLATVIEW_MORPHO_INPUT_FILE))

override INPUTS := input/$(FLATMAP_NRRD_FILE)\
	input/$(FLATVIEW_MORPHO_INPUT_FILE)\
	input/$(FLATVIEW_ANNOTATIONS_FILE)

override OUTPUTS := output/$(FLATVIEW_MORPHO_PNG_FILE)\
	output/flat_soma_dots.xyz\
	output/flat_axon_dots.xyz

override USER_PARAMETERS := FLATPLOT_MORPHO_EXTRA\
	PIXEL_RESOLUTION

override USER_BINARIES := FLATPLOT_BIN
