#!/bin/bash

maitre=`cat /home/merelas/Bureau/parallel/addressemaitre`

for j in `ls /home/merelas/Bureau/parallel/maitre/`;
	do
		scp /home/merelas/Bureau/parallel/maitre/$j pi@$maitre:/home/pi
done
scp /home/merelas/Bureau/parallel/constantes.h pi@$maitre:/home/pi

