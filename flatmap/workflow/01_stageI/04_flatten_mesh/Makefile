include ../../common_header.mk

output/$(FLAT_MESH_FILE): input/$(REFINED_MESH_FILE)
	$(FLATTEN_MESH_BIN) $< $@ $(FLATTEN_MESH_EXTRA)

# input links
input/$(REFINED_MESH_FILE): SRC := $(STAGE_I_ROOT)/03_refine_mesh/output/$(REFINED_MESH_FILE)

# auxiliary commands
view: view-flat-mesh

.PHONY: view-flat-mesh
view-flat-mesh:
	$(GEOMVIEW) output/$(FLAT_MESH_FILE)

include ../../common_footer.mk
