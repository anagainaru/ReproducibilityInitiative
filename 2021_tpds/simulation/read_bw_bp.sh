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
writers=$(( $wr_procs * $cperp ))
readers=$(( $rd_procs * $cperp ))
read_array_size=$(( $(( $array_size * $wr_procs )) / $rd_procs ))

echo "
#!/bin/bash -l
#BSUB -P CSC143
#BSUB -W 00:30
#BSUB -nnodes $procs
#BSUB -J bpTest
#BSUB -o output_bp.%J
#BSUB -e output_bp.%J

module load gcc

jsrun -n$writers /gpfs/alpine/csc143/proj-shared/againaru/adios/engine_perf/build/bpWriter $i $array_size 1 
jsrun -n${readers} /gpfs/alpine/csc143/proj-shared/againaru/adios/engine_perf/build/bpReader $i $read_array_size 1
" > submit_bp_temp$i.sh

bsub submit_bp_temp$i.sh
done
