# This makefile is meant to be included by flatmap.mk

# Required utilities
PARALLEL ?= parallel
JQ ?= jq

FLATPATH_TRIANGLES := $(FLATPATH_ROOT)/triangles.py

ifeq (,$(wildcard ./regions.json)) # regions.json does not exist
# Basic straight mapping
regions.json: regions.txt
	$(AWK) '\
		BEGIN { print "{" } \
		{ printf "\"%s\": [\"%s\"],\n",$$1,$$1 } \
		END { print "}" } \
		' $< | sed -z 's/,\n}/\n}/' > $@
endif

# Generate starting triangles and plots
.PHONY: triangles
triangles: triangles.json
triangles.json: flatmap.nrrd $(ATLAS_HIERARCHY) $(ATLAS_REGIONS) regions.json
	$(LOAD_VOXCELL)
	$(PYTHON3) $(FLATPATH_TRIANGLES) $^ > $@

triangles_corrected.json:
	@echo '$@: Generate this file by editing a copy of triangles.json'

# Generate plots with correctly-oriented triangles
.PHONY: triangles-corrected
triangles-corrected: flatmap.nrrd $(ATLAS_HIERARCHY) $(ATLAS_REGIONS) regions.json triangles_corrected.json
	$(LOAD_VOXCELL)
	$(PYTHON3) $(FLATPATH_TRIANGLES) $^

.PHONY: map-compare
map-compare: regions.json
	$(JQ) 'to_entries[] | .key + " " + (.value | join("_"))' $< | $(SED) 's/\"//g' \
		 | $(PARALLEL) -C' ' convert +append {1}.png {2}.png map_{1}_{2}.png

