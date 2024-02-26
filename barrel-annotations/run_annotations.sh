#!/bin/bash -l
#SBATCH --time=5:00:00
#SBATCH --partition=prod
#SBATCH --account=proj100
#SBATCH --nodes=1
#SBATCH --job-name=metrics
#SBATCH --exclusive
#SBATCH --mem=0 


echo 'load venv'

module purge
module load archive/2023-06 python
source /gpfs/bbp.cscs.ch/project/proj100/analysis/vbca/bin/activate
cd /gpfs/bbp.cscs.ch/project/proj100/atlas_work/mouse/atlas-enhancement

# python annotations_to_positions.py  
python split_barrel_columns.py
python generate_barrels_nrrd.py
python add_barrels_nrrd.py

echo 'done'     