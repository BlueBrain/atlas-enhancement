include ../../common_header.mk

$(OUTPUTS_CONICITY): output/$(call METRICS_PREIMAGE_CONICITY_NRRD_FILE,%): output/$(call METRICS_PREIMAGE_GEOMETRY_NRRD_FILE,%)
	$(EXTRACT_NRRD_Z_BIN) $< 1 $@

$(OUTPUTS_RADIUS): output/$(call METRICS_PREIMAGE_RADIUS_NRRD_FILE,%): output/$(call METRICS_PREIMAGE_GEOMETRY_NRRD_FILE,%)
	$(EXTRACT_NRRD_Z_BIN) $< 0 $@

$(OUTPUTS_GEOMETRY): output/$(call METRICS_PREIMAGE_GEOMETRY_NRRD_FILE,%): input/$(FLATMAP_NRRD_FILE) input/$(ORIENTATION_X_FILE) input/$(ORIENTATION_Y_FILE) input/$(ORIENTATION_Z_FILE)
	$(METRICS_PREIMAGE_GEOMETRY_BIN) $< $* $(wordlist 2,4,$^) $@ $(NCPU)

# input links
input/$(FLATMAP_NRRD_FILE): SRC := $(STAGE_III_ROOT)/02_flatmap/output/$(FLATMAP_NRRD_FILE)

# auxiliary commands
view: view-nrrd

include ../../common_footer.mk
