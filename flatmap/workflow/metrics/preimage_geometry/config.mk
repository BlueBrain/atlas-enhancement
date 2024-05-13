# paths to executables
METRICS_PREIMAGE_GEOMETRY_BIN := $(PYTHON3) $(SOURCE_CODE_ROOT)/metrics/preimage_geometry.py
EXTRACT_NRRD_Z_BIN := $(PYTHON3) $(SOURCE_CODE_ROOT)/utils/extract_nrrd_z.py

# DO NOT EDIT BELOW THIS LINE
override INPUTS := input/$(FLATMAP_NRRD_FILE)\
	$(addprefix input/,$(USER_INPUT_ORIENTATION_FILES))

OUTPUTS_GEOMETRY = $(addprefix output/,$(foreach r,$(METRICS_RESOLUTION),$(call METRICS_PREIMAGE_GEOMETRY_NRRD_FILE,$r)))
OUTPUTS_RADIUS = $(addprefix output/,$(foreach r,$(METRICS_RESOLUTION),$(call METRICS_PREIMAGE_RADIUS_NRRD_FILE,$r)))
OUTPUTS_CONICITY = $(addprefix output/,$(foreach r,$(METRICS_RESOLUTION),$(call METRICS_PREIMAGE_CONICITY_NRRD_FILE,$r)))
override OUTPUTS = $(OUTPUTS_RADIUS) $(OUTPUTS_CONICITY)

override INTERMEDIATE = $(OUTPUTS_GEOMETRY)

override USER_PARAMETERS := METRICS_RESOLUTION

override USER_BINARIES := METRICS_PREIMAGE_GEOMETRY_BIN\
	EXTRACT_NRRD_Z_BIN
