## steps
GLOBAL_STEPS := coverage\
				usage_fraction\
				usage_fraction_discrete

PER_VERTEX_STEPS := preimage_size

PER_PIXEL_STEPS := pairwise_preimage_continuity\
				   preimage_connectedness\
				   preimage_geometry\
				   preimage_size_discrete\
				   preimage_size_uniformity

PER_VOXEL_STEPS := divergence\
				   laplacian\
				   orthogonality

DISCRETE_STEPS := $(PER_PIXEL_STEPS)\
				  usage_fraction_discrete\
				  heatmaps

STEPS := $(GLOBAL_STEPS)\
		 $(PER_VERTEX_STEPS)\
		 $(PER_PIXEL_STEPS)\
		 $(PER_VOXEL_STEPS)\
		 heatmaps

DEFAULT_STEPS := $(STEPS)

## file names
METRICS_COVERAGE_FILE := coverage.txt
METRICS_USAGE_FRACTION_FILE := usage_fraction.txt
METRICS_PREIMAGE_SIZE_FILE := preimage_size.txt
METRICS_USAGE_FRACTION_DISCRETE_FILE := usage_fraction_discrete.txt
METRICS_ORTHOGONALITY_NRRD_FILE := orthogonality.nrrd
METRICS_DIVERGENCE_NRRD_FILE := divergence.nrrd
METRICS_LAPLACIAN_NRRD_FILE := laplacian.nrrd

METRICS_PREIMAGE_SIZE_DISCRETE_NRRD_FILE := preimage_size_discrete.nrrd
METRICS_PREIMAGE_SIZE_UNIFORMITY_NRRD_FILE := preimage_size_uniformity.nrrd
METRICS_PREIMAGE_CONNECTEDNESS_NRRD_FILE := preimage_connectedness.nrrd
METRICS_PAIRWISE_PREIMAGE_CONTINUITY_NRRD_FILE := pairwise_preimage_continuity.nrrd
METRICS_PREIMAGE_GEOMETRY_NRRD_FILE := preimage_geometry.nrrd
METRICS_PREIMAGE_RADIUS_NRRD_FILE := preimage_radius.nrrd
METRICS_PREIMAGE_CONICITY_NRRD_FILE := preimage_conicity.nrrd

METRICS_DISCRETE := PAIRWISE_PREIMAGE_CONTINUITY\
					PREIMAGE_CONICITY\
					PREIMAGE_CONNECTEDNESS\
					PREIMAGE_RADIUS\
					PREIMAGE_SIZE_DISCRETE\
					PREIMAGE_SIZE_UNIFORMITY
