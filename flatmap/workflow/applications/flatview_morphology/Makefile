include ../../common_header.mk

ifndef FLATVIEW_MORPHO_INPUT_FILE
$(error Please set FLATVIEW_MORPHO_INPUT_FILE to location of input JSON morphology file)
endif

FLATVIEW_MORPHO_PNG_FILE_base := $(subst .png,,$(FLATVIEW_MORPHO_PNG_FILE))

FLATPLOT_MORPHO_EXTRA ?=
override FLATPLOT_FLAGS := $(FLATPLOT_MORPHO_EXTRA) $(FLATPLOT_FLAGS)

ifeq (1,$(REFLECT))
override FLATPLOT_FLAGS := $(FLATPLOT_FLAGS) --reflect
endif

ifeq (1,$(BOTH))
FLATMAP_DEP := input/$(FLATMAP_BOTH_NRRD_FILE)
ANNOTATIONS_DEP := input/$(FLATVIEW_ANNOTATIONS_BOTH_FILE)
override FLATPLOT_FLAGS := $(FLATPLOT_FLAGS) --dual
WIDTH := $(shell expr 2 \* $(PIXEL_RESOLUTION))
else
FLATMAP_DEP := input/$(FLATMAP_NRRD_FILE)
ANNOTATIONS_DEP := input/$(FLATVIEW_ANNOTATIONS_FILE)
WIDTH := $(PIXEL_RESOLUTION)
endif

output/$(FLATVIEW_MORPHO_PNG_FILE): $(ANNOTATIONS_DEP) output/$(FLATVIEW_MORPHO_PNG_FILE_base).soma.png output/$(FLATVIEW_MORPHO_PNG_FILE_base).axon.png
	$(MAGICK) composite -blend 50 $< -size $(WIDTH)x$(PIXEL_RESOLUTION) xc:white -alpha Set $@.tmp.png
	$(MAGICK) composite -compose CopyOpacity $< $@.tmp.png $@.tmp.png
	$(MAGICK) composite $(word 3,$^) $@.tmp.png - \
		| $(MAGICK) composite $(word 2,$^) - $@
	$(RM) $@.tmp.png

output/$(FLATVIEW_MORPHO_PNG_FILE_base).soma.png: JSON_FIELD := soma
output/$(FLATVIEW_MORPHO_PNG_FILE_base).soma.png: COLOR := \#FFFFFF
output/$(FLATVIEW_MORPHO_PNG_FILE_base).soma.png: FLATPLOT_EXTRA := --spread 2
output/$(FLATVIEW_MORPHO_PNG_FILE_base).axon.png: JSON_FIELD := axon[]
output/$(FLATVIEW_MORPHO_PNG_FILE_base).axon.png: COLOR := \#000000
output/$(FLATVIEW_MORPHO_PNG_FILE_base).soma.png output/$(FLATVIEW_MORPHO_PNG_FILE_base).axon.png: input/$(FLATVIEW_MORPHO_JSON_FILE) $(FLATMAP_DEP)
	$(JQ) '.neuron.$(JSON_FIELD) | .x, .y, .z' $< > $$$$.tmp
	$(FLATPLOT_BIN) --dots -c '$(COLOR)' -p $(PIXEL_RESOLUTION) $(FLATPLOT_EXTRA) $(FLATPLOT_FLAGS) $(word 2,$^) $$$$.tmp $(subst .png,,$@)
	$(RM) $$$$.tmp

# input links
input/$(FLATVIEW_MORPHO_JSON_FILE): SRC := $(FLATVIEW_MORPHO_INPUT_FILE)
input/$(FLATMAP_NRRD_FILE): SRC := $(STAGE_III_ROOT)/02_flatmap/output/$(FLATMAP_NRRD_FILE)
input/$(FLATMAP_BOTH_NRRD_FILE): SRC := $(POSTPROC_ROOT)/both_hemispheres/output/$(FLATMAP_BOTH_NRRD_FILE)
input/$(FLATVIEW_ANNOTATIONS_FILE): SRC := $(APPLICATIONS_ROOT)/flatview_annotations/output/$(FLATVIEW_ANNOTATIONS_FILE)
input/$(FLATVIEW_ANNOTATIONS_BOTH_FILE): SRC := $(APPLICATIONS_ROOT)/flatview_annotations/output/$(FLATVIEW_ANNOTATIONS_BOTH_FILE)

# auxiliary commands
view: view-image

include ../../common_footer.mk
