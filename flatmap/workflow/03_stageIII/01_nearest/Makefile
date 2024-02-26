include ../../common_header.mk

output/$(NEAREST_VERTICES_FILE): input/$(REFINED_MESH_FILE) input/$(VOXEL_PROJECTIONS_POSITION_FILE)
	$(SPATIAL_NN_SEARCH_BIN) $(wordlist 1,2,$^) > $@

# input links
input/$(REFINED_MESH_FILE): SRC := $(STAGE_I_ROOT)/03_refine_mesh/output/$(REFINED_MESH_FILE)
input/$(VOXEL_PROJECTIONS_POSITION_FILE): SRC := $(STAGE_II_ROOT)/02_intersections/output/$(VOXEL_PROJECTIONS_POSITION_FILE)

# auxiliary commands
view: view-error

.PHONY: view-error
.SILENT: view-error
view-error:
	$(AWK) 'BEGIN { min = 1E32; max = -1E32; } \
		{ if($$5 < min) min = $$5; if($$5 > max) max = $$5; } \
		END { printf "Minimum projection error: %s\n",min; \
		printf "Maximum projection error: %s\n",max; }' output/$(NEAREST_VERTICES_FILE) \
		>/dev/stderr

include ../../common_footer.mk
