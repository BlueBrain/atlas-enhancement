include ../../common_header.mk

$(OUTPUTS): output/$(call METRICS_PREIMAGE_SIZE_UNIFORMITY_NRRD_FILE,%): input/$(FLATMAP_NRRD_FILE)
	$(METRICS_PREIMAGE_SIZE_UNIFORMITY_BIN) $< $* $@

# input links
input/$(FLATMAP_NRRD_FILE): SRC := $(STAGE_III_ROOT)/02_flatmap/output/$(FLATMAP_NRRD_FILE)

# auxiliary commands
view: view-nrrd

include ../../common_footer.mk
