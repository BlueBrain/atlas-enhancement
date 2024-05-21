include ../../common_header.mk

ifndef FLATVIEW_DATA_INPUT_FILE
$(error Please set FLATVIEW_DATA_INPUT_FILE to location of input NRRD data file)
endif

FLATPLOT_DATA_EXTRA ?= --autospan -p $(PIXEL_RESOLUTION)
override FLATPLOT_FLAGS := $(FLATPLOT_DATA_EXTRA) $(FLATPLOT_FLAGS)

ifeq (1,$(BOTH))
TARGET := output/$(subst .png,_both.png,$(FLATVIEW_DATA_PNG_FILE))
FLATMAP_DEP := input/$(FLATMAP_BOTH_NRRD_FILE)
override FLATPLOT_FLAGS := $(FLATPLOT_FLAGS) --dual
else
TARGET := output/$(FLATVIEW_DATA_PNG_FILE)
FLATMAP_DEP := input/$(FLATMAP_NRRD_FILE)
endif

ifeq (1,$(SPLIT))
override FLATPLOT_FLAGS := $(FLATPLOT_FLAGS) --split --layers input/$(LAYERS_FILE)
TARGET := $(subst .png,_split,$(TARGET))
EXTRA_DEPS := input/$(LAYERS_FILE)
endif

ifdef FLATVIEW_DATA_MASK_FILE
EXTRA_DEPS += input/$(notdir $(FLATVIEW_DATA_MASK_FILE))
override FLATPLOT_FLAGS := $(FLATPLOT_FLAGS) --mask input/$(notdir $(FLATVIEW_DATA_MASK_FILE))
input/$(notdir $(FLATVIEW_DATA_MASK_FILE)): SRC := $(FLATVIEW_DATA_MASK_FILE)
endif

$(TARGET): $(FLATMAP_DEP) input/$(FLATVIEW_DATA_NRRD_FILE) $(EXTRA_DEPS)
	$(FLATPLOT_BIN) $(FLATPLOT_FLAGS) $< $(word 2,$^) $(subst .png,,$@)

# input links
input/$(FLATMAP_NRRD_FILE): SRC := $(STAGE_III_ROOT)/02_flatmap/output/$(FLATMAP_NRRD_FILE)
input/$(FLATMAP_BOTH_NRRD_FILE): SRC := $(POSTPROC_ROOT)/both_hemispheres/output/$(FLATMAP_BOTH_NRRD_FILE)
input/$(FLATVIEW_DATA_NRRD_FILE): SRC := $(FLATVIEW_DATA_INPUT_FILE)

# auxiliary commands
view: view-image

include ../../common_footer.mk
