include ../../common_header.mk

output/$(METRICS_ORTHOGONALITY_NRRD_FILE): input/$(FLATMAP_NRRD_FILE) $(addprefix input/,$(ORIENTATION_FILES)) input/$(MASK_FILE)
	$(METRICS_ORTHOGONALITY_BIN) $(wordlist 1,5,$^) $@

# input links
input/$(FLATMAP_NRRD_FILE): SRC := $(STAGE_III_ROOT)/02_flatmap/output/$(FLATMAP_NRRD_FILE)

# auxiliary commands
view: view-nrrd

include ../../common_footer.mk
