#!/bin/bash

maitre=`cat /home/merelas/Bureau/parallel/addressemaitre`

scp /home/merelas/Bureau/parallel/maitre/maitre.c pi@$maitre:/home/pi
scp /home/merelas/Bureau/parallel/maitre/Makefile pi@$maitre:/home/pi


for i in `cat /home/merelas/Bureau/parallel/addressescalcul`;
do
	scp /home/merelas/Bureau/parallel/calcul/calcul.c pi@$i:/home/pi
	scp /home/merelas/Bureau/parallel/calcul/headers.h pi@$i:/home/pi
	scp /home/merelas/Bureau/parallel/calcul/Makefile pi@$i:/home/pi
done

