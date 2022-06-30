rd_procs=$1
if [ -z "$1" ]
  then
    rd_procs=10
fi

for i in 1 10 100 200; do

array_size=$(( $(( 240 * 20000 * 1024 )) / 4 ))
wr_procs=$i
writers=$(( wr_procs * 24 ))
array_size=$(( $array_size / $writers ))
procs=$(( $wr_procs + $rd_procs ))
cores=$(( $procs * 24 ))
readers=$(( $rd_procs * 24 ))

echo "
#!/bin/bash -l
#BSUB -P CSC143
#BSUB -W 00:10
#BSUB -nnodes $procs
#BSUB -J sscTest
#BSUB -o output_ssc.%J
#BSUB -e output_ssc.%J

module load gcc

export LD_PRELOAD=$LD_PRELOAD:/usr/lib64/libibverbs.so.1:/usr/lib64/librdmacm.so.1

jsrun -n$cores /gpfs/alpine/csc143/proj-shared/againaru/adios/engine_perf/build/sscReadWriter $i $array_size 1 $readers
" > submit_ssc_temp$i.sh

bsub submit_ssc_temp$i.sh
done
