PYTHON := python3

CCFV3_INPUT_DIR := input
OUTPUT_DIR := output
ORIENTATION_OUTPUTS := $(OUTPUT_DIR)/orientation_x.nrrd $(OUTPUT_DIR)/orientation_y.nrrd $(OUTPUT_DIR)/orientation_z.nrrd
ALL_OUTPUTS := $(OUTPUT_DIR)/annotations.nrrd $(OUTPUT_DIR)/mask.nrrd $(OUTPUT_DIR)/relative_depth.nrrd $(ORIENTATION_OUTPUTS)

.DELETE_ON_ERROR: $(ALL_OUTPUTS)

.PHONY: all
all: $(ALL_OUTPUTS)

# outside, inside, sides, bottom, top
$(OUTPUT_DIR)/mask.nrrd: $(OUTPUT_DIR)/mask_hemi.nrrd $(CCFV3_INPUT_DIR)/isocortex_mask_10.nrrd $(CCFV3_INPUT_DIR)/isocortex_boundary_10.nrrd | $(OUTPUT_DIR)/
	echo -e "\
import numpy as np                                   \n\
from voxcell import VoxelData                        \n\
hem = VoxelData.load_nrrd('$<')                      \n\
iso = VoxelData.load_nrrd('$(word 2,$^)')            \n\
bnd = VoxelData.load_nrrd('$(word 3,$^)')            \n\
bnd.raw[bnd.raw == 4] = 2  # sides                   \n\
bnd.raw[bnd.raw == 1] = 4  # top                     \n\
bnd.raw[(bnd.raw == 0) & (iso.raw > 0)] = 1  #inner  \n\
bnd.raw[hem.raw == 0] = 0                            \n\
bnd.with_data(bnd.raw.astype(np.int8)).save_nrrd('$@')\n\
" | $(PYTHON)

# isocortex regions in one hemisphere
$(OUTPUT_DIR)/annotations.nrrd: $(OUTPUT_DIR)/mask_hemi.nrrd $(OUTPUT_DIR)/isocortex_regions_10.nrrd | $(OUTPUT_DIR)/
	echo -e "\
import numpy as np                                   \n\
from voxcell import VoxelData                        \n\
hem = VoxelData.load_nrrd('$<')                      \n\
reg = VoxelData.load_nrrd('$(word 2,$^)')            \n\
reg.raw[hem.raw == 0] = -1                           \n\
reg.save_nrrd('$@')                                  \n\
" | $(PYTHON)

# orientation vector
$(ORIENTATION_OUTPUTS): $(OUTPUT_DIR)/extension.nrrd $(OUTPUT_DIR)/mask_isocortex.nrrd | $(OUTPUT_DIR)/
	echo -e "\
import numpy as np                                   \n\
from voxcell import VoxelData                        \n\
ext = VoxelData.load_nrrd('$<')                      \n\
msk = VoxelData.load_nrrd('$(word 2,$^)')            \n\
msk_ext = (msk.raw == 0)                             \n\
dx, dy, dz = np.gradient(ext.raw)                    \n\
dx[msk_ext] = 0                                      \n\
dy[msk_ext] = 0                                      \n\
dz[msk_ext] = 0                                      \n\
msk.with_data(dx.astype(np.float32)).save_nrrd('$(word 1,$(ORIENTATION_OUTPUTS))')\n\
msk.with_data(dy.astype(np.float32)).save_nrrd('$(word 2,$(ORIENTATION_OUTPUTS))')\n\
msk.with_data(dz.astype(np.float32)).save_nrrd('$(word 3,$(ORIENTATION_OUTPUTS))')\n\
" | $(PYTHON)

# extend values before gradient computation
$(OUTPUT_DIR)/extension.nrrd: $(OUTPUT_DIR)/mask_hemi.nrrd $(OUTPUT_DIR)/mask_isocortex.nrrd $(OUTPUT_DIR)/relative_depth.nrrd $(CCFV3_INPUT_DIR)/isocortex_boundary_10.nrrd | $(OUTPUT_DIR)/
	echo -e "\
import numpy as np                                   \n\
from voxcell import VoxelData                        \n\
from scipy import ndimage                            \n\
from joblib import Parallel, delayed                 \n\
import sys                                           \n\
# find voxel in src nearest to (x,y,z)               \n\
def get_nearest(src,x,y,z):                          \n\
    D = src - (x,y,z)                                \n\
    d = np.sum(D * D, axis=1)                        \n\
    ix = np.argmin(d)                                \n\
    return src[ix]                                   \n\
# load data                                          \n\
hem = VoxelData.load_nrrd('$(word 1,$^)')            \n\
msk = VoxelData.load_nrrd('$(word 2,$^)')            \n\
rdp = VoxelData.load_nrrd('$(word 3,$^)')            \n\
bnd = VoxelData.load_nrrd('$(word 4,$^)')            \n\
# masks                                              \n\
top = (bnd.raw == 1) & (hem.raw == 1)                \n\
bottom = (bnd.raw == 3) & (hem.raw == 1)             \n\
sides = (bnd.raw == 4) & (hem.raw == 1)              \n\
msk_ext = (msk.raw == 0)                             \n\
msk_int = (msk.raw != 0)                             \n\
# extend masks                                       \n\
ball = ndimage.generate_binary_structure(3, 3)       \n\
top_ext = ndimage.binary_dilation(top, ball) & msk_ext\n\
sides_ext = ndimage.binary_dilation(sides, ball) & msk_ext\n\
bottom_ext = ndimage.binary_dilation(bottom, ball) & msk_ext\n\
sides_int = ndimage.binary_dilation(sides, ball, 2) & msk_int\n\
# extrapolate top and bottom                         \n\
rdp.raw[msk_ext] = 0  # NaN to zero                  \n\
rdp.raw[top_ext] = 1.01  # slightly above 1          \n\
rdp.raw[bottom_ext] = -0.01  # slightly below 0      \n\
# extrapolate sides                                  \n\
src = np.where(sides_int)                            \n\
dst = np.where(sides_ext)                            \n\
with Parallel(n_jobs=-1, prefer='threads') as parallel:\n\
    src_arr = np.array(src).T                        \n\
    dst_arr = np.array(dst).T                        \n\
    nearest = parallel(delayed(get_nearest)(src_arr,*x)\\n\
                       for x in dst_arr)             \n\
nst_ix = tuple([x for x in np.array(nearest).T])     \n\
rdp.raw[dst] = rdp.raw[nst_ix]                       \n\
# save data                                          \n\
rdp.with_data(rdp.raw.astype(np.float32)).save_nrrd('$@')\n\
" | $(PYTHON)

# Laplace solution as relative depth
$(OUTPUT_DIR)/relative_depth.nrrd: $(OUTPUT_DIR)/mask_isocortex.nrrd $(CCFV3_INPUT_DIR)/laplacian_10.nrrd | $(OUTPUT_DIR)/
	echo -e "\
import numpy as np                                   \n\
from voxcell import VoxelData                        \n\
msk = VoxelData.load_nrrd('$<')                      \n\
lap = VoxelData.load_nrrd('$(word 2,$^)')            \n\
res = np.full_like(msk.raw,np.nan,dtype = np.float32)\n\
bmsk = (msk.raw != 0)                                \n\
res[bmsk] = 1 - lap.raw[bmsk]                        \n\
lap.with_data(res).save_nrrd('$@')                   \n\
" | $(PYTHON)

# isocortex mask in one hemisphere
$(OUTPUT_DIR)/mask_isocortex.nrrd: $(CCFV3_INPUT_DIR)/isocortex_mask_10.nrrd $(OUTPUT_DIR)/mask_hemi.nrrd | $(OUTPUT_DIR)/
	echo -e "\
import numpy as np                                   \n\
from voxcell import VoxelData                        \n\
msk = VoxelData.load_nrrd('$<')                      \n\
hem = VoxelData.load_nrrd('$(word 2,$^)')            \n\
msk.raw = msk.raw * hem.raw                          \n\
msk.raw[msk.raw != 0] = 1                            \n\
msk.with_data(msk.raw.astype(np.uint8)).save_nrrd('$@')\n\
" | $(PYTHON)

# other hemisphere
#$(OUTPUT_DIR)/mask_hemi.nrrd: RANGE := 0, msk.shape[2] // 2
$(OUTPUT_DIR)/mask_hemi.nrrd: RANGE := msk.shape[2] // 2, msk.shape[2]
$(OUTPUT_DIR)/mask_hemi.nrrd: $(CCFV3_INPUT_DIR)/isocortex_mask_10.nrrd | $(OUTPUT_DIR)/
	echo -e "\
import numpy as np                                   \n\
from voxcell import VoxelData                        \n\
msk = VoxelData.load_nrrd('$<')                      \n\
hem = np.zeros_like(msk.raw)                         \n\
hem[:,:,range($(RANGE))] = 1                         \n\
msk.with_data(hem.astype(np.uint8)).save_nrrd('$@')  \n\
" | $(PYTHON)

# isocortex regions with simple labels
$(OUTPUT_DIR)/isocortex_regions_10.nrrd: $(OUTPUT_DIR)/isocortex_annotation_10.nrrd $(CCFV3_INPUT_DIR)/isocortex_mask_10.nrrd $(OUTPUT_DIR)/1_layersfix.json | $(OUTPUT_DIR)/
	echo -e "\
import numpy as np                                   \n\
from voxcell import VoxelData, RegionMap             \n\
annot = VoxelData.load_nrrd('$(word 1,$^)')          \n\
msk = VoxelData.load_nrrd('$(word 2,$^)')            \n\
rmap = RegionMap.load_json('$(word 3,$^)')           \n\
w = np.where(msk.raw > 0)                            \n\
wval = annot.raw[w]                                  \n\
ids = np.unique(wval)                                \n\
rdict = {id: { 'acro': rmap.get(id,'acronym',with_ascendants=True), 'name': rmap.get(id,'name')} for id in ids}\n\
rdict = {k: v['acro'][1] for k,v in rdict.items() if 'Isocortex' in v['acro']}\n\
rdict = list(set(rdict.values()))                    \n\
rdict.sort()                                         \n\
rdict = {k: i for i,k in enumerate(rdict)}           \n\
map = {}                                             \n\
for id in ids:                                       \n\
    found = [x in rmap.get(id,'acronym',with_ascendants=True) for x in rdict.keys()]\n\
    if any(found):                                   \n\
        ix = found.index(True)                       \n\
        map.update({id: ix})                         \n\
    else:                                            \n\
        map.update({id: -1})                         \n\
res = np.full_like(annot.raw, -1, dtype=np.int8)     \n\
res[w] = [map[x] for x in wval]                      \n\
annot.with_data(res).save_nrrd('$@')                 \n\
" | $(PYTHON)

# isocortex only
$(OUTPUT_DIR)/isocortex_annotation_10.nrrd: $(CCFV3_INPUT_DIR)/isocortex_mask_10.nrrd $(CCFV3_INPUT_DIR)/annotation_10.nrrd | $(OUTPUT_DIR)/
	echo -e "\
from voxcell import VoxelData                        \n\
msk = VoxelData.load_nrrd('$<')                      \n\
annot = VoxelData.load_nrrd('$(word 2,$^)')          \n\
annot.raw[msk.raw == 0] = 0                          \n\
annot.save_nrrd('$@')                                \n\
" | $(PYTHON)

# unify layer names in hierarchy
$(OUTPUT_DIR)/1_layersfix.json: $(CCFV3_INPUT_DIR)/1.json | $(OUTPUT_DIR)/
	sed \
		-e 's/, Layer/, layer/' \
		-e 's@/Layer \([1-6]\)@, layer \1@' \
		-e 's/, 6\([ab]\)/, layer 6\1/' \
		-e 's/,layer/, layer/' \
		$< > $@

# output directory
$(OUTPUT_DIR)/:
	mkdir -p $@
