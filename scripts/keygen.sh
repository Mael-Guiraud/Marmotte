#!/bin/bash

maitre=`cat /home/merelas/Bureau/parallel/addressemaitre`

ssh-keygen -f "/home/merelas/.ssh//known_hosts" -R $maitre



for i in `cat /home/merelas/Bureau/parallel/addressescalcul`;
do
	ssh-keygen -f "/home/merelas/.ssh//known_hosts" -R $i
done

