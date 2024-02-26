# Some of the more detailed metrics require the barrel centers and depth files, so run those first.

echo 'extract barrel centers'
python extract_barrel_centers.py

echo 'extract depth'
python get_layer_shells.py
python shell_based_depth.py
python extract_shell_depth.py

echo 'extract other metrics'
python extract_volume_properties.py
python extract_ellipticity.py
python extract_conicity.py
python extract_curvature_displacement.py


echo 'done' 