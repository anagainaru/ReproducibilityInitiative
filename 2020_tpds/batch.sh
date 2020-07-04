#!/usr/bin/env bash
#Job name
#SBATCH -J SLANT_1
# Asking for nodes
#SBATCH -N 1 -C miriel -t 48:00:00 --exclusive
# Output result message
#SBATCH -o slurm.sh%j.out
#Output error message
#SBATCH -e slurm.sh%j.err

source ~/.bashrc

echo "=====my job informations ==== "
echo "Node List: " $SLURM_NODELIST
echo "my jobID: " $SLURM_JOB_ID
echo "Partition: " $SLURM_JOB_PARTITION
echo "submit directory:" $SLURM_SUBMIT_DIR
echo "submit host:" $SLURM_SUBMIT_HOST
echo "In the directory: `pwd`"
echo "As the user: `whoami`"



### Path to working directory on lustre ###
path="/beegfs/vhonore/neuroscience/runs/part1"
path_input="/beegfs/vhonore/neuroscience/INPUT1"

rm -rf $path/logs/*


### List all the MRI inputs for SLANT ###
list=$(ls $path_input)




### Main loop ###
i=1
for input in $list
do
        echo --------- Input: $input ---------
        # Create folder for ith run
        mkdir $path/logs/$i
        # Move the considered input to input directory of SLANT
        cp $path_input/$input $path/input_dir
        # Rename the input file into test_volume.nii.gz
        mv $path/input_dir/$input $path/input_dir/test_volume.nii.gz
        # Link Singularity path to lustre partition
        export SINGULARITY_BINDPATH="$path/input_dir:/INPUTS,$path/logs/$i:/OUTPUTS"
        # Start SLANT application using Singularity to run docker image
        time singularity exec slant-deep_brain_seg_v1_0_0_CPU.simg /extra/run_deep_brain_seg.sh &
	sleep 2
	./monitoring.sh starter-suid $path/logs/$i
        # Move the input to save_logs folder on lustre partition
        rm -f $path/input_dir/test_volume.nii.gz
        ((i=i+1))
	sleep 5
done

### move the slurm files into save_logs folder on lustre ###
mv *.out *.err $path/logs
