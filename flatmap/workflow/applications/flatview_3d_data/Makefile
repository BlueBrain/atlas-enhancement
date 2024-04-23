include ../../common_header.mk

ifndef FLATVIEW_DATA_INPUT_FILE
$(error Please set FLATVIEW_DATA_INPUT_FILE to location of input NRRD data file)
endif

output/$(FLATVIEW_DATA_PNG_FILE): input/$(FLATMAP_NRRD_FILE) input/$(FLATVIEW_DATA_INPUT_FILE)
	$(FLATPLOT_BIN) $(FLATPLOT_DATA_EXTRA) $< $(word 2,$^) $(subst .png,,$@)

# input links
input/$(FLATMAP_NRRD_FILE): SRC := $(STAGE_III_ROOT)/02_flatmap/output/$(FLATMAP_NRRD_FILE)
input/$(FLATVIEW_DATA_INPUT_FILE): SRC := $(FLATVIEW_DATA_INPUT_FILE)

# auxiliary commands
view: view-image

include ../../common_footer.mk
