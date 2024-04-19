# A jupyter notebook to generate plots comparing flatmaps

The following files will be needed:
  - One reference flatmap
    - We used the AIBS dorsal flatmap from: https://github.com/AllenInstitute/mouse_connectivity_models/tree/master/mcmodels/core/cortical_coordinates
  - One candidate flatmap
    - We used our candidate flatmap for mouse Isocortex at 100 um resolution. To be obtained from our Zenodo repo.
  - Region annotation voxel volume
    - We used the AIBS CCFv3 volume: https://download.alleninstitute.org/informatics-archive/current-release/mouse_ccf/annotation
    - Note that the annotation volume and both flatmaps must be using the same voxel dimensions and offset!
  - File defining the region hierarchy
    - Obtain Table S2 from the article introducing the Allen CCFv3 (https://www.sciencedirect.com/science/article/pii/S0092867420304025#mmc2), then convert it from the offered Excel format to .csv


