from voxcell import VoxelData, RegionMap
from scipy.ndimage import distance_transform_edt as edt
import numpy as np
import sys

regions_nrrd = sys.argv[1]
hierarchy_json = sys.argv[2]

lobe = sys.argv[3]
if lobe == "CUL4,5":  # name fix
    lobe = "CUL4, 5"

rmap = RegionMap.load_json(hierarchy_json)
mo_ids = [y[0] for y in [(x,rmap.get(x,'acronym',with_ascendants=True)) for x in rmap.find('@.*mo$',attr='acronym')] if 'CBX' in y[1]]
gr_ids = [y[0] for y in [(x,rmap.get(x,'acronym',with_ascendants=True)) for x in rmap.find('@.*gr$',attr='acronym')] if 'CBX' in y[1]]
cbx_ids = mo_ids + gr_ids
wm_ids = list(rmap.find('arb',attr='acronym')) + list(rmap.find('cbf',attr='acronym',with_descendants=True))
cbn_ids = list(rmap.find('CBN',attr='acronym',with_descendants=True))
cb_ids = cbx_ids + wm_ids + cbn_ids

reg = VoxelData.load_nrrd(regions_nrrd)

w_cbx = np.isin(reg.raw, cbx_ids)
wval = reg.raw[w_cbx]
uids = np.unique(wval)

# get simple numbered regions
annot = np.full_like(reg.raw, -1, dtype=np.int8)

w_mo = np.isin(reg.raw, mo_ids)
annot[w_mo] = 2# upper layer

w_gr = np.isin(reg.raw, gr_ids)
annot[w_gr] = 1# lower layer

w_wm = np.isin(reg.raw, wm_ids + cbn_ids)
annot[w_wm] = 0# WM

## set WM next to molecular layer to outside
nn_mo = (edt(~w_mo) == 1)
annot[nn_mo & (annot == 0)] = -1

## set outside next to granular layer to WM
nn_gr = (edt(~w_gr) == 1)
annot[nn_gr & (annot == -1)] = 0

## clear everything else
annot[~nn_mo & ~nn_gr & ~w_mo & ~w_gr] = -1

reg.with_data(annot).save_nrrd('regs_mo_gr_wm.nrrd')

# produce per-lobe input for lapgasm
lobes = list(set([rmap.get(x, 'acronym', with_ascendants=True)[1] for x in uids]))
if lobe not in lobes:
    raise ValueError('Unknown lobe {}'.format(lobe))

lobe_ids = [y[0] for y in [(x,rmap.get(x,'acronym',with_ascendants=True)) for x in rmap.find('@.*mo$',attr='acronym')] if lobe in y[1]] +\
           [y[0] for y in [(x,rmap.get(x,'acronym',with_ascendants=True)) for x in rmap.find('@.*gr$',attr='acronym')] if lobe in y[1]]
lobe_msk = np.isin(reg.raw, lobe_ids)
lobe_msk_nn = ((edt(~lobe_msk) == 1) & (annot == 0)) | lobe_msk
lobe_annot = np.copy(annot)
lobe_annot[~lobe_msk_nn] = -1
lobe_annot[lobe_annot == 0] = 3  # wm
lobe_annot[lobe_annot == 1] = 4  # temp
lobe_annot[lobe_annot == 2] = 1  # mo
lobe_annot[lobe_annot == 4] = 2  # gr
lobe_annot[lobe_annot == -1] = 0  # outside
reg.with_data(lobe_annot.astype(np.uint8)).save_nrrd('lobe_{}.nrrd'.format(lobe.replace(" ","")))
