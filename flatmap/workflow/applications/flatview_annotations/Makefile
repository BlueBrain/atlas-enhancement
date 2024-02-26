include ../../common_header.mk

output/$(FLATVIEW_ANNOTATIONS_FILE): input/$(FLATMAP_NRRD_FILE) input/$(ANNOTATIONS_FILE)
	$(FLATPLOT_BIN) --regions --colormap $(FLATVIEW_ANNOTATIONS_COLORMAP) -p $(PIXEL_RESOLUTION) \
		$< $(word 2,$^) $(subst .png,,$@)

# input links
input/$(FLATMAP_NRRD_FILE): SRC := $(STAGE_III_ROOT)/02_flatmap/output/$(FLATMAP_NRRD_FILE)

# auxiliary commands
view: view-image

include ../../common_footer.mk
