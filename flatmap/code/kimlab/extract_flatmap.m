clear

shrink_ratio = 1;

flat_map_data = load("flat_map_data_10um_v2.mat");

plotting_1 = flat_map_data.plotting_1;
plotting_2 = flat_map_data.plotting_2;
pre_flat_index = flat_map_data.pre_flat_index;
pre_flat_dimention = flat_map_data.pre_flat_dimention;

mask = zeros(pre_flat_dimention, "uint8");
mask(pre_flat_index) = 1;

flat_xxx = plotting_1 - min(plotting_1);
flat_yyy = plotting_2 - min(plotting_2);

flat_xxx = ceil(flat_xxx .* shrink_ratio) + 1;
flat_yyy = ceil(flat_yyy .* shrink_ratio) + 1;

xflat = zeros(pre_flat_dimention, "uint16");
xflat(pre_flat_index) = flat_xxx;

yflat = zeros(pre_flat_dimention, "uint16");
yflat(pre_flat_index) = flat_yyy;

niftiwrite(mask,"mask.nii.gz");
niftiwrite(xflat,"xflat.nii.gz");
niftiwrite(yflat,"yflat.nii.gz");
