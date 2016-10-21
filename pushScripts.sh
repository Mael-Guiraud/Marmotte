#!/bin/bash

maitre=`cat /home/merelas/Bureau/parallel/addressemaitre`

scp /home/merelas/Bureau/Parallelisation/maitre/script.sh pi@$maitre:/home/pi



for i in `cat /home/merelas/Bureau/parallel/addressescalcul`;
do
	scp /home/merelas/Bureau/Parallelisation/calcul/script.sh pi@$i:/home/pi

done

