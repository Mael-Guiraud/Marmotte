#!/bin/bash

maitre=`cat /home/merelas/Bureau/parallel/addressemaitre`

gnome-terminal -x ssh pi@$maitre&

for i in `cat /home/merelas/Bureau/parallel/addressescalcul`;
do
	gnome-terminal -x ssh pi@$i &

done
