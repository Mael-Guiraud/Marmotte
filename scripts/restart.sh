#!/bin/bash

maitre=`cat ../addressemaitre`

ssh pi@$maitre 'sudo reboot' &

for i in `cat ../addressescalcul`;
do
	for j in `ls ../calcul/`;
	do
		ssh pi@$i 'sudo reboot' &
	done

done
