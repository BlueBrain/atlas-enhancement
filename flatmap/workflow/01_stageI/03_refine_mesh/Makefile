include ../../common_header.mk

TMPFILE_PLY := $(subst .off,.ply2,output/$(REFINED_MESH_FILE))
output/$(REFINED_MESH_FILE): input/$(PROJECTION_MESH_FILE)
	$(OFF2PLY_BIN) $< $(TMPFILE_PLY)
	for i in `$(SEQ) 1 $(NREFINE)`; do \
		$(GMSH_BIN) $(TMPFILE_PLY) -2 -refine -format ply2 -o $(TMPFILE_PLY); \
	done
	$(PLY2OFF_BIN) $(TMPFILE_PLY) $@
	$(RM) $(TMPFILE_PLY)

# input links
input/$(PROJECTION_MESH_FILE): SRC := $(STAGE_I_ROOT)/02_reconstruct_mesh/output/$(PROJECTION_MESH_FILE)

# auxiliary commands
view: view-refined-mesh

.PHONY: view-refined-mesh
view-refined-mesh:
	$(GEOMVIEW) output/$(REFINED_MESH_FILE)

include ../../common_footer.mk
