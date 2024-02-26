include ../../common_header.mk

output/$(METRICS_DIVERGENCE_NRRD_FILE): $(addprefix input/,$(USER_INPUT_ORIENTATION_FILES)) input/$(MASK_FILE)
	$(METRICS_DIVERGENCE_BIN) $(wordlist 1,4,$^) $@

# auxiliary commands
view: view-nrrd

include ../../common_footer.mk
