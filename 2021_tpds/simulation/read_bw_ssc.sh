wr_procs=$1
if [ -z "$1" ]
  then
    wr_procs=100
fi

cperp=24
for i in 1 10 100 200; do

array_size=$(( 40000 * 1024 ))
rd_procs=$i
procs=$(( $wr_procs + $rd_procs ))
cores=$(( $procs * $cperp ))
readers=$(( $rd_procs * $cperp ))

echo "
#!/bin/bash -l
#BSUB -P CSC143
#BSUB -W 00:30
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
