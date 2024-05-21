PYTHON := python3
CAT := cat
SED := sed

CCFV3_INPUT_DIR := input
OUTPUT_DIR := output
ORIENTATION_OUTPUTS := $(OUTPUT_DIR)/orientation_x.nrrd $(OUTPUT_DIR)/orientation_y.nrrd $(OUTPUT_DIR)/orientation_z.nrrd
ALL_OUTPUTS := $(OUTPUT_DIR)/annotations.nrrd $(OUTPUT_DIR)/mask.nrrd $(OUTPUT_DIR)/relative_depth.nrrd $(ORIENTATION_OUTPUTS)

.DELETE_ON_ERROR: $(ALL_OUTPUTS)
.ONESHELL:

.PHONY: noop
noop:

.PHONY: all
all: $(ALL_OUTPUTS)

# layers
define recipe_layers
#import numpy as np
#from voxcell import VoxelData, RegionMap
#annot = VoxelData.load_nrrd('$<')
#rmap = RegionMap.load_json('$(word 2,$^)')
#zids = np.unique(annot.raw)
#ids = zids
#if zids[0] == 0:
#    ids = zids[1:]
#names = [rmap.get(x,'name') for x in list(ids)]
#pos = [x.index('layer ') if 'layer ' in x else -1 for x in names]
#lstr = [x[i+len('layer '):] if i > 0 else '0' for (x,i) in zip(names,pos)]
#ldict = {'0':0,'1':1,'2/3':2,'4':4,'5':5,'6a':6,'6b':6}
#lay = np.array([0] + [ldict[x] for x in lstr])
#sort_idx = np.argsort(zids)
#idx = np.searchsorted(zids,annot.raw,sorter = sort_idx)
#out = lay[sort_idx][idx]
#annot.with_data(out.astype('uint8')).save_nrrd('$@')
endef

.PHONY: layers
layers: $(OUTPUT_DIR)/layers.nrrd
$(OUTPUT_DIR)/layers.nrrd: $(OUTPUT_DIR)/isocortex_annotation_10.nrrd $(OUTPUT_DIR)/1_layersfix.json
	$(CAT) <<EOF | $(SED) 's/^#//' | $(PYTHON)
	$(recipe_layers)
	EOF

# outside, inside, sides, bottom, top
define recipe_mask
#import numpy as np
#from voxcell import VoxelData
#hem = VoxelData.load_nrrd('$<')
#iso = VoxelData.load_nrrd('$(word 2,$^)')
#bnd = VoxelData.load_nrrd('$(word 3,$^)')
#bnd.raw[bnd.raw == 4] = 2  # sides
#bnd.raw[bnd.raw == 1] = 4  # top
#bnd.raw[(bnd.raw == 0) & (iso.raw > 0)] = 1  #inner
#bnd.raw[hem.raw == 0] = 0
#bnd.with_data(bnd.raw.astype(np.int8)).save_nrrd('$@')
endef

.PHONY: mask
mask: $(OUTPUT_DIR)/mask.nrrd
$(OUTPUT_DIR)/mask.nrrd: $(OUTPUT_DIR)/mask_hemi.nrrd $(CCFV3_INPUT_DIR)/isocortex_mask_10.nrrd $(CCFV3_INPUT_DIR)/isocortex_boundary_10.nrrd | $(OUTPUT_DIR)/
	$(CAT) <<EOF | $(SED) 's/^#//' | $(PYTHON)
	$(recipe_mask)
	EOF

# isocortex regions in one hemisphere (1-based)
define recipe_isocortex
#import numpy as np
#from voxcell import VoxelData
#hem = VoxelData.load_nrrd('$<')
#reg = VoxelData.load_nrrd('$(word 2,$^)')
#reg.raw[hem.raw == 0] = -1
#reg.raw = reg.raw + 1
#reg.save_nrrd('$@')
endef

.PHONY: isocortex
isocortex: $(OUTPUT_DIR)/isocortex.nrrd
$(OUTPUT_DIR)/isocortex.nrrd: $(OUTPUT_DIR)/mask_hemi.nrrd $(OUTPUT_DIR)/isocortex_regions_10.nrrd | $(OUTPUT_DIR)/
	$(CAT) <<EOF | $(SED) 's/^#//' | $(PYTHON)
	$(recipe_isocortex)
	EOF

# isocortex regions in both hemispheres (0-based)
.PHONY: annotations
annotations: $(OUTPUT_DIR)/annotations.nrrd
$(OUTPUT_DIR)/annotations.nrrd: $(OUTPUT_DIR)/isocortex_regions_10.nrrd | $(OUTPUT_DIR)/
	cp $< $@

# orientation vector
define recipe_orientation
#import numpy as np
#from voxcell import VoxelData
#ext = VoxelData.load_nrrd('$<')
#msk = VoxelData.load_nrrd('$(word 2,$^)')
#msk_ext = (msk.raw != 1)
#dx, dy, dz = np.gradient(ext.raw)
#dx[msk_ext] = 0
#dy[msk_ext] = 0
#dz[msk_ext] = 0
#msk.with_data(dx.astype(np.float32)).save_nrrd('$(word 1,$(ORIENTATION_OUTPUTS))')
#msk.with_data(dy.astype(np.float32)).save_nrrd('$(word 2,$(ORIENTATION_OUTPUTS))')
#msk.with_data(dz.astype(np.float32)).save_nrrd('$(word 3,$(ORIENTATION_OUTPUTS))')
endef

.PHONY: orientation
orientation: $(ORIENTATION_OUTPUTS)
$(ORIENTATION_OUTPUTS): $(OUTPUT_DIR)/extension.nrrd $(OUTPUT_DIR)/hemispheres.nrrd | $(OUTPUT_DIR)/
	$(CAT) <<EOF | $(SED) 's/^#//' | $(PYTHON)
	$(recipe_orientation)
	EOF

# extend relative_depth values before gradient computation
define recipe_extension
#import numpy as np
#from voxcell import VoxelData
#from scipy import ndimage
#from joblib import Parallel, delayed
#import sys
## find voxel in src nearest to (x,y,z)
#def get_nearest(src,x,y,z):
#    D = src - (x,y,z)
#    d = np.sum(D * D, axis=1)
#    ix = np.argmin(d)
#    return src[ix]
## load data
#hem = VoxelData.load_nrrd('$(word 1,$^)')
#msk = VoxelData.load_nrrd('$(word 2,$^)')
#rdp = VoxelData.load_nrrd('$(word 3,$^)')
#bnd = VoxelData.load_nrrd('$(word 4,$^)')
## masks
#top = (bnd.raw == 1) & (hem.raw == 1)
#bottom = (bnd.raw == 3) & (hem.raw == 1)
#sides = (bnd.raw == 4) & (hem.raw == 1)
#msk_ext = (msk.raw != 1)
#msk_int = (msk.raw == 1)
## extend masks
#ball = ndimage.generate_binary_structure(3, 3)
#top_ext = ndimage.binary_dilation(top, ball) & msk_ext
#sides_ext = ndimage.binary_dilation(sides, ball) & msk_ext
#bottom_ext = ndimage.binary_dilation(bottom, ball) & msk_ext
#sides_int = ndimage.binary_dilation(sides, ball, 2) & msk_int
## extrapolate top and bottom
#rdp.raw[msk_ext] = 0  # NaN to zero
#rdp.raw[top_ext] = 1.01  # slightly above 1
#rdp.raw[bottom_ext] = -0.01  # slightly below 0
## extrapolate sides
#src = np.where(sides_int)
#dst = np.where(sides_ext)
#with Parallel(n_jobs=-1, prefer='threads') as parallel:
#    src_arr = np.array(src).T
#    dst_arr = np.array(dst).T
#    nearest = parallel(delayed(get_nearest)(src_arr,*x) for x in dst_arr)
#nst_ix = tuple([x for x in np.array(nearest).T])
#rdp.raw[dst] = rdp.raw[nst_ix]
## save data
#rdp.with_data(rdp.raw.astype(np.float32)).save_nrrd('$@')
endef

.PHONY: extension
extension: $(OUTPUT_DIR)/extension.nrrd
$(OUTPUT_DIR)/extension.nrrd: $(OUTPUT_DIR)/mask_hemi.nrrd $(OUTPUT_DIR)/hemispheres.nrrd $(OUTPUT_DIR)/relative_depth.nrrd $(CCFV3_INPUT_DIR)/isocortex_boundary_10.nrrd | $(OUTPUT_DIR)/
	$(CAT) <<EOF | $(SED) 's/^#//' | $(PYTHON)
	$(recipe_extension)
	EOF

# Laplace solution as relative depth, for a single hemisphere
define recipe_relative_depth
#import numpy as np
#from voxcell import VoxelData
#msk = VoxelData.load_nrrd('$<')
#lap = noxelData.load_nrrd('$(word 2,$^)')
#res = np.full_like(msk.raw, np.nan, dtype=np.float32)
#bmsk = (msk.raw == 1)
#res[bmsk] = 1 - lap.raw[bmsk]
#lap.with_data(res).save_nrrd('$@')
endef

.PHONY: relative-depth
relative-depth: $(OUTPUT_DIR)/relative_depth.nrrd
$(OUTPUT_DIR)/relative_depth.nrrd: $(OUTPUT_DIR)/hemispheres.nrrd $(CCFV3_INPUT_DIR)/laplacian_10.nrrd | $(OUTPUT_DIR)/
	$(CAT) <<EOF | $(SED) 's/^#//' | $(PYTHON)
	$(recipe_relative_depth)
	EOF

# isocortex mask in both hemispheres
define recipe_hemispheres
#import numpy as np
#from voxcell import VoxelData
#msk = VoxelData.load_nrrd('$<')
#hem = VoxelData.load_nrrd('$(word 2,$^)')
#hemo = VoxelData.load_nrrd('$(word 3,$^)')
#msk.raw[msk.raw != 0] = 1
#msk.raw[hemo.raw == 1] = msk.raw[hemo.raw == 1] * 2
#msk.with_data(msk.raw.astype(np.uint8)).save_nrrd('$@')
endef

.PHONY: hemispheres
hemispheres: $(OUTPUT_DIR)/hemispheres.nrrd
$(OUTPUT_DIR)/hemispheres.nrrd: $(CCFV3_INPUT_DIR)/isocortex_mask_10.nrrd $(OUTPUT_DIR)/mask_hemi.nrrd $(OUTPUT_DIR)/mask_hemi_other.nrrd
	$(CAT) <<EOF | $(SED) 's/^#//' | $(PYTHON)
	$(recipe_hemispheres)
	EOF

# hemisphere masks
define recipe_hemisphere_mask
#import numpy as np
#from voxcell import VoxelData
#msk = VoxelData.load_nrrd('$<')
#hem = np.zeros_like(msk.raw)
#hem[:,:,range($(RANGE))] = 1
#msk.with_data(hem.astype(np.uint8)).save_nrrd('$@')
endef

.PHONY: hemisphere-masks
hemisphere-masks: $(OUTPUT_DIR)/mask_hemi.nrrd $(OUTPUT_DIR)/mask_hemi_other.nrrd
$(OUTPUT_DIR)/mask_hemi.nrrd: RANGE := msk.shape[2] // 2, msk.shape[2]
$(OUTPUT_DIR)/mask_hemi_other.nrrd: RANGE := 0, msk.shape[2] // 2
$(OUTPUT_DIR)/mask_hemi.nrrd $(OUTPUT_DIR)/mask_hemi_other.nrrd: $(CCFV3_INPUT_DIR)/isocortex_mask_10.nrrd | $(OUTPUT_DIR)/
	$(CAT) <<EOF | $(SED) 's/^#//' | $(PYTHON)
	$(recipe_hemisphere_mask)
	EOF

# isocortex regions with simple labels
define recipe_isocortex_regions
#import numpy as np
#from voxcell import VoxelData, RegionMap
#annot = VoxelData.load_nrrd('$(word 1,$^)')
#msk = VoxelData.load_nrrd('$(word 2,$^)')
#rmap = RegionMap.load_json('$(word 3,$^)')
#w = np.where(msk.raw > 0)
#wval = annot.raw[w]
#ids = np.unique(wval)
#rdict = {id: { 'acro': rmap.get(id,'acronym',with_ascendants=True), 'name': rmap.get(id,'name')} for id in ids}
#rdict = {k: v['acro'][1] for k, v in rdict.items() if 'Isocortex' in v['acro']}
#rdict = list(set(rdict.values()))
#rdict.sort()
#rdict = {k: i for i, k in enumerate(rdict)}
#map = {}
#for id in ids:
#    found = [x in rmap.get(id, 'acronym', with_ascendants=True) for x in rdict.keys()]
#    if any(found):
#        ix = found.index(True)
#        map.update({id: ix})
#    else:
#        map.update({id: -1})
#res = np.full_like(annot.raw, -1, dtype=np.int8)
#res[w] = [map[x] for x in wval]
#annot.with_data(res).save_nrrd('$@')
endef

.PHONY: isocortex-regions
isocortex-regions: $(OUTPUT_DIR)/isocortex_regions_10.nrrd
$(OUTPUT_DIR)/isocortex_regions_10.nrrd: $(OUTPUT_DIR)/isocortex_annotation_10.nrrd $(CCFV3_INPUT_DIR)/isocortex_mask_10.nrrd $(OUTPUT_DIR)/1_layersfix.json | $(OUTPUT_DIR)/
	$(CAT) <<EOF | $(SED) 's/^#//' | $(PYTHON)
	$(recipe_isocortex_regions)
	EOF

# extract isocortex only
define recipe_isocortex_annotation
#from voxcell import VoxelData
#msk = VoxelData.load_nrrd('$<')
#annot = VoxelData.load_nrrd('$(word 2,$^)')
#annot.raw[msk.raw == 0] = 0
#annot.save_nrrd('$@')
endef

.PHONY: isocortex-annotation
isocortex-annotation: $(OUTPUT_DIR)/isocortex_annotation_10.nrrd
$(OUTPUT_DIR)/isocortex_annotation_10.nrrd: $(CCFV3_INPUT_DIR)/isocortex_mask_10.nrrd $(CCFV3_INPUT_DIR)/annotation_10.nrrd | $(OUTPUT_DIR)/
	$(CAT) <<EOF | $(SED) 's/^#//' | $(PYTHON)
	$(recipe_isocortex_annotation)
	EOF

# unify layer names in hierarchy
.PHONY: layers-fix
layers-fix: $(OUTPUT_DIR)/1_layersfix.json
$(OUTPUT_DIR)/1_layersfix.json: $(CCFV3_INPUT_DIR)/1.json | $(OUTPUT_DIR)/
	$(SED) \
		-e 's/, Layer/, layer/' \
		-e 's@/Layer \([1-6]\)@, layer \1@' \
		-e 's/, 6\([ab]\)/, layer 6\1/' \
		-e 's/,layer/, layer/' \
		$< > $@

# output directory
$(OUTPUT_DIR)/:
	mkdir -p $@
