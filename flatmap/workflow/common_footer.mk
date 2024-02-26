$(addprefix input/,$(USER_INPUT_FILES)) input/$(ANNOTATIONS_FILE) input/$(MASK_FILE): input/%: $(USER_DATA_ROOT)/%
	$(LN) -sf $< $@

input/%:
	$(LN) -rsf $(SRC) $@

$(OUTPUTS): .userconfig.mk

# export
.PHONY: export
ifdef EXPORT_DIR
export:
	cp $(OUTPUTS) $(EXPORT_DIR)
else
export:
	$(error Please set EXPORT_DIR)
endif

# view
.PHONY: view-nrrd
view-nrrd:
	$(foreach o,$(OUTPUTS),$(NRRD_VIEWER) $o;)

.PHONY: view-image
view-image:
	$(foreach o,$(OUTPUTS),$(IMAGE_VIEWER) $o;)

# cleanup
.PHONY: clean clean-all clean-inputs clean-outputs
clean-input:
	$(RM) $(INPUTS)

clean-output:
	$(RM) $(OUTPUTS)
ifdef INTERMEDIATE
	$(RM) $(INTERMEDIATE)
endif

clean: clean-output
ifdef CLEAN_EXTRA
	$(RM) $(CLEAN_EXTRA)
endif

clean-all: clean clean-input
	$(RM) .userconfig.mk
