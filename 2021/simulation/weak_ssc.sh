exetime='00:30'
num_variables=1
for i in 32 64 128 256 512 1024; do

array_size=$1
if [ -z "$1" ]
  then
    array_size=25000000000 # default ~1 GB data per writer
fi

wr_procs=$i
rd_procs=$(( $i / 2 ))
writers=$(( wr_procs * 24 ))
procs=$(( $wr_procs + $rd_procs ))
cores=$(( $procs * 24 ))
readers=$(( $rd_procs * 24 ))
array_size=$(( $array_size / $writers ))
read_array_size=$(( $(( $array_size * $wr_procs )) / $rd_procs ))
if [ $i -gt 256 ]; then
exetime='1:00'
fi

echo "
#!/bin/bash -l
#BSUB -P CSC143
#BSUB -W $exetime
#BSUB -nnodes $procs
#BSUB -J sscTest
#BSUB -o output_ssc.%J
#BSUB -e output_ssc.%J

module load gcc

export LD_PRELOAD=$LD_PRELOAD:/usr/lib64/libibverbs.so.1:/usr/lib64/librdmacm.so.1

jsrun -n$cores /gpfs/alpine/csc143/proj-shared/againaru/adios/engine_perf/build/sscReadWriter $i $array_size $num_variables $readers
" > submit_ssc_temp$i.sh

bsub submit_ssc_temp$i.sh
done
