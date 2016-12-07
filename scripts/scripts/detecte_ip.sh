#!/bin/bash



for X in `seq 1 254`
do

	ping -c 1 192.168.90.$X &

done
