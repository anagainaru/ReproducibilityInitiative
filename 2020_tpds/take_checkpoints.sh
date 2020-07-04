#!/bin/bash

memorylow=(10120 10117)
memoryhigh=(10320 10317)
idx=0

while true
do
        echo "Look for ${memorylow[$idx]} to ${memoryhigh[$idx]} MB"
        mem=`podman stats --no-stream | grep GB | cut -d" " -f10`
#       echo "Reading Podman memory: $mem"
        if [[ "$mem" == *"G"* ]]; then
                mem=`echo $mem | cut -d"G" -f1`
                mem=`echo "$mem * 1024" | bc -l`
        else
                mem=`echo $mem | cut -d"M" -f1`
        fi

        mem=`echo $mem | cut -d"." -f1`
        echo "Podman memory: $mem MB"

        if [ "$mem" -lt "${memoryhigh[$idx]}" ]; then
                if [ "$mem" -gt "${memorylow[$idx]}" ]; then
                        idx=$(( $idx + 1 ))
                        ( time podman container checkpoint -l ) |&  tee check_run$idx.info
                        du /var/lib/containers/storage/overlay-containers/*/userdata/checkpoint >> slant_logs/newrun15.cr
                        ( time podman container restore -l ) |&  tee restart_run$idx.info
                        echo "Take checkpoint"
                        sleep 10
                fi
        fi

        if [ $idx -gt 1 ]; then
                break
        fi
done

podman stop slant
podman logs --latest | grep time > slant_logs/newrun.log
podman container inspect slant >> slant_logs/newrun.log
podman rm slant
rm input/*.nii.gz
rm -rf input/output/*
