#!/bin/bash

#maitre=`cat /home/merelas/Bureau/parallel/addressemaitre`

#for j in `ls /home/merelas/Bureau/parallel/maitre/`;
#	do
#		scp /home/merelas/Bureau/parallel/maitre/$j pi@$maitre:/home/pi
#done




for i in `cat /home/merelas/Bureau/parallel/addressescalcul`;
do
	echo "envoi a $i$"
	scp /home/merelas/Bureau/parallel/const.h pi@$i:/home/pi
	for j in `ls /home/merelas/Bureau/parallel/server/`;
	do
		scp /home/merelas/Bureau/parallel/server/$j pi@$i:/home/pi
	done

done

