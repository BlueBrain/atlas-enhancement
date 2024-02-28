## user input (atlas data)
RELATIVE_DEPTH_FILE := relative_depth.nrrd
ORIENTATION_X_FILE := orientation_x.nrrd
ORIENTATION_Y_FILE := orientation_y.nrrd
ORIENTATION_Z_FILE := orientation_z.nrrd
ANNOTATIONS_FILE := annotations.nrrd
MASK_FILE := mask.nrrd
USER_INPUT_ORIENTATION_FILES := $(ORIENTATION_X_FILE)\
								$(ORIENTATION_Y_FILE)\
								$(ORIENTATION_Z_FILE)
USER_INPUT_FILES := $(RELATIVE_DEPTH_FILE)\
					$(USER_INPUT_ORIENTATION_FILES)

define required_user_file
ifeq (,$(wildcard $1))
$$(error Required user input file $$$$USER_DATA_ROOT/$$(notdir $1) not found)
endif
endef

$(eval $(call required_user_file,$(USER_DATA_ROOT)/config.mk))
$(foreach f,$(USER_INPUT_FILES),$(eval $(call required_user_file,$(USER_DATA_ROOT)/$f)))
