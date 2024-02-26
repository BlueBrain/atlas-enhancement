include ../../common_header.mk

output/$(FLATMAP_NRRD_FILE): input/$(RELATIVE_DEPTH_FILE) output/$(FLATMAP_FILE)
	$(MAKE_FLATMAP_BIN) $< $(word 2,$^) $@ $(FLATMAP_EXTRA)

output/$(FLATMAP_FILE): input/$(NEAREST_VERTICES_FILE) input/$(VOXEL_PROJECTIONS_INDEX_FILE) input/$(FLAT_MESH_FILE)
	$(AWK) 'NR == 2 { nv=$$1; } NR > 2 && NR <= 2 + nv;' $(word 3,$^) > .tmp.xyz
	$(AWK) '{ print $$1 }' $< | $(SED) 's/^v//' \
		| $(PASTE) - $(word 2,$^) \
		| $(AWK) '\
		BEGIN { i = 0; while(getline < ARGV[2] > 0) { fx[i] = $$1; fy[i] = $$2; i++ }; ARGC-- } \
		{ print $$2,$$3,$$4,fx[$$1],fy[$$1] }' \
		- .tmp.xyz > $@
	$(AWK) '$$4 < 0 || $$4 > 1 || $$5 < 0 || $$5 > 1 { print "bad point found: ",$$4,$$5,NR; exit 1 }' $@
	$(RM) .tmp.xyz

# input links
input/$(NEAREST_VERTICES_FILE): SRC := $(STAGE_III_ROOT)/01_nearest/output/$(NEAREST_VERTICES_FILE)
input/$(VOXEL_PROJECTIONS_INDEX_FILE): SRC := $(STAGE_II_ROOT)/02_intersections/output/$(VOXEL_PROJECTIONS_INDEX_FILE)
input/$(FLAT_MESH_FILE): SRC := $(STAGE_I_ROOT)/04_flatten_mesh/output/$(FLAT_MESH_FILE)

input/$(RELATIVE_DEPTH_FILE): $(USER_DATA_ROOT)/$(RELATIVE_DEPTH_FILE)
	$(LN) -sf $< $@

include ../../common_footer.mk
