include ../../common_header.mk

output/$(METRICS_LAPLACIAN_NRRD_FILE): input/$(RELATIVE_DEPTH_FILE) input/$(MASK_FILE)
	$(METRICS_LAPLACIAN_BIN) $(wordlist 1,2,$^) $@

# auxiliary commands
view: view-nrrd

include ../../common_footer.mk
