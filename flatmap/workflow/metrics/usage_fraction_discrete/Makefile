include ../../common_header.mk

output/$(METRICS_USAGE_FRACTION_DISCRETE_FILE): input/$(FLATMAP_DISCRETE_NRRD_FILE)
	$(METRICS_USAGE_FRACTION_BIN) $< $(PIXEL_RESOLUTION) > $@

# input links
input/$(FLATMAP_DISCRETE_NRRD_FILE): SRC := $(STAGE_III_ROOT)/03_discretization/output/$(FLATMAP_DISCRETE_NRRD_FILE)

include ../../common_footer.mk
