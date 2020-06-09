#!/usr/bin/bash


usage(){ 
	echo "Usage: ./monitoring.sh [program-name-to-monitor] [path-to-output]" 
	echo "-h : Print help"  
} 



if [ $# -ne 2 ] ; then
	usage
else
	echo ### Begin Monitoring ###
	counter=0
	while pidof $1 > /dev/null; do
		tmp=$(vmstat -s | grep 'used memory' | cut -d'K' -f1)
		echo $counter,$tmp >> $2/monitoring.csv
		sleep 2
		((counter=counter+2))
	done
	echo ### End Monitoring ###
fi

