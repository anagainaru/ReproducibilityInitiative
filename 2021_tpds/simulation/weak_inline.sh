exetime='10'
num_variables=1
for i in 32 64 128 256 512 1024; do

array_size=$1
if [ -z "$1" ]
  then
    array_size=250000000 # default ~1 GB data per writer
fi

procs=$i
cores=$(( $procs * 24 ))
array_size=$(( $array_size / $cores ))

if [ $i -gt 256 ]; then
exetime='50'
fi

echo "
#!/bin/bash -l
#BSUB -P CSC143
#BSUB -W 00:$exetime
#BSUB -nnodes $procs
#BSUB -J inlineTest
#BSUB -o output_inline.%J
#BSUB -e output_inline.%J

module load gcc

jsrun -n$cores /gpfs/alpine/csc143/proj-shared/againaru/adios/engine_perf/build/iReadWriter $i $array_size $num_variables
" > submit_inline_temp$i.sh

bsub submit_inline_temp$i.sh
done
