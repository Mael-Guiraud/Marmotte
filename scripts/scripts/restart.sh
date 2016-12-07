#!/bin/bash

maitre=`cat /home/merelas/Bureau/parallel/addressemaitre`

ssh pi@$maitre 'sudo reboot' &

for i in `cat /home/merelas/Bureau/parallel/addressescalcul`;
do
	for j in `ls /home/merelas/Bureau/parallel/calcul/`;
	do
		ssh pi@$i 'sudo reboot' &
	done

done

