ifndef ATLAS_ENHANCEMENT_ROOT
$(error Please set ATLAS_ENHANCEMENT_ROOT to the location of atlas-enhancement)
endif

ifndef USER_DATA_ROOT
$(error Please set USER_DATA_ROOT to the location of user data directory)
endif

SOURCE_CODE_ROOT := $(ATLAS_ENHANCEMENT_ROOT)/flatmap/code
WORKFLOW_ROOT := $(ATLAS_ENHANCEMENT_ROOT)/flatmap/workflow
include $(WORKFLOW_ROOT)/os_detect.mk
include $(WORKFLOW_ROOT)/userdata.mk

# set shell (more modern than sh)
SHELL := /bin/bash

# paths to common system utilities
CMP := cmp
GREP := grep
ifeq (Darwin,$(DETECTED_OS))
# use GNU versions, not BSD versions
MV := gmv
LN := gln
AWK := gawk
SED := gsed
CAT := gcat
SEQ := gseq
ECHO := gecho
HEAD := ghead
EXPR := gexpr
PASTE := gpaste
else
MV := mv
LN := ln
AWK := awk
SED := sed
CAT := cat
SEQ := seq
ECHO := echo
HEAD := head
EXPR := expr
PASTE := paste
endif

# path to Python 3
PYTHON3 := python3

# path to GNU parallel
PARALLEL := parallel

# paths to auxiliary programs (optional)
GNUPLOT := gnuplot
GEOMVIEW := geomview
IMAGE_VIEWER := feh
NRRD_VIEWER := xgrid3d -nosel
NRRD_EDITOR := xgrid3d

# DO NOT EDIT BELOW THIS LINE
STAGE_I_ROOT := $(WORKFLOW_ROOT)/01_stageI
STAGE_II_ROOT := $(WORKFLOW_ROOT)/02_stageII
STAGE_III_ROOT := $(WORKFLOW_ROOT)/03_stageIII
METRICS_ROOT := $(WORKFLOW_ROOT)/metrics
APPLICATIONS_ROOT := $(WORKFLOW_ROOT)/applications

## steps
FLATMAP_STEPS := 01_stageI\
				 02_stageII\
				 03_stageIII

# NOTE: include possible only from WORKFLOW_ROOT
-include postproc/config.mk

POSTPROC_STEPS := $(STEPS)
POSTPROC_STEPS := $(addprefix postproc/,$(POSTPROC_STEPS))

# NOTE: include possible only from WORKFLOW_ROOT
-include metrics/config.mk

METRICS_STEPS := $(STEPS)
METRICS_STEPS := $(addprefix metrics/,$(METRICS_STEPS))

DISCRETE_STEPS := $(FLATMAP_STEPS)\
				  $(addprefix metrics/,$(DISCRETE_STEPS))

# NOTE: include possible only from WORKFLOW_ROOT
-include applications/config.mk

APPLICATIONS_STEPS := $(STEPS)
APPLICATIONS_STEPS := $(addprefix applications/,$(APPLICATIONS_STEPS))

STEPS := $(FLATMAP_STEPS)\
		 $(POSTPROC_STEPS)\
		 $(METRICS_STEPS)\
		 $(APPLICATIONS_STEPS)

DEFAULT_STEPS := $(FLATMAP_STEPS)

## stageI
ISOSURFACE_NRRD_FILE := isosurface_dots.nrrd
ISOSURFACE_DOTS_FILE := isosurface_dots.xyz
PROJECTION_MESH_FILE := projection_mesh.off
REFINED_MESH_FILE := refined_mesh.off
FLAT_MESH_FILE := flat_mesh.off
## stageII
VOXEL_PROJECTIONS_FILE := voxel_projections.txt
DIMENSIONS_FILE := dimensions.txt
VOXEL_PROJECTIONS_INDEX_FILE := voxel_projections_index.xyz
VOXEL_PROJECTIONS_POSITION_FILE := voxel_projections_position.xyz
## stageIII
NEAREST_VERTICES_FILE := nearest_vertices.txt
FLATMAP_FILE := flatmap.txt
FLATMAP_NRRD_FILE := flatmap.nrrd
FLATMAP_DISCRETE_NRRD_FILE := flatmap_discrete.nrrd
