# path to executable
FLATTEN_MESH_BIN := $(SOURCE_CODE_ROOT)/surf_cgal/build/Flatten_Authalic

# DO NOT EDIT BELOW THIS LINE
override INPUTS := input/$(REFINED_MESH_FILE)

override OUTPUTS := output/$(FLAT_MESH_FILE)

override USER_PARAMETERS := FLATTEN_MESH_EXTRA

override USER_BINARIES := FLATTEN_MESH_BIN
