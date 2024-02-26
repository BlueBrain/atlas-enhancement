include ../../common_header.mk

output/$(METRICS_USAGE_FRACTION_FILE): input/$(NEAREST_VERTICES_FILE) input/$(FLAT_MESH_FILE)
	$(AWK) '{ cnt[$$1]++ } END { sum = 0; n = 0; for(k in cnt) { n++; sum += cnt[k] } print n,sum / n }' $< \
		| $(AWK) 'NR == 1 { nvtx_used = $$1; nvox_mean = $$2 } \
				  NR == 3 { nvtx_total = $$1 } \
				  END { print nvtx_used / nvtx_total,nvox_mean }' - $(word 2,$^) \
		> $@

# input links
input/$(FLAT_MESH_FILE): SRC := $(STAGE_I_ROOT)/04_flatten_mesh/output/$(FLAT_MESH_FILE)
input/$(NEAREST_VERTICES_FILE): SRC := $(STAGE_III_ROOT)/01_nearest/output/$(NEAREST_VERTICES_FILE)

include ../../common_footer.mk
