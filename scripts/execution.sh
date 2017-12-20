#!/bin/bash

#maitre=`cat /home/merelas/Bureau/parallel/addressemaitre`

#gnome-terminal -e "bash -c 'ssh pi@$maitre "make";read' "&
#sleep 2
for i in `cat ../addressescalcul`;
do
#	xterm -hold -e "ssh pi@$i 'make'"&
	ssh pi@$i 'make' &
done