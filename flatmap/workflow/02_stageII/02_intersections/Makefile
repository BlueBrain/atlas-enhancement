include ../../common_header.mk

output/$(VOXEL_PROJECTIONS_INDEX_FILE) output/$(VOXEL_PROJECTIONS_POSITION_FILE): input/$(VOXEL_PROJECTIONS_FILE)
	$(AWK) '$$4 != "nan" && $$5 != "nan" && $$6 != "nan" { \
	   print $$1,$$2,$$3 > "output/$(VOXEL_PROJECTIONS_INDEX_FILE)"; \
	   print $$4,$$5,$$6 > "output/$(VOXEL_PROJECTIONS_POSITION_FILE)" }' $<

# input links
input/$(VOXEL_PROJECTIONS_FILE): SRC := $(STAGE_II_ROOT)/01_streamlines/output/$(VOXEL_PROJECTIONS_FILE)

# auxiliary commands
view: view-intersections

.PHONY: view-intersections
view-intersections:
	$(ECHO) "set view equal xyz; splot 'output/$(VOXEL_PROJECTIONS_POSITION_FILE)' w dots" > .tmp.gp
	$(GNUPLOT) -p .tmp.gp -
	$(RM) .tmp.gp

include ../../common_footer.mk
