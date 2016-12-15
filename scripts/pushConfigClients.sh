#!/bin/bash

#maitre=`cat /home/merelas/Bureau/parallel/addressemaitre`

#for j in `ls /home/merelas/Bureau/parallel/maitre/`;
#	do
#		scp /home/merelas/Bureau/parallel/maitre/$j pi@$maitre:/home/pi
#done
#scp /home/merelas/Bureau/parallel/constantes.h pi@$maitre:/home/pi


for i in `cat /home/merelas/Bureau/parallel/addressescalcul`;
do
	echo "envoi a $i$"
	for j in `ls /home/merelas/Bureau/parallel/calcul/`;
	do
		scp /home/merelas/Bureau/parallel/calcul/$j pi@$i:/home/pi
	done

done

