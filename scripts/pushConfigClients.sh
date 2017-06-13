#!/bin/bash

#maitre=`cat /home/merelas/Bureau/parallel/addressemaitre`

#for j in `ls /home/merelas/Bureau/parallel/maitre/`;
#	do
#		scp /home/merelas/Bureau/parallel/maitre/$j pi@$maitre:/home/pi
#done




for i in `cat ../addressescalcul`;
do
	echo "envoi a $i$"
	scp ../const.h pi@$i:/home/pi
	for j in `ls ../server/`;
	do
		scp ../server/$j pi@$i:/home/pi
	done

done

