procs=$1
if [ -z "$1" ]
  then
    procs=4
fi

echo "
app 0: /gpfs/alpine/csc143/proj-shared/againaru/gray-scott/build/gray-scott /gpfs/alpine/csc143/proj-shared/againaru/gray-scott/simulation/settings.json 
app 1: /gpfs/alpine/csc143/proj-shared/againaru/gray-scott/build/pdf_calc gs.bp pdf.bp 100
overlapping_rs: warn
oversubscribe_cpu: warn
oversubscribe_mem: allow
oversubscribe_gpu: allow
launch_distribution: packed
" > gray-scott.erf

cnt=0
apid=0
for (( p=1; p<=$(( $procs + 1 )); p++ )); do
for (( core=0; core<24; core++ )); do
coreid=$(( $core * 4 ))
echo "rank: $cnt: { host: $p; cpu: {$coreid-$(( $coreid + 3 ))} } : app $apid" >> gray-scott.erf
cnt=$(( $cnt + 1 ))
done
if [ $p -eq $procs ]; then
apid=1
fi
done

echo "
#!/bin/bash -l
#BSUB -P CSC143
#BSUB -W 00:30
#BSUB -nnodes $(( $procs + 1 ))
#BSUB -J gray-scott
#BSUB -o output_gs.%J
#BSUB -e output_gs.%J

module load gcc

export LD_PRELOAD=$LD_PRELOAD:/usr/lib64/libibverbs.so.1:/usr/lib64/librdmacm.so.1

jsrun --erf_input gray-scott.erf
" > gray-scott-summit.sh

bsub gray-scott-summit.sh
