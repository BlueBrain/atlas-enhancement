# A jupyter notebook to compare flatmaps in various ways

The following files will be needed:
  - One reference flatmap
    - We used the AIBS dorsal flatmap at 10 um resolution: `https://download.alleninstitute.org/informatics-archive/current-release/mouse_ccf/cortical_coordinates/ccf_2017/dorsal_flatmap_paths_10.h5`
  - One candidate flatmap
    - We used our flatmap for mouse Isocortex at 10 um resolution, which can be obtained from our Zenodo repo: `https://zenodo.org/records/10686776`.
  - Region annotation voxel volume
    - We used the AIBS CCFv3 volume at 10 um resolution: `https://download.alleninstitute.org/informatics-archive/current-release/mouse_ccf/annotation/ccf_2022/annotation_10.nrrd`
    - Note that the annotation volume and both flatmaps must be using the same voxel dimensions and offset!
  - File defining the region hierarchy
    - We used the AIBS CCFv3 hierarchy: `http://api.brain-map.org/api/v2/structure_graph_download/1.json`
