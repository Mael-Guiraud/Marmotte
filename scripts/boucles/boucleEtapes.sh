#!/bin/bash

maitre=`cat /home/merelas/Bureau/parallel/addressemaitre`

#gnome-terminal -e "bash -c 'ssh pi@$maitre "make";read' "&

let "X = 6"

while [ $X -le 60 ]
do

	    if [ $X -ge 10 ] 
		then
			let "X = X+2"
		fi 
		if [ $X -ge 30 ]
		then

			let "X = X+2"
		fi 
		if [ $X -ge 40 ]
		then
			let "X = X+2"
		fi 
		
		
	ssh pi@$maitre 'rm -f res'
	echo "#define NComponent 5
	#define NB_MACHINES 6
	#define NB_INTER $X
	#define TAILLE_SEQUENCE 1200000
	" > /home/merelas/Bureau/parallel/constantes.h
	./pushConfig3.sh
	sleep 1
	for j in `seq 1 1000`;
	do

		ssh pi@$maitre 'make' &
		sleep 1
		for i in `cat /home/merelas/Bureau/parallel/addressescalcul`;
		do
			ssh pi@$i 'make' &
		done
		echo "TOUR DE BOUCLE NUMERO $j ($X)"
		sleep 3

		if [ $X -ge 40 ]
		then
			sleep 3
		fi 
		 
	    if [ $j -eq 1 ]
		then
			echo "On attends les make"
        	sleep 2 
		fi 
	done
	scp pi@$maitre:/home/pi/res result$X
	let "X = X+1"

done

