# Annotation of barrels and barrel columns

Code needed to add barrel cortex annotations to the CCFv3 Allen Atlas at 10 $\mu m$ resolution.

## Download the files

All of the necessary files should be stored in `data` directory.

```
mkdir data
wget http://download.alleninstitute.org/informatics-archive/current-release/mouse_ccf/annotation/ccf_2022/annotation_10.nrrd
mv annotation_10.nrrd data/
python download_hierarchy.py
```


The atlas input files in data directory:

```
annotation_10.nrrd
1.json
```

### Download the chosen missing files from zenodo:

```
depth.nrrd
flatmap_float.nrrd

central-streamlines.feather
barrel_positions.feather

annotation_barrels.nrrd
hierarchy.json
```
---

## Place annotations and modify hierarchy

The following sections describes how to extend the atlas with barrel annotations:
* Introduction of barrels to the hierarchy.json
* Introduction of the annotated volumes to the annotations.nrrd

Annotations including barrel columns can be build in two ways:
- Original full implementation, starting with the barrel positions extracted from the images
- Faster intermediate implementation, using pre-generated `annotation_barrels.nrrd` and `hierarchy.json` that can be directly downloaded from Zenodo and transplanted into the Allen annottations

### Atlas-splitter approach with `feather` input

This approach is more computationally expensive but contains the full pipeline used in the paper.

1. Extract the full barrel column positions based on the image annotations from the Allen average atlas images and flatmap. Generate `annotated_barrels.feather`, the file containing positions of the 33 barrel columns obtained with a flatmap. 

   ```
   python annotations_to_positions.py
   ```

2. Run a modified `atlas-splitter` code using the positions annotations:

   Input files:
   - `annotation_10.nrrd`
   - `1.json`
   - `annotated_barrels.feather`

   ```
   python split_barrel_columns.py
   ```

   Output files:
   - `hierarchy.json`
   - `annotation_barrels_10.nrrd`

3. Generate the optional nrrd file containing only the new barrel annotation ids. This can be transplanted directly into the original atlas (this file can be also downloaded from Zenodo).

   ```
   python generate_barrel_nrrd.py
   ```
   Output file:
   - `annotation_barrels.nrrd`
   
### Using barrel columns `nrrd`

Using pre-generated file `annotation_barrels.nrrd`, the newly annotated voxels can be transplanted directly into the ``annotation_10.nrrd``. New voxels will overwrite parts of the SSp-bfd, accodring to the included ``hierarchy.json``.


Input files:
- `annotation_barrels.nrrd`
- `annotation_10.nrrd`
- `hierarchy.json`

```
python transplant_barrels_nrrd.py
```

Output files:
- `annotation_barrels_10.nrrd`

---

## Extract metrics of the barrels and barrel columns

To extract the measured reported in the paper, you can use the code provided in `metrics` subdirectory.
Datasets necessary to extract the measures:

```
data/annotated_barrels.feather
data/central-streamlines.feather
data/depth.nrrd
```

Note: Before running the metrics extraction, you need to extract all of the voxel positions corresponding to each barrel column (if the file is missing`data/annotated_barrels.feather`), by running the following script:

```
annotations_to_positions.py
```

To extract the metrics values, you have to run a script in the metrics directory:

```
bash run_metrics.sh
```
Note: the order of the metric extraction matters, as some of the metrics depend on the others (i.e. curvature displacement requires depth)

   
---

## Funding

This project/research was supported by funding to the Blue Brain Project, a research center of the École polytechnique fédérale de Lausanne (EPFL), from the Swiss government’s ETH Board of the Swiss Federal Institutes of Technology.
