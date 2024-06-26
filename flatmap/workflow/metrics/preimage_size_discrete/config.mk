# path to executable
METRICS_PREIMAGE_SIZE_BIN := $(PYTHON3) $(SOURCE_CODE_ROOT)/metrics/preimage_size.py

# DO NOT EDIT BELOW THIS LINE
override INPUTS := input/$(FLATMAP_NRRD_FILE)

override OUTPUTS = $(addprefix output/,$(foreach r,$(METRICS_RESOLUTION),$(call METRICS_PREIMAGE_SIZE_DISCRETE_NRRD_FILE,$r)))

override USER_PARAMETERS := METRICS_RESOLUTION

override USER_BINARIES := METRICS_PREIMAGE_SIZE_BIN
