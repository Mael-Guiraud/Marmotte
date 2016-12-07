#!/bin/bash

maitre=`cat /home/merelas/Bureau/parallel/addressemaitre`

scp /home/merelas/Bureau/parallel/maitre/maitre.c pi@$maitre:/home/pi
scp /home/merelas/Bureau/parallel/maitre/Makefile pi@$maitre:/home/pi
scp /home/merelas/Bureau/parallel/constantes.h pi@$maitre:/home/pi


for i in `cat /home/merelas/Bureau/parallel/addressescalcul`;
do
	for j in `ls /home/merelas/Bureau/parallel/calcul/`;
	do
		scp /home/merelas/Bureau/parallel/calcul/$j pi@$i:/home/pi
	done
	scp /home/merelas/Bureau/parallel/constantes.h pi@$i:/home/pi

done

