PYTHON := python3
LAPGASM3D_FULL := $(HOME)/src/atlas-enhancement/flatmap/code/LaplaceGaussSmooth/lapgasm3d_full
DATA_DIMS := x y z
DATA_SIZES := 566 320 456
VOXEL_SIZE := 25# um

DATA_FILES := relative_depth.nrrd \
			  orientation_x.nrrd \
			  orientation_y.nrrd \
			  orientation_z.nrrd \
			  mask.nrrd \
			  annotations.nrrd

.DELETE_ON_ERROR: $(DATA_FILES)
all: $(DATA_FILES)

orientation_x.nrrd: IX := 0
orientation_y.nrrd: IX := 1
orientation_z.nrrd: IX := 2
orientation_x.nrrd orientation_y.nrrd orientation_z.nrrd: input/direction_vectors.nrrd mask.nrrd
	echo -e "\
from voxcell import VoxelData                        \n\
import numpy as np                                   \n\
vec = VoxelData.load_nrrd('$<')                      \n\
msk = VoxelData.load_nrrd('$(word 2,$^)')            \n\
sel = (msk.raw == 0)                                 \n\
ori = vec.raw[:,:,:,$(IX)]                           \n\
ori[sel] = 0                                         \n\
vec.with_data(ori.astype(np.float32)).save_nrrd('$@')\n\
" | $(PYTHON)

relative_depth.nrrd: height.nrrd input/\[PH\]y.nrrd mask.nrrd
	echo -e "\
from voxcell import VoxelData                        \n\
import numpy as np                                   \n\
hei = VoxelData.load_nrrd('$<')                      \n\
phy = VoxelData.load_nrrd('$(word 2,$^)')            \n\
msk = VoxelData.load_nrrd('$(word 3,$^)')            \n\
rdp = np.full_like(msk.raw, np.nan, dtype=np.float32)\n\
sel = (msk.raw > 0)                                  \n\
rdp[sel] = phy.raw[sel] / hei.raw[sel]               \n\
msk.with_data(rdp).save_nrrd('$@')\n\
" | $(PYTHON)

height.nrrd: input/\[PH\]CBXmo.nrrd
	echo -e "\
from voxcell import VoxelData                        \n\
dat = VoxelData.load_nrrd('$<')                      \n\
up = dat.raw[:,:,:,1]                                \n\
dat.with_data(up).save_nrrd('$@')                    \n\
" | $(PYTHON)

mask.nrrd: mask_all.nrrd
	cp $< $@

#mask.nrrd: REGION := SIM
#mask.nrrd: REG_IX = $(shell awk '$$2 == "$(REGION)" { print $$1 }' regions.txt)
#mask.nrrd: annotations.nrrd
	#echo -e "\
#from voxcell import VoxelData                        \n\
#import numpy as np                                   \n\
#annot = VoxelData.load_nrrd('$<')                    \n\
#msk = (annot.raw == $(REG_IX))                       \n\
#annot.with_data(msk.astype(np.uint8)).save_nrrd('$@')\n\
#" | $(PYTHON)

annotations.nrrd: PARENT := CBX
annotations.nrrd: input/brain_regions.nrrd input/hierarchy.json mask_all.nrrd
	echo -e "\
from voxcell import VoxelData, RegionMap             \n\
import numpy as np                                   \n\
annot = VoxelData.load_nrrd('$(word 1,$^)')          \n\
rmap = RegionMap.load_json('$(word 2,$^)')           \n\
msk = VoxelData.load_nrrd('$(word 3,$^)')            \n\
w = np.where(msk.raw > 0)                            \n\
wval = annot.raw[w]                                  \n\
ids = np.unique(wval)                                \n\
rdict = {id: { 'acro': rmap.get(id,'acronym',with_ascendants=True), 'name': rmap.get(id,'name')} for id in ids}\n\
rdict = {k: v['acro'][1] for k,v in rdict.items() if '$(PARENT)' in v['acro']}\n\
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
for k,v in rdict.items():                            \n\
    print('{} {}'.format(v,k))                       \n\
" | $(PYTHON) > regions.txt

mask_all.nrrd: mask_CBX.nrrd mask_hemi.nrrd
	echo -e "\
from voxcell import VoxelData                        \n\
msk = VoxelData.load_nrrd('$<')                      \n\
hem = VoxelData.load_nrrd('$(word 2,$^)')            \n\
msk.raw = msk.raw * hem.raw                          \n\
msk.save_nrrd('$@')                                  \n\
" | $(PYTHON)

mask_CBX.nrrd: mask_%.nrrd:
	echo -e "\
from voxcell.nexus.voxelbrain import Atlas           \n\
import numpy as np                                   \n\
atlas = Atlas.open('input')                          \n\
msk = atlas.get_region_mask('$*')                    \n\
reg = atlas.load_data('brain_regions')               \n\
reg.with_data(msk.raw.astype(np.uint8)).save_nrrd('$@')\n\
" | $(PYTHON)

mask_hemi.nrrd: RANGE := msk.shape[2] // 2, msk.shape[2]
mask_hemi.nrrd: input/brain_regions.nrrd
	echo -e "\
from voxcell import VoxelData                        \n\
import numpy as np                                   \n\
msk = VoxelData.load_nrrd('$<')                      \n\
hem = np.zeros_like(msk.raw)                         \n\
hem[:,:,range($(RANGE))] = 1                         \n\
msk.with_data(hem.astype(np.uint8)).save_nrrd('$@')  \n\
" | $(PYTHON)

# per-lobe stuff
LOBES := ANcr1 ANcr2 CENT2 CENT3 COPY CUL4,5\
		 DEC FL FOTU LING NOD PFL PRM PYR SIM UVU
LOBE_FILES := $(addsuffix .smooth.img,$(addprefix lobe_,$(LOBES)))

relative_depth.nrrd: all_lobes.smooth.nrrd
	echo -e "\
from voxcell import VoxelData                        \n\
import numpy as np                                   \n\
dat = VoxelData.load_nrrd('$<')                      \n\
dat.raw[dat.raw == -1] = np.nan                      \n\
dat.raw[dat.raw == 2] = np.nan                       \n\
dat.save_nrrd('$@')                                  \n\
" | $(PYTHON)

all_lobes.smooth.nrrd: all_lobes.smooth.img
	# convert IMG to NRRD

all_lobes.smooth.img: NITER := 100000
all_lobes.smooth.img: LAPGASM3D_FULL_FLAGS := -B 1,0.598383,0
all_lobes.smooth.img: all_lobes.img all_lobes.mask.bin
	$(LAPGASM3D_FULL) \
		$(foreach i,1 2 3,-$(word $i,$(DATA_DIMS)) $(word $i,$(DATA_SIZES))) \
		-l $< -m $(word 2,$^) -M $(subst mask,nmask,$(word 2,$^)) \
		-r 3 -n $(NITER) -o $@ $(LAPGASM3D_FULL_FLAGS)

all_lobes.mask.bin: all_lobes_mask.img
	$(LAPGASM3D_FULL) \
		$(foreach i,1 2 3,-$(word $i,$(DATA_DIMS)) $(word $i,$(DATA_SIZES))) \
		-r 3 -E -l $< -m $@ -M $(subst mask,nmask,$@)

all_lobes_mask.img: all_lobes.img
	# manually set:
	# exterior (== 2.0) -> -1
	# interior (in [0.0,1.0]) -> 0
	# top boundary (== 1.0) -> 1
	# mid boundary (== 0.5) -> 2
	# bottom boundary (== 0.0) -> 3

all_lobes.img:
	# manually add all lobe_%.smooth.img files and fix values:
	# outside -> 2
	# WM -> 1
	# interior -> [0,1]

.PHONY: lobes
lobes: $(LOBE_FILES)

.PRECIOUS: lobe_%.img lobe_%.mask.bin

lobe_%.smooth.img: NITER := 2000
lobe_%.smooth.img: LAPGASM3D_FULL_FLAGS := -B 1,0.5,0 -f
lobe_%.smooth.img: lobe_%.img lobe_%.mask.bin
	$(LAPGASM3D_FULL) \
		$(foreach i,1 2 3,-$(word $i,$(DATA_DIMS)) $(word $i,$(DATA_SIZES))) \
		-l $< -m $(word 2,$^) -M $(subst mask,nmask,$(word 2,$^)) \
		-r 3 -n $(NITER) -o $@ $(LAPGASM3D_FULL_FLAGS)

lobe_%.mask.bin: lobe_%.img
	$(LAPGASM3D_FULL) \
		$(foreach i,1 2 3,-$(word $i,$(DATA_DIMS)) $(word $i,$(DATA_SIZES))) \
		-r 3 -G -l $< -m $@ -M $(subst mask,nmask,$@)

lobe_%.img: lobe_%.nrrd
	unu save -f nrrd -e raw -i $< | unu data - > $@

lobe_%.nrrd: input/brain_regions.nrrd input/hierarchy.json
	$(PYTHON) get_lobe.py $^ $*

.PHONY: clean-lobes clean-all-lobes
clean-lobes:
	$(RM) $(LOBE_FILES)

clean-all-lobes: clean-lobes
	$(RM) lobe_*.img lobe_*.bin
