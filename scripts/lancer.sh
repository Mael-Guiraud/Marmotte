#!/bin/bash

maitre=`cat ../addressemaitre`

gnome-terminal -x ssh pi@$maitre&

for i in `cat ../addressescalcul`;
do
	gnome-terminal -x ssh pi@$i &

done
