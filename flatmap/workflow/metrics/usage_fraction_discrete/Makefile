include ../../common_header.mk

NJOB ?= $(words $(METRICS_RESOLUTION))
output/$(METRICS_USAGE_FRACTION_DISCRETE_FILE): input/$(FLATMAP_NRRD_FILE)
	$(PARALLEL) -k -j$(NJOB) $(METRICS_USAGE_FRACTION_BIN) $< {} ::: $(METRICS_RESOLUTION) > $@

# input links
input/$(FLATMAP_NRRD_FILE): SRC := $(STAGE_III_ROOT)/02_flatmap/output/$(FLATMAP_NRRD_FILE)

# auxiliary commands
view: view-txt

include ../../common_footer.mk
