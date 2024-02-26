include ../../common_header.mk

output/$(FLATMAP_DISCRETE_NRRD_FILE): input/$(FLATMAP_NRRD_FILE)
	$(DISCRETIZE_FLATMAP_BIN) $< $(PIXEL_RESOLUTION) $@

# input links
input/$(FLATMAP_NRRD_FILE): SRC := $(STAGE_III_ROOT)/02_flatmap/output/$(FLATMAP_NRRD_FILE)

include ../../common_footer.mk
