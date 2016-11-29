#!/bin/bash

maitre=`cat /home/merelas/Bureau/parallel/addressemaitre`

#gnome-terminal -e "bash -c 'ssh pi@$maitre "make";read' "&



for X in `seq 1 12`;
do

	echo "#define NComponent 10
	#define NB_MACHINES 6
	#define X $X
	#define TAILLE_SEQUENCE 120000
	" > /home/merelas/Bureau/parallel/constantes.h
	./pushConfig3.sh
	for j in `seq 1 1000`;
	do

		ssh pi@$maitre 'make' &
		sleep 2
		for i in `cat /home/merelas/Bureau/parallel/addressescalcul`;
		do
			ssh pi@$i 'make' &
		done
		echo "TOUR DE BOUCLE NUMERO $j ($X)"
	    sleep 2  
	done
	scp pi@$maitre:/home/pi/res result$X
	ssh pi@$maitre 'rm -f res'

done

