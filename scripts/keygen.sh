#!/bin/bash

maitre=`cat ../addressemaitre`

ssh-keygen -f "~/.ssh//known_hosts" -R $maitre



for i in `cat ../addressescalcul`;
do
	ssh-keygen -f "~/.ssh//known_hosts" -R $i
done
