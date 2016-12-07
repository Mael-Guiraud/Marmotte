#!/bin/bash

maitre=`cat /home/merelas/Bureau/parallel/addressemaitre`

scp /home/merelas/Bureau/parallel/tests/testmaster.c pi@$maitre:/home/pi



for i in `cat /home/merelas/Bureau/parallel/addressescalcul`;
do
	scp /home/merelas/Bureau/parallel/tests/testslave.c pi@$i:/home/pi

done

