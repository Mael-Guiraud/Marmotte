#!/bin/bash


for i in `cat ../addressescalcul`;
do
		ssh pi@$i 'sudo reboot' &	
done
