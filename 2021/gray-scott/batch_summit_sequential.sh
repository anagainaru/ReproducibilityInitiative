#!/bin/bash -l
#BSUB -P CSC143
#BSUB -W 00:30
#BSUB -nnodes 10
#BSUB -J gray-scott
#BSUB -o output_bpgs.%J
#BSUB -e output_bpgs.%J

module load gcc

jrun -n 80 /gpfs/alpine/csc143/proj-shared/againaru/gray-scott/build/gray-scott /gpfs/alpine/csc143/proj-shared/againaru/gray-scott/simulation/settings.json
jrun -n 20 /gpfs/alpine/csc143/proj-shared/againaru/gray-scott/build/pdf_calc gs.bp pdf.bp 100
