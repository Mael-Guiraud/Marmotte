#!/bin/bash

maitre=`cat /home/merelas/Bureau/parallel/addressemaitre`

scp /home/merelas/Bureau/Parallelisation/maitre/maitre.c pi@$maitre:/home/pi
scp /home/merelas/Bureau/Parallelisation/maitre/Makefile pi@$maitre:/home/pi


for i in `cat /home/merelas/Bureau/parallel/addressescalcul`;
do
	scp /home/merelas/Bureau/Parallelisation/calcul/calcul.c pi@$i:/home/pi
	scp /home/merelas/Bureau/Parallelisation/calcul/Makefile pi@$i:/home/pi
done

