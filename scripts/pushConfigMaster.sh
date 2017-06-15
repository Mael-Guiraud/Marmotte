#!/bin/bash

maitre=`cat ../addressemaitre`

for j in `ls ../maitre/`;
	do
		scp ../maitre/$j pi@$maitre:/home/pi
done
scp ../constantes.h pi@$maitre:/home/pi
