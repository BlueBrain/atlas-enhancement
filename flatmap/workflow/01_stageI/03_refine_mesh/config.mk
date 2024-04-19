# paths to executables
GMSH_BIN := gmsh
$(call required,$(GMSH_BIN))
OFF2PLY_BIN := $(AWK) -f $(SOURCE_CODE_ROOT)/utils/off2ply.awk
PLY2OFF_BIN := $(AWK) -f $(SOURCE_CODE_ROOT)/utils/ply2off.awk

# DO NOT EDIT BELOW THIS LINE
override INPUTS := input/$(PROJECTION_MESH_FILE)

override OUTPUTS := output/$(REFINED_MESH_FILE)

override USER_PARAMETERS := NREFINE

override USER_BINARIES := GMSH_BIN\
	OFF2PLY_BIN\
	PLY2OFF_BIN
