include ../../common_header.mk

output/$(PROJECTION_MESH_FILE): input/$(ISOSURFACE_DOTS_FILE)
	$(RECONSTRUCT_SURFACE_BIN) $< $@ $(RECONSTRUCT_SURFACE_EXTRA)

# input links
input/$(ISOSURFACE_DOTS_FILE): SRC := $(STAGE_I_ROOT)/01_extract_surface/output/$(ISOSURFACE_DOTS_FILE)

# auxiliary commands
view: view-projection-mesh

.PHONY: view-projection-mesh
view-projection-mesh:
	$(GEOMVIEW) output/$(PROJECTION_MESH_FILE)

include ../../common_footer.mk
