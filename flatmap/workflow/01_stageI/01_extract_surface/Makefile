include ../../common_header.mk

output/$(ISOSURFACE_DOTS_FILE): output/$(ISOSURFACE_NRRD_FILE)
	$(EXTRACT_DOTS_BIN) $< $@

output/$(ISOSURFACE_NRRD_FILE): input/$(RELATIVE_DEPTH_FILE)
	$(EXTRACT_ISOSURFACE_VOXELS_BIN) $< $(PROJECTION_SURFACE_DSTAR) $(PROJECTION_SURFACE_SIDE) $@

# auxiliary commands
view: view-isosurface-dots

.PHONY: view-isosurface-dots
view-isosurface-dots:
	$(ECHO) "set view equal xyz; splot 'output/$(ISOSURFACE_DOTS_FILE)' w dots" > .tmp.gp
	$(GNUPLOT) -p .tmp.gp -
	$(RM) .tmp.gp

## do manual edits
.PHONY:
edit:
	$(NRRD_EDITOR) output/$(ISOSURFACE_NRRD_FILE)

include ../../common_footer.mk
