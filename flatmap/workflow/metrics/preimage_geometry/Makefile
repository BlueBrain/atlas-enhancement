include ../../common_header.mk

output/$(METRICS_PREIMAGE_RADIUS_NRRD_FILE): ZINDEX := 0
output/$(METRICS_PREIMAGE_CONICITY_NRRD_FILE): ZINDEX := 1
$(OUTPUTS): output/$(METRICS_PREIMAGE_GEOMETRY_NRRD_FILE)
	$(EXTRACT_NRRD_Z_BIN) $< $(ZINDEX) $@

output/$(METRICS_PREIMAGE_GEOMETRY_NRRD_FILE): input/$(FLATMAP_DISCRETE_NRRD_FILE) input/$(ORIENTATION_X_FILE) input/$(ORIENTATION_Y_FILE) input/$(ORIENTATION_Z_FILE)
	$(METRICS_PREIMAGE_GEOMETRY_BIN) $< $(PIXEL_RESOLUTION) $(wordlist 2,4,$^) $@ $(NCPU)

# input links
input/$(FLATMAP_DISCRETE_NRRD_FILE): SRC := $(STAGE_III_ROOT)/03_discretization/output/$(FLATMAP_DISCRETE_NRRD_FILE)

# auxiliary commands
view: view-nrrd

include ../../common_footer.mk
