include ../../common_header.mk

output/$(METRICS_PER_VOXEL_PLOT_FILE): output/divergence.txt output/laplacian.txt output/orthogonality.txt
	$(PLOT_PER_VOXEL_BIN) $(wordlist 1,3,$^) $@

output/divergence.txt: input/$(METRICS_DIVERGENCE_NRRD_FILE)
output/laplacian.txt: input/$(METRICS_LAPLACIAN_NRRD_FILE)
output/orthogonality.txt: input/$(METRICS_ORTHOGONALITY_NRRD_FILE)
output/divergence.txt output/laplacian.txt output/orthogonality.txt: input/$(MASK_FILE)
	$(DUMP_VALUES_BIN) $(word 2,$^) $@ $<

# input links
input/$(METRICS_DIVERGENCE_NRRD_FILE): SRC := $(METRICS_ROOT)/divergence/output/$(METRICS_DIVERGENCE_NRRD_FILE)
input/$(METRICS_LAPLACIAN_NRRD_FILE): SRC := $(METRICS_ROOT)/laplacian/output/$(METRICS_LAPLACIAN_NRRD_FILE)
input/$(METRICS_ORTHOGONALITY_NRRD_FILE): SRC := $(METRICS_ROOT)/orthogonality/output/$(METRICS_ORTHOGONALITY_NRRD_FILE)

# auxiliary commands
view: view-image

include ../../common_footer.mk
