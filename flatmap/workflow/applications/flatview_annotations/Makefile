include ../../common_header.mk

ifeq (1,$(BOTH))
TARGET := output/$(FLATVIEW_ANNOTATIONS_BOTH_FILE)
FLATMAP_DEP := input/$(FLATMAP_BOTH_NRRD_FILE)
FLATPLOT_FLAGS += --dual
else
TARGET := output/$(FLATVIEW_ANNOTATIONS_FILE)
FLATMAP_DEP := input/$(FLATMAP_NRRD_FILE)
MASK_DEP := input/$(MASK_FILE)
FLATPLOT_FLAGS += --mask $(MASK_DEP)
endif

ifeq (1,$(SPLIT))
FLATPLOT_FLAGS += --split --layers input/$(LAYERS_FILE)
TARGET := $(subst .png,_split,$(TARGET))
EXTRA_DEPS := input/$(LAYERS_FILE)
endif

$(TARGET): $(FLATMAP_DEP) input/$(ANNOTATIONS_FILE) $(MASK_DEP) $(EXTRA_DEPS)
	$(FLATPLOT_BIN) $(FLATPLOT_FLAGS) --regions --colormap $(FLATVIEW_ANNOTATIONS_COLORMAP) -p $(PIXEL_RESOLUTION) \
		$< $(word 2,$^) $(subst .png,,$@)

# input links
input/$(FLATMAP_NRRD_FILE): SRC := $(STAGE_III_ROOT)/02_flatmap/output/$(FLATMAP_NRRD_FILE)
input/$(FLATMAP_BOTH_NRRD_FILE): SRC := $(POSTPROC_ROOT)/both_hemispheres/output/$(FLATMAP_BOTH_NRRD_FILE)

# auxiliary commands
view: view-image

include ../../common_footer.mk
