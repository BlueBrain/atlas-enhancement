include ../../common_header.mk

output/$(METRICS_COVERAGE_FILE): input/$(FLATMAP_NRRD_FILE) input/$(RELATIVE_DEPTH_FILE)
	$(METRICS_COVERAGE_BIN) $< $(word 2,$^) > $@

# input links
input/$(FLATMAP_NRRD_FILE): SRC := $(STAGE_III_ROOT)/02_flatmap/output/$(FLATMAP_NRRD_FILE)

include ../../common_footer.mk
