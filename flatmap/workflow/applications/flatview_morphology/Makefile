include ../../common_header.mk

ifndef FLATVIEW_MORPHO_INPUT_FILE
$(error Please set FLATVIEW_MORPHO_INPUT_FILE to location of input JSON morphology file)
endif

TMPFILE_SOMA = $@.soma.tmp
TMPFILE_AXON = $@.axon.tmp
FLATPLOT_FLAGS := --dots
ifeq (1,$(REFLECT))
FLATPLOT_FLAGS += --reflect
endif
output/$(FLATVIEW_MORPHO_PNG_FILE): input/$(FLATVIEW_MORPHO_INPUT_FILE) input/$(FLATMAP_NRRD_FILE) input/$(FLATVIEW_ANNOTATIONS_FILE)
	$(JQ) '.neuron.soma | .x, .y, .z' $< > $(TMPFILE_SOMA)
	$(JQ) '.neuron.axon[] | .x, .y, .z' $< > $(TMPFILE_AXON)
	$(FLATPLOT_BIN) $(FLATPLOT_FLAGS) -c '#FFFFFF' -p $(PIXEL_RESOLUTION) --spread 1 $(FLATPLOT_MORPHO_EXTRA) $(word 2,$^) $(TMPFILE_SOMA) $(TMPFILE_SOMA)
	$(FLATPLOT_BIN) $(FLATPLOT_FLAGS) -c '#000000' -p $(PIXEL_RESOLUTION) $(FLATPLOT_MORPHO_EXTRA) $(word 2,$^) $(TMPFILE_AXON) $(TMPFILE_AXON)
	$(MAGICK) composite -blend 50 $(word 3,$^) -size $(PIXEL_RESOLUTION)x$(PIXEL_RESOLUTION) xc:white -alpha Set $@.tmp.png
	$(MAGICK) composite -compose CopyOpacity $(word 3,$^) $@.tmp.png $@.tmp.png
	$(MAGICK) composite $(TMPFILE_AXON).png $@.tmp.png - \
		| $(MAGICK) composite $(TMPFILE_SOMA).png - $@
	$(RM) $(TMPFILE_SOMA) $(TMPFILE_AXON) $(TMPFILE_SOMA).png $(TMPFILE_AXON).png $@.tmp.png

# input links
input/$(FLATMAP_NRRD_FILE): SRC := $(STAGE_III_ROOT)/02_flatmap/output/$(FLATMAP_NRRD_FILE)
input/$(FLATVIEW_ANNOTATIONS_FILE): SRC := $(APPLICATIONS_ROOT)/flatview_annotations/output/$(FLATVIEW_ANNOTATIONS_FILE)

# auxiliary commands
view: view-image

include ../../common_footer.mk
