include ../../common_header.mk

output/$(METRICS_PAIRWISE_PREIMAGE_CONTINUITY_NRRD_FILE): input/$(FLATMAP_DISCRETE_NRRD_FILE)
	$(METRICS_PAIRWISE_PREIMAGE_CONTINUITY_BIN) $< $(PIXEL_RESOLUTION) $@ $(NCPU)

# input links
input/$(FLATMAP_DISCRETE_NRRD_FILE): SRC := $(STAGE_III_ROOT)/03_discretization/output/$(FLATMAP_DISCRETE_NRRD_FILE)

# auxiliary commands
view: view-nrrd

include ../../common_footer.mk
