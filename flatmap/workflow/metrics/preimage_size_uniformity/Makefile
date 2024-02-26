include ../../common_header.mk

output/$(METRICS_PREIMAGE_SIZE_UNIFORMITY_NRRD_FILE): input/$(FLATMAP_DISCRETE_NRRD_FILE)
	$(METRICS_PREIMAGE_SIZE_UNIFORMITY_BIN) $< $(PIXEL_RESOLUTION) $@

# input links
input/$(FLATMAP_DISCRETE_NRRD_FILE): SRC := $(STAGE_III_ROOT)/03_discretization/output/$(FLATMAP_DISCRETE_NRRD_FILE)

# auxiliary commands
view: view-nrrd

include ../../common_footer.mk
